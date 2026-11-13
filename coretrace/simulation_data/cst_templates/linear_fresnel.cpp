#include "linear_fresnel.hpp"

#include <stdexcept>
#include <sstream>

#include "aperture.hpp"
#include "constants.hpp"
#include "element.hpp"
#include "surface.hpp"
#include "utilities.hpp"

namespace SolTrace::Data
{

    LinearFresnel::LinearFresnel()
        : CompositeElement(),
          initialized(false),
          receiver_height(-1.0),
          azimuth(-1.0),
          tilt(-1.0),
          focused_panels(false),
          aperture_size_x(-1.0),
          aperture_size_y(-1.0),
          num_panels_x(-1),
          num_panels_y(-1),
          gap_x(-1.0),
          gap_y(-1.0),
          gap_center(-1.0),
          abs_diameter(-1.0),
          env_diameter(-1.0),
          env_thickness(-1.0),
          // tracking_angle(0.0),
          tracking_limit_lower(-180.0),
          tracking_limit_upper(180.0)
    {
        this->tracking_origin = {1.0, 0.0, 0.0};
        this->rotation_axis = {0.0, 1.0, 0.0};
        this->neutral_normal = {0.0, 0.0, 1.0};
    }

    LinearFresnel::~LinearFresnel()
    {
        // Clear vectors
        this->mirrors.clear();
        this->absorbers.clear();
        this->envelope.clear();

        return;
    }

    // double LinearFresnel::get_tracking_angle_degrees() const
    // {
    //     return this->tracking_angle;
    // }

    // double LinearFresnel::get_tracking_angle_radians() const
    // {
    //     return D2R * this->get_tracking_angle_degrees();
    // }

    void LinearFresnel::set_angles(double azimuth, double tilt)
    {
        if (azimuth < -180.0 || azimuth > 180.0)
        {
            std::stringstream ss;
            ss << "LinearFresnel::set_angles: Invalid azimuth angle ("
               << azimuth << "). Must be between -180 and 180 degrees.";
            throw std::invalid_argument(ss.str());
        }

        if (tilt < 0.0 || tilt > 90.0)
        {
            std::stringstream ss;
            ss << "LinearFresnel::set_angles: Invalid tilt angle ("
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
        this->rotation_axis = { sin(inc) * cos(pol),
                                       sin(inc) * sin(pol),
                                       cos(inc) };

        // z-axis
        this->neutral_normal = { sin(-el) * cos(pol),
                                        sin(-el) * sin(pol),
                                        cos(-el) };

        this->tracking_origin = glm::cross(
            this->rotation_axis,
            this->neutral_normal
        );

        return;
    }

    void LinearFresnel::set_aperture_size(double len_x, double len_y)
    {
        if (len_x <= 0.0 || len_y <= 0.0)
        {
            std::stringstream ss;
            ss << "LinearFresnel::set_aperture_size: Invalid aperture dimensions. "
               << "len_x (" << len_x << ") and len_y (" << len_y
               << ") must be positive.";
            throw std::invalid_argument(ss.str());
        }

        this->initialized = false;
        this->aperture_size_x = len_x;
        this->aperture_size_y = len_y;

        return;
    }

    void LinearFresnel::set_focused_panels(bool focused)
    {
        this->initialized = false;
        this->focused_panels = focused;
        return;
    }

    void LinearFresnel::set_gaps(double gap_x,
                                 double gap_y,
                                 double gap_center)
    {
        if (gap_x < 0.0 || gap_y < 0.0 || gap_center < 0.0)
        {
            std::stringstream ss;
            ss << "LinearFresnel::set_gaps: Invalid gap dimensions. "
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

    void LinearFresnel::set_number_panels(int_fast64_t num_x,
                                          int_fast64_t num_y)
    {
        if (num_x <= 0 || num_y <= 0)
        {
            std::stringstream ss;
            ss << "LinearFresnel::set_number_panels: Invalid panel count. "
               << "num_x (" << num_x << ") and num_y (" << num_y
               << ") must be positive.";
            throw std::invalid_argument(ss.str());
        }

        this->initialized = false;
        this->num_panels_x = num_x;
        this->num_panels_y = num_y;

        return;
    }

    void LinearFresnel::set_optics(const OpticalPropertySetReference mirror,
                                   const OpticalPropertySetReference absorber,
                                   const OpticalPropertySetReference envelop_outer,
                                   const OpticalPropertySetReference envelop_inner)
    {
        this->optics_mirror = mirror;
        this->optics_absorber = absorber;
        this->optics_env_out = envelop_outer;
        this->optics_env_in = envelop_inner;
        return;
    }

    void LinearFresnel::set_receiver_height(double height)
    {
        if (height <= 0.0)
        {
            std::stringstream ss;
            ss << "LinearFresnel::set_receiver_height: Invalid receiver height ("
               << height << "). Height must be positive.";
            throw std::invalid_argument(ss.str());
        }

        this->initialized = false;
        this->receiver_height = height;

        return;
    }

    void LinearFresnel::set_receiver_dimensions(double absorber_diameter,
                                                double envelop_diameter,
                                                double envelop_thickness)
    {
        if (absorber_diameter <= 0.0)
        {
            std::stringstream ss;
            ss << "LinearFresnel::set_receiver_dimensions: "
               << "Invalid absorber diameter (" << absorber_diameter
               << "). Must be positive.";
            throw std::invalid_argument(ss.str());
        }

        if (envelop_diameter <= 0.0)
        {
            std::stringstream ss;
            ss << "LinearFresnel::set_receiver_dimensions: "
               << "Invalid envelope diameter (" << envelop_diameter
               << "). Must be positive.";
            throw std::invalid_argument(ss.str());
        }

        if (envelop_thickness < 0.0)
        {
            std::stringstream ss;
            ss << "LinearFresnel::set_receiver_dimensions: "
               << "Invalid envelope thickness (" << envelop_thickness
               << "). Must be non-negative.";
            throw std::invalid_argument(ss.str());
        }

        if (absorber_diameter >= envelop_diameter)
        {
            std::stringstream ss;
            ss << "LinearFresnel::set_receiver_dimensions: "
               << "Absorber diameter (" << absorber_diameter
               << ") must be smaller than envelope diameter ("
               << envelop_diameter << ").";
            throw std::invalid_argument(ss.str());
        }

        this->initialized = false;
        this->abs_diameter = absorber_diameter;
        this->env_diameter = envelop_diameter;
        this->env_thickness = envelop_thickness;

        return;
    }

    void LinearFresnel::set_tracking_limits(double lower, double upper)
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

    void LinearFresnel::create_geometry()
    {
        if (this->initialized)
        {
            return;
        }

        // TODO: Does receiver height give the height to the center or bottom edge?
        // Currently assumes it is the center.

        this->enforce_user_fields_set();

        this->mirrors.clear();
        this->absorbers.clear();
        this->envelope.clear();
        this->clear();

        // Create mirrors
        if (this->num_panels_x == 1 && this->gap_center != 0.0)
        {
            // TODO: This should generate a warning.
            this->gap_center = 0.0;
            this->gap_x = 0.0;
        }

        double panel_len_x = this->aperture_size_x -
                             this->gap_x * (this->num_panels_x - 1);
        panel_len_x -= this->gap_center - this->gap_x;
        panel_len_x /= this->num_panels_x;
        double panel_x = -0.5 * this->aperture_size_x + 0.5 * panel_len_x;

        double panel_len_y = this->aperture_size_y -
                             this->gap_y * (this->num_panels_y - 1);
        panel_len_y /= this->num_panels_y;

        element_id sts;
        glm::dvec3 origin;
        glm::dvec3 aim;
        glm::dvec3 receiver_pos(0.0,
                              0.0,
                              this->receiver_height);
        double panel_y, flen;
        single_element_ptr mirror;
        aperture_ptr ap;
        surface_ptr surf;
        glm::dvec3 khat(0.0, 0.0, 1.0);

        for (int_fast64_t i = 0; i < this->num_panels_x; ++i)
        {
            panel_y = -0.5 * this->aperture_size_y + 0.5 * panel_len_y;
            for (int_fast64_t j = 0; j < this->num_panels_y; ++j)
            {
                mirror = make_element<SingleElement>();

                origin = {panel_x, panel_y, 0.0};
                aim = 1.0 * receiver_pos + -1.0 * origin;
                // Project into xz-plane
                aim[1] = 0.0;
                aim = glm::normalize(aim);
                // std::cout << "Panel x: " << panel_x
                //           << "\nPanel y: " << panel_y
                //           << "\nRec: " << aim
                //           << std::endl;
                aim = 0.5 * khat + 0.5 * aim;
                // std::cout << "Aim: " << aim << std::endl;
                aim = 1.0 * origin + 1000.0 * aim;
                // std::cout << "Aim Point: " << aim << std::endl;
                // std::cout << "Origin: " << origin << std::endl;
                mirror->set_reference_frame_geometry(origin, aim, 0.0);

                ap = make_aperture<Rectangle>(panel_len_x, panel_len_y);
                mirror->set_aperture(ap);

                if (this->focused_panels)
                {
                    flen = sqrt(panel_x * panel_x +
                                this->receiver_height * this->receiver_height);
                    surf = make_surface<Parabola>(
                        Parabola::curvature_from_focal_length(flen), 0.0);
                }
                else
                {
                    surf = make_surface<Flat>();
                }
                mirror->set_surface(surf);

                mirror->set_optical_property_set(optics_mirror);
                mirror->enable();

                std::stringstream name;
                name << "Mirror_" << this->num_panels_y * i + j;
                mirror->set_name(name.str());

                this->mirrors.push_back(mirror);
                sts = this->add_element(mirror);
                if (!Element::is_success(sts))
                {
                    // TODO: Make this more helpful
                    throw std::runtime_error("Failed to add mirror element");
                }

                panel_y += panel_len_y + this->gap_y;
            }
            panel_x += panel_len_x;
            if (i == this->num_panels_x / 2 - 1)
            {
                panel_x += this->gap_center;
            }
            else
            {
                panel_x += this->gap_x;
            }
        }

        // Absorber
        // TODO: Single element at the moment. Break up into multiple tubes.
        auto abs = make_element<SingleElement>();
        abs->set_name("Absorber");
        origin = {
            0.0,
            0.0,
            this->receiver_height - 0.5 * this->abs_diameter
        };
        aim = {0.0, 0.0, 1.0};
        aim = 1.0 * origin + 1.0 * aim;
        abs->set_reference_frame_geometry(origin, aim, 0.0);
        abs->set_aperture(make_aperture<Rectangle>(this->abs_diameter,
                                                   this->aperture_size_y));
        abs->set_surface(make_surface<Cylinder>(0.5 * this->abs_diameter));
        abs->set_optical_property_set(optics_absorber);
        abs->enable();

        this->absorbers.push_back(abs);
        sts = this->add_element(abs);
        if (!Element::is_success(sts))
        {
            // TODO: Make this more helpful
            throw std::runtime_error("Failed to add absorber element");
        }

        // Envelope -- Outer
        auto envout = make_element<SingleElement>();
        envout->set_name("EnvelopeOuter");
        origin = {
            0.0,
            0.0,
            this->receiver_height - 0.5 * this->env_diameter
        };
        aim = {0.0, 0.0, 1.0};
        aim = 1.0 * origin + 1.0 * aim;
        envout->set_reference_frame_geometry(origin, aim, 0.0);
        envout->set_aperture(make_aperture<Rectangle>(this->env_diameter,
                                                      this->aperture_size_y));
        envout->set_surface(make_surface<Cylinder>(0.5 * this->env_diameter));
        envout->set_optical_property_set(optics_env_out);
        envout->enable();

        this->envelope.push_back(envout);
        sts = this->add_element(envout);
        if (!Element::is_success(sts))
        {
            throw std::runtime_error("Failed to add outer envelope element");
        }

        // Envelope -- Inner
        auto envin = make_element<SingleElement>();
        envin->set_name("EnvelopeInner");
        origin = {
            0.0,
            0.0,
            this->receiver_height -
                0.5 * this->env_diameter + this->env_thickness};
        aim = {0.0, 0.0, 1.0};
        aim = 1.0 * origin + 1.0 * aim;
        envin->set_reference_frame_geometry(origin, aim, 0.0);
        double ap_x = this->env_diameter - 2 * this->env_thickness;
        double ap_y = this->aperture_size_y;
        envin->set_aperture(make_aperture<Rectangle>(ap_x, ap_y));
        envin->set_surface(make_surface<Cylinder>(0.5 * ap_x));
        envin->set_optical_property_set(optics_env_in);
        envin->enable();
        this->envelope.push_back(envin);
        sts = this->add_element(envin);
        if (!Element::is_success(sts))
        {
            throw std::runtime_error("Failed to add inner envelope element");
        }

        this->initialized = true;

        return;
    }

    void LinearFresnel::update_geometry(double azimuth, double elevation)
    {
        if (elevation < 0.0 || elevation > 90.0)
        {
            std::stringstream ss;
            ss << "LinearFresnel::update_geometry: Invalid elevation ("
               << elevation << "). Elevation must lie between 0 and 90 degrees.";
            throw std::invalid_argument(ss.str());
        }

        if (azimuth < -180.0 || azimuth > 180.0)
        {
            std::stringstream ss;
            ss << "LinearFresnel::update_geometry: Invalid azimuth ("
               << elevation << "). Azimuth must lie between -180 and 180 degrees.";
            throw std::invalid_argument(ss.str());
        }

        if (!this->initialized)
        {
            std::stringstream ss;
            ss << "LinearFresnel::update_geometry: Uninitialized. "
               << "Call create_geometry() first.";
            throw std::invalid_argument(ss.str());
        }

        // NOTE: Setting the aim point and z-rotation only need to be done
        // once. But they need to be done after the LinearFresnel object
        // has been added to its reference element (if any) so we do it
        // here at the cost of repeating ourselves.

        // Set aim point
        glm::dvec3 z_axis_ref, y_axis_ref;
        this->convert_global_to_reference(z_axis_ref, this->neutral_normal);
        this->convert_global_to_reference(y_axis_ref, this->rotation_axis);
        this->aim = 1000.0 * z_axis_ref + 1.0 * this->origin;

        // Set z-rotation
        double beta = asin(z_axis_ref[1]);
        double gamma = acos(y_axis_ref[1] / cos(beta));
        this->set_zrot_radians(gamma);

        // Update coordinate conversions so we can use them below
        this->coordinates_initialized = false;
        this->compute_coordinate_rotations();

        // std::cout << "Rotation axis: " << this->rotation_axis
        //           << "\nNeutral normal: " << this->neutral_normal
        //           << std::endl;

        // std::cout << "Global to Local: " << this->get_global_to_local()
        //           << std::endl;

        // std::cout << "z axis ref: " << z_axis_ref
        //           << "\ny axis ref: " << y_axis_ref
        //           << "\nBeta: " << beta
        //           << "\nGamma: " << gamma
        //           << std::endl;

        // Sun position projected into rotation plane and converted
        // to LinearFresnel object coordinates
        glm::dvec3 sun_pos, sun_proj_local;
        sun_position_vector_degrees(sun_pos, azimuth, elevation);
        this->convert_vector_global_to_local(sun_proj_local, sun_pos);
        // Project to rotation plane
        sun_proj_local[1] = 0.0;
        sun_proj_local = glm::normalize(sun_proj_local);

        // std::cout << "Sun Position: " << sun_pos
        //           << "\nSun Proj Local: " << sun_proj_local
        //           << std::endl;

        // Set aimpoint for mirrors
        glm::dvec3 aim_mirror_ref;
        // Absorber position projected into the plane of rotation (the xz-plane)
        glm::dvec3 origin_abs_proj(0.0,
                                 0.0,
                                 this->receiver_height);
        // origin_abs_proj.make_unit();
        for (auto& iter : this->mirrors)
        {
            // Get mirror to to receiver vector
            aim_mirror_ref = origin_abs_proj - iter->get_origin_ref();

            // Project onto the rotation plane
            aim_mirror_ref[1] = 0.0;
            aim_mirror_ref = glm::normalize(aim_mirror_ref);
            // std::cout << "Origin: " << iter->get_origin_ref()
            //           << "\nMirror to Receiver: " << aim_mirror_ref
            //           << std::endl;

            // Take bisector vector with the sun
            aim_mirror_ref = sun_proj_local + aim_mirror_ref;
            aim_mirror_ref = glm::normalize(aim_mirror_ref);
            // std::cout << "Sun Proj Local: " << sun_proj_local
            //           << "\nAim Mirror Ref: " << aim_mirror_ref
            //           << std::endl;

            // Dot product with [0, 0, 1]
            double theta = acos(aim_mirror_ref[2]) * R2D;
            // Dot product with [1, 0, 0]
            if (aim_mirror_ref[0] < 0)
                theta = -theta;
            // std::cout << "Theta: " << theta << std::endl;

            if (theta < this->tracking_limit_lower)
            {
                theta = this->tracking_limit_lower * D2R;
                aim_mirror_ref = {sin(theta), 0.0, cos(theta)};
            }
            else if (theta > this->tracking_limit_upper)
            {
                theta = this->tracking_limit_upper * D2R;
                aim_mirror_ref = {sin(theta), 0.0, cos(theta)};
            }

            // std::cout << "Theta 2: " << theta * R2D
            //           << "\nAim Mirror Ref: " << aim_mirror_ref
            //           << std::endl;

            // Add origin of mirror
            aim_mirror_ref = iter->get_origin_ref() + 1000.0 * aim_mirror_ref;
            // aim_mirror_ref.scalar_mult(1000.0);

            // Set aim point
            iter->set_aim_vector(aim_mirror_ref);
            iter->compute_coordinate_rotations();
        }

        return;
    }

    void LinearFresnel::enforce_user_fields_set() const
    {
        if (this->initialized)
        {
            CompositeElement::enforce_user_fields_set();
        }

        // Validate that all required parameters have been set
        if (this->aperture_size_x <= 0.0 || this->aperture_size_y <= 0.0)
        {
            throw std::invalid_argument(
                "LinearFresnel: Aperture size must be set "
                "before creating geometry.");
        }

        if (this->num_panels_x <= 0 || this->num_panels_y <= 0)
        {
            throw std::invalid_argument(
                "LinearFresnel: Number of panels must be set "
                "before creating geometry.");
        }

        if (this->receiver_height <= 0.0)
        {
            throw std::invalid_argument(
                "LinearFresnel: Receiver height must be set "
                "before creating geometry.");
        }

        if (this->abs_diameter <= 0.0 ||
            this->env_diameter <= 0.0)
        {
            throw std::invalid_argument(
                "LinearFresnel: Receiver dimensions must be set "
                "before creating geometry.");
        }

        return;
    }

} // namespace SolTrace::Data
