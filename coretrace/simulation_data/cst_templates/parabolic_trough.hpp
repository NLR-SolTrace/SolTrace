/**
 * @file parabolic_trough.hpp
 * @brief Parabolic trough CST template for linear concentrator systems
 *
 * Defines the ParabolicTrough class which generates parabolic trough
 * solar collector systems with configurable mirror panels, receiver tubes,
 * and single-axis tracking capabilities. Supports various collector
 * configurations including heat collection elements and glass envelopes.
 *
 * @defgroup cst_templates CST Templates
 * @{
 */

#ifndef SOLTRACE_PARABOLIC_TROUGH_H
#define SOLTRACE_PARABOLIC_TROUGH_H

#include <cstdint>

#include <vector>

// #include "aperture.hpp"
#include "composite_element.hpp"
#include "element.hpp"
#include "optical_properties.hpp"
#include "simulation_data.hpp"
#include "single_element.hpp"
// #include "surface.hpp"

namespace SolTrace::Data {

class ParabolicTrough : public CompositeElement
{
public:
    explicit ParabolicTrough(ElementKey key);
    ~ParabolicTrough();

    // void disable() { this->elements->disable(); }
    // void enable() { this->elements->enable(); }

    // element_id add_self(stage_ptr stage);
    // element_id add_self(SimulationData &sd);
    // void remove_self(SimulationData &sd);

    void create_geometry();
    void update_geometry(double azimuth, double elevation);

    double calculate_receiver_power();

    void set_angles(double azimuth, double tilt);
    void set_aperture_size(double size_x, double size_y);
    void set_focal_length(double flen);
    void set_gaps(double gap_x, double gap_y, double gap_center);
    void set_number_panels(int_fast64_t num_x, int_fast64_t num_y);
    void set_optics(const OpticalPropertySetReference mirror,
        const OpticalPropertySetReference absorber,
        const OpticalPropertySetReference envelope_inner,
        const OpticalPropertySetReference envelope_outer);
    // void set_position(const glm::dvec3 &pos)
    // {
    //     this->position = pos;
    //     return;
    // }
    void set_receiver_dimensions(double abs_diam,
                                 double env_diam,
                                 double env_thick);
    void set_tracking_limits(double lower, double upper);

    virtual void enforce_user_fields_set() const override;

    glm::dvec3 get_tracking_origin() const
    {
        return this->tracking_origin;
    }
    glm::dvec3 get_rotation_vector() const
    {
        return this->rotation_axis;
    }
    glm::dvec3 get_neutral_normal() const
    {
        return this->neutral_normal;
    }
    glm::dvec3 get_tracking_limit_lower() const
    {
        return this->vector_lower_limit;
    }
    glm::dvec3 get_tracking_limit_upper() const
    {
        return this->vector_upper_limit;
    }

    double get_tracking_angle_degrees() const;
    double get_tracking_angle_radians() const;

private:
    // Flags
    bool initialized;

    // Global Characteristics
    // Degrees from north (GLOBAL y-axis)
    double azimuth;
    // Degrees from ground plane (GLOBAL z-axis)
    double tilt;
    // Aperture normal when tracking angle is 0.0 (GLOBAL coordinates)
    glm::dvec3 tracking_origin;
    // Axis trough rotates about (GLOBAL coordinates)
    glm::dvec3 rotation_axis;
    // Aperture normal when tracking angle is 90.0 (GLOBAL coordinates)
    glm::dvec3 neutral_normal;

    // Reflector(s) Characteristic(s)
    double aperture_size_x;
    double aperture_size_y;
    double focal_length;
    double cx;
    double gap_x;
    double gap_y;
    double gap_center;
    int_fast64_t num_panels_x;
    int_fast64_t num_panels_y;
    OpticalPropertySetReference optics_mirror;

    // Receiver Characteristics
    double absorber_diameter;
    double envelope_diameter;
    double envelope_thickness;
    // double length;
    OpticalPropertySetReference optics_absorber;
    OpticalPropertySetReference optics_envelope_inner;
    OpticalPropertySetReference optics_envelope_outer;

    // Solar Tracking
    double tracking_angle;
    double tracking_limit_lower;
    double tracking_limit_upper;
    // Aperture normal at lower limit in global coordinate
    glm::dvec3 vector_lower_limit;
    // Aperture normal at upper limit in global coordinate
    glm::dvec3 vector_upper_limit;

    // Element Management
    // composite_element_ptr elements;
    std::vector<single_element_ptr> absorbers;
    std::vector<single_element_ptr> envelopes;
    std::vector<single_element_ptr> mirrors;

    double determine_x_coordinate(double x0, double arc_length);
};

} // namespace SolTrace::Data

/**
 * @}
 */

#endif
