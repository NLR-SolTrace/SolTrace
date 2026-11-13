
#include "parabolic_trough.hpp"

#include <algorithm>
#include <sstream>
#include <stdexcept>

#include "aperture.hpp"
#include "arclength.hpp"
#include "composite_element.hpp"
#include "constants.hpp"
#include "element.hpp"
#include "utilities.hpp"
#include "surface.hpp"

namespace SolTrace::Data {

ParabolicTrough::ParabolicTrough()
    : initialized(false),
      //   ready_to_add_self(false),
      azimuth(-1.0),
      tilt(-1.0),
      aperture_size_x(-1.0),
      aperture_size_y(-1.0),
      focal_length(-1.0),
      gap_x(-1.0),
      gap_y(-1.0),
      gap_center(-1.0),
      num_panels_x(-1),
      num_panels_y(-1),
      absorber_diameter(-1.0),
      envelope_diameter(-1.0),
      envelope_thickness(-1.0),
      tracking_angle(0.0),
      tracking_limit_lower(-180.0),
      tracking_limit_upper(180.0)
{
    this->tracking_origin = {1.0, 0.0, 0.0};
    this->rotation_axis = {0.0, 1.0, 0.0};
    this->neutral_normal = {0.0, 0.0, 1.0};
    return;
}

ParabolicTrough::~ParabolicTrough()
{
    absorbers.clear();
    envelopes.clear();
    mirrors.clear();
    return;
}

void ParabolicTrough::create_geometry()
{
    // TODO: Check that gap values are consistent with number of panels

    if (initialized)
        return;

    this->enforce_user_fields_set();

    this->absorbers.clear();
    this->mirrors.clear();
    this->envelopes.clear();
    this->clear();

    // Mirrors
    aperture_ptr ap = nullptr;
    single_element_ptr panel = nullptr;
    surface_ptr surf = nullptr;
    glm::dvec3 origin;
    glm::dvec3 aim;
    double xstop;
    double gap;
    double ystart, ycoord;

    // double ystart = -0.5 * this->aperture_size_y;
    double panel_length_y = (this->aperture_size_y -
                             this->gap_y * (this->num_panels_y - 1)) /
                            this->num_panels_y;

    double hw = 0.5 * this->aperture_size_x;
    double xstart = -hw;
    double arc_length = hw * sqrt(hw * this->cx * hw * this->cx + 1) +
                        asinh(hw * this->cx) / this->cx;
    double panel_arc_length = arc_length -
                              this->gap_x * (this->num_panels_x - 1);
    panel_arc_length -= this->gap_center - this->gap_x;
    panel_arc_length /= this->num_panels_x;

    // NOTE: Integer (floor) division is intentional here.
    uint_fast64_t half_n_x = this->num_panels_x / 2;

    for (int i = 0; i < this->num_panels_x; ++i)
    {
        xstop = determine_x_coordinate(xstart, panel_arc_length);
        ystart = -0.5 * this->aperture_size_y;
        ycoord = ystart + 0.5 * panel_length_y;

        for (int j = 0; j < this->num_panels_y; ++j)
        {
            // std::cout << "**** i = " << i << "  j = " << j << " ****"
            //           << "\nxstart = " << xstart << "  xstop = " << xstop
            //           << "\nystart = " << ystart << "  ycoord = " << ycoord
            //           << "\npanel_len_y = " << panel_length_y
            //           << "\npanel_arc_len = " << panel_arc_length
            //           << "\narc_len = " << arc_length
            //           << std::endl;

            origin = {0.0, ycoord, 0.0};
            aim = {0.0, ycoord, 1000.0};

            panel = make_element<SingleElement>();
            panel->set_name("ParabolicMirror");
            panel->set_reference_frame_geometry(origin, aim, 0.0);
            ap = make_aperture<Rectangle>(xstop - xstart,
                                          panel_length_y,
                                          xstart,
                                          ystart);
            panel->set_aperture(ap);
            surf = make_surface<Parabola>(
                Parabola::curvature_from_focal_length(this->focal_length), 0.0);
            panel->set_surface(surf);
            panel->set_optical_property_set(this->optics_mirror);
            panel->enable();

            this->mirrors.push_back(panel);
            this->add_element(panel);

            ystart += panel_length_y + this->gap_y;
            ycoord = ystart + 0.5 * panel_length_y;
        }
        gap = i == this->num_panels_x / 2 - 1 ? this->gap_center : this->gap_x;
        // std::cout << "Gap: " << gap << std::endl;
        // std::cout << "this->num_panels_x / 2 = " << this->num_panels_x / 2
        //           << std::endl;
        xstart = determine_x_coordinate(xstop, gap);
    }

    // Absorber
    // TODO: Single element at the moment. Break up into multiple tubes.
    auto abs = make_element<SingleElement>();
    abs->set_name("Absorber");
    abs->set_origin(0.0, 0.0,
                    this->focal_length - 0.5 * this->absorber_diameter);
    abs->set_aim_vector(0.0, 0.0, 1.0);
    abs->set_zrot(0.0);
    abs->compute_coordinate_rotations();
    abs->set_aperture(make_aperture<Rectangle>(this->absorber_diameter,
                                               this->aperture_size_y));
    abs->set_surface(make_surface<Cylinder>(0.5 * this->absorber_diameter));
    abs->set_optical_property_set(optics_absorber);
    abs->enable();
    this->absorbers.push_back(abs);
    auto id = this->add_element(abs);
    if (!Element::is_success(id))
    {
        // TODO: Make this more helpful
        throw std::runtime_error("Failed to add absorber element");
    }

    // Envelope -- Outer
    auto envout = make_element<SingleElement>();
    envout->set_name("EnvelopeOuter");
    envout->set_origin(0.0, 0.0,
                       this->focal_length - 0.5 * this->envelope_diameter);
    envout->set_aim_vector(0.0, 0.0, 1.0);
    envout->set_zrot(0.0);
    envout->compute_coordinate_rotations();
    envout->set_aperture(make_aperture<Rectangle>(this->envelope_diameter,
                                                  this->aperture_size_y));
    envout->set_surface(make_surface<Cylinder>(0.5 * this->envelope_diameter));
    envout->set_optical_property_set(optics_envelope_outer);
    envout->enable();
    this->envelopes.push_back(envout);
    id = this->add_element(envout);
    if (!Element::is_success(id))
    {
        throw std::runtime_error("Failed to add outer envelope element");
    }

    // Envelope -- Inner
    auto envin = make_element<SingleElement>();
    envin->set_name("EnvelopeInner");
    envin->set_origin(0.0, 0.0,
                      this->focal_length - 0.5 * this->envelope_diameter +
                          this->envelope_thickness);
    envin->set_aim_vector(0.0, 0.0, 1.0);
    envin->set_zrot(0.0);
    envin->compute_coordinate_rotations();
    double ap_x = this->envelope_diameter - 2 * this->envelope_thickness;
    double ap_y = this->aperture_size_y;
    envin->set_aperture(make_aperture<Rectangle>(ap_x, ap_y));
    envin->set_surface(make_surface<Cylinder>(0.5 * ap_x));
    envin->set_optical_property_set(optics_envelope_inner);
    envin->enable();
    this->envelopes.push_back(envin);
    id = this->add_element(envin);
    if (!Element::is_success(id))
    {
        throw std::runtime_error("Failed to add inner envelope element");
    }

    this->enable();

    this->rotation_axis = glm::normalize(this->rotation_axis);
    this->tracking_origin = glm::normalize(this->tracking_origin);

    rotate_vector_degrees(this->rotation_axis,
                          this->tracking_origin,
                          -this->tracking_limit_lower,
                          this->vector_lower_limit);
    this->vector_lower_limit = glm::normalize(this->vector_lower_limit);

    rotate_vector_degrees(this->rotation_axis,
                          this->tracking_origin,
                          -this->tracking_limit_upper,
                          this->vector_upper_limit);
    this->vector_upper_limit = glm::normalize(this->vector_upper_limit);

    this->initialized = true;

    return;
}

void ParabolicTrough::update_geometry(double azimuth,
                                      double elevation)
{

    if (elevation < 0.0 || elevation > 90.0)
    {
        std::stringstream ss;
        ss << "ParabolicTrough::update_geometry: Invalid elevation ("
           << elevation << "). Elevation must lie between 0 and 90 degrees.";
        throw std::invalid_argument(ss.str());
    }

    if (azimuth < -180.0 || azimuth > 180.0)
    {
        std::stringstream ss;
        ss << "ParabolicTrough::update_geometry: Invalid azimuth ("
           << elevation << "). Azimuth must lie between -180 and 180 degrees.";
        throw std::invalid_argument(ss.str());
    }

    if (!this->initialized)
    {
        std::stringstream ss;
        ss << "ParabolicTrough::update_geometry: Uninitialized. "
           << "Call create_geometry() first.";
        throw std::invalid_argument(ss.str());
    }

    this->coordinates_initialized = false;

    glm::dvec3 sun_pos;
    sun_position_vector_degrees(sun_pos, azimuth, elevation);
    sun_pos = glm::normalize(sun_pos);

    // Project into the plane defined by rotation axis as the normal
    glm::dvec3 sun_proj;
    project_onto_plane(this->rotation_axis, sun_pos, sun_proj);
    sun_proj = glm::normalize(sun_proj);

    assert(glm::dot(sun_proj, this->rotation_axis) < 1e-12);

    // double theta = acos(glm::dot(sun_proj, this->tracking_origin)) * R2D;
    double theta = acos(glm::dot(sun_proj, this->neutral_normal)) * R2D;
    if (glm::dot(sun_proj, this->tracking_origin) < 0.0)
        theta = -theta;

    if (theta < this->tracking_limit_lower)
    {
        this->tracking_angle = this->tracking_limit_lower;
        this->convert_global_to_reference(this->aim,
                                          this->vector_lower_limit);
    }
    else if (theta > this->tracking_limit_upper)
    {
        this->tracking_angle = this->tracking_limit_upper;
        this->convert_global_to_reference(this->aim,
                                          this->vector_upper_limit);
    }
    else
    {
        this->tracking_angle = theta;
        this->convert_global_to_reference(this->aim, sun_proj);
    }

    glm::dvec3 rotation_axis_ref;
    this->aim = glm::normalize(this->aim);
    double beta = asin(this->aim[1]);
    this->convert_global_to_reference(rotation_axis_ref,
                                      this->rotation_axis);
    double gamma = acos(rotation_axis_ref[1] / cos(beta));

    this->set_zrot_radians(gamma);
    this->aim *= 1000.0;
    this->aim = this->origin + this->aim;

    this->compute_coordinate_rotations();

    return;
}

double ParabolicTrough::calculate_receiver_power()
{
    // TODO: Implement...
    return 0.0;
}

double ParabolicTrough::get_tracking_angle_degrees() const
{
    return this->tracking_angle;
}

double ParabolicTrough::get_tracking_angle_radians() const
{
    return D2R * this->get_tracking_angle_degrees();
}

void ParabolicTrough::set_angles(double azimuth, double tilt)
{
    if (azimuth < -180.0 || azimuth > 180.0)
    {
        std::stringstream ss;
        ss << "ParabolicTrough::set_angles: Invalid azimuth angle ("
           << azimuth << "). Must be between -180 and 180 degrees.";
        throw std::invalid_argument(ss.str());
    }

    if (tilt < 0.0 || tilt > 90.0)
    {
        std::stringstream ss;
        ss << "ParabolicTrough::set_angles: Invalid tilt angle ("
           << tilt << "). Must be between 0 and 90 degrees.";
        throw std::invalid_argument(ss.str());
    }

    this->coordinates_initialized = false;
    this->azimuth = azimuth;
    this->tilt = tilt;

    // Convert input angles to spherical coordinate angles
    double az = azimuth * D2R;
    double el = tilt * D2R;
    double pol = 0.5 * PI - az;
    double inc = 0.5 * PI - el;

    // Convert spherical coordinates to cartesian coordinates
    // y-axis
    this->rotation_axis = {
        sin(inc) * cos(pol),
        sin(inc) * sin(pol),
        cos(inc)
    };

    // z-axis
    this->neutral_normal = {
        sin(-el) * cos(pol),
        sin(-el) * sin(pol),
        cos(-el)
    };

    // x-axis
    this->tracking_origin = glm::cross(
        this->rotation_axis,
        this->neutral_normal
    );

    return;
}

void ParabolicTrough::set_aperture_size(double size_x,
                                        double size_y)
{
    if (size_x <= 0.0 || size_y <= 0.0)
    {
        std::stringstream ss;
        ss << "ParabolicTrough::set_aperture_size: Invalid aperture "
           << "dimensions. size_x (" << size_x << ") and size_y ("
           << size_y << ") must be positive.";
        throw std::invalid_argument(ss.str());
    }

    this->initialized = false;
    this->aperture_size_x = size_x;
    this->aperture_size_y = size_y;

    return;
}

void ParabolicTrough::set_focal_length(double flen)
{
    if (flen <= 0.0)
    {
        std::stringstream ss;
        ss << "ParabolicTrough::set_focal_length: Invalid focal length ("
           << flen << "). Must be positive.";
        throw std::invalid_argument(ss.str());
    }

    this->initialized = false;
    this->focal_length = flen;
    this->cx = 0.5 / this->focal_length;

    return;
}

void ParabolicTrough::set_gaps(double gap_x,
                               double gap_y,
                               double gap_center)
{
    if (gap_x < 0.0 || gap_y < 0.0 || gap_center < 0.0)
    {
        std::stringstream ss;
        ss << "ParabolicTrough::set_gaps: Invalid gap dimensions. "
           << "gap_x (" << gap_x << "), gap_y (" << gap_y
           << "), and gap_center (" << gap_center
           << ") must be non-negative.";
        throw std::invalid_argument(ss.str());
    }

    this->initialized = false;
    this->gap_x = gap_x;
    this->gap_y = gap_y;
    this->gap_center = gap_center;

    return;
}

void ParabolicTrough::set_number_panels(int_fast64_t num_x,
                                        int_fast64_t num_y)
{
    if (num_x < 1 || num_y < 1)
    {
        std::stringstream ss;
        ss << "ParabolicTrough::set_number_panels: Invalid panel count. "
           << "num_x (" << num_x << ") and num_y (" << num_y
           << ") must be at least 1.";
        throw std::invalid_argument(ss.str());
    }

    if (num_x != 1 && num_x % 2 != 0)
    {
        std::stringstream ss;
        ss << "ParabolicTrough::set_number_panels: Invalid panel count "
           << "in x direction (" << num_x
           << "). Must be either 1 or an even number.";
        throw std::invalid_argument(ss.str());
    }

    this->initialized = false;
    this->num_panels_x = num_x;
    this->num_panels_y = num_y;

    return;
}

void ParabolicTrough::set_optics(const OpticalPropertySetReference mirror,
                                 const OpticalPropertySetReference absorber,
                                 const OpticalPropertySetReference envelope_inner,
                                 const OpticalPropertySetReference envelope_outer)
{
    this->optics_mirror = mirror;
    this->optics_absorber = absorber;
    this->optics_envelope_inner = envelope_inner;
    this->optics_envelope_outer = envelope_outer;
    return;
}

void ParabolicTrough::set_receiver_dimensions(double abs_diam,
                                              double env_diam,
                                              double env_thick)
{
    if (abs_diam <= 0.0)
    {
        std::stringstream ss;
        ss << "ParabolicTrough::set_receiver_dimensions: "
           << "Invalid absorber diameter (" << abs_diam
           << "). Must be positive.";
        throw std::invalid_argument(ss.str());
    }

    if (env_diam <= 0.0)
    {
        std::stringstream ss;
        ss << "ParabolicTrough::set_receiver_dimensions: "
           << "Invalid envelope diameter (" << env_diam
           << "). Must be positive.";
        throw std::invalid_argument(ss.str());
    }

    if (env_thick <= 0.0)
    {
        std::stringstream ss;
        ss << "ParabolicTrough::set_receiver_dimensions: "
           << "Invalid envelope thickness (" << env_thick
           << "). Must be positive.";
        throw std::invalid_argument(ss.str());
    }

    if (env_diam - 2 * env_thick < abs_diam)
    {
        std::stringstream ss;
        ss << "ParabolicTrough::set_receiver_dimensions: "
           << "Envelope inner diameter (" << (env_diam - 2 * env_thick)
           << ") is smaller than absorber diameter (" << abs_diam
           << "). Envelope is too small for the given absorber.";
        throw std::invalid_argument(ss.str());
    }

    this->initialized = false;
    this->absorber_diameter = abs_diam;
    this->envelope_diameter = env_diam;
    this->envelope_thickness = env_thick;

    return;
}

void ParabolicTrough::set_tracking_limits(double lower, double upper)
{
    if (lower > upper)
    {
        std::stringstream ss;
        ss << "ParabolicTrough::set_tracking_limits: Invalid tracking "
           << "limits. Lower limit (" << lower
           << ") must be less than or equal to upper limit ("
           << upper << ").";
        throw std::invalid_argument(ss.str());
    }

    this->tracking_limit_lower = lower;
    this->tracking_limit_upper = upper;

    return;
}

double ParabolicTrough::determine_x_coordinate(double x0,
                                               double arc_length)
{
    return parabolic_determine_x_coordinate(this->cx, x0, arc_length);
}

void ParabolicTrough::enforce_user_fields_set() const
{
    if (this->initialized)
    {
        // If we have created subelements, do composite element checks
        CompositeElement::enforce_user_fields_set();
    }

    // Validate that all required parameters have been set
    if (this->aperture_size_x <= 0.0 || this->aperture_size_y <= 0.0)
    {
        throw std::invalid_argument(
            "ParabolicTrough: Aperture size must be set "
            "before creating geometry.");
    }

    if (this->focal_length <= 0.0)
    {
        throw std::invalid_argument(
            "ParabolicTrough: Focal length must be set "
            "before creating geometry.");
    }

    if (this->num_panels_x <= 0 || this->num_panels_y <= 0)
    {
        throw std::invalid_argument(
            "ParabolicTrough: Number of panels must be set "
            "before creating geometry.");
    }

    if (this->absorber_diameter <= 0.0 ||
        this->envelope_diameter <= 0.0 ||
        this->envelope_thickness <= 0.0)
    {
        throw std::invalid_argument(
            "ParabolicTrough: Receiver dimensions must be set "
            "before creating geometry.");
    }

    return;
}

} // namespace SolTrace::Data
