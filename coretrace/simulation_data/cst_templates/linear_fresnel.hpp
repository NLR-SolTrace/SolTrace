/**
 * @file linear_fresnel.hpp
 * @brief Linear Fresnel reflector CST template
 *
 * Defines the LinearFresnel class which generates linear Fresnel
 * reflector systems with configurable mirror panels, receiver geometry,
 * and tracking capabilities for concentrated solar thermal applications.
 * Supports both focused and flat panel configurations.
 *
 * @defgroup cst_templates CST Templates
 * @{
 */

#ifndef SOLTRACE_LINEAR_FRESNEL_H
#define SOLTRACE_LINEAR_FRESNEL_H

#include "composite_element.hpp"

#include <cstdint>
#include <vector>

#include "element.hpp"
#include "single_element.hpp"

namespace SolTrace::Data {

class LinearFresnel : public CompositeElement
{
public:
    explicit LinearFresnel(ElementKey key);
    ~LinearFresnel();

    void set_angles(double azimuth, double tilt);
    void set_aperture_size(double len_x, double len_y);
    void set_focused_panels(bool focused);
    void set_gaps(double gap_x, double gap_y, double gap_center);
    void set_number_panels(int_fast64_t num_x, int_fast64_t num_y);
    void set_optics(const OpticalPropertySetReference mirror,
                    const OpticalPropertySetReference absorber,
                    const OpticalPropertySetReference envelop_outer,
                    const OpticalPropertySetReference envelop_inner);
    void set_receiver_height(double height);
    void set_receiver_dimensions(double absorber_diameter,
                                 double envelop_diameter,
                                 double envelop_thickness);
    void set_tracking_limits(double lower, double upper);

    virtual void enforce_user_fields_set() const override;

    void create_geometry();
    void update_geometry(double azimuth, double elevation);

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

    const std::vector<single_element_ptr>& get_mirrors() const
    {
        return this->mirrors;
    }

    const std::vector<single_element_ptr>& get_absorbers() const
    {
        return this->absorbers;
    }

private:
    bool initialized;

    // Global Characteristics
    double receiver_height;
    double azimuth;
    double tilt;

    // Mirror Characteristics
    bool focused_panels;
    double aperture_size_x;
    double aperture_size_y;
    int_fast64_t num_panels_x;
    int_fast64_t num_panels_y;
    double gap_x;
    double gap_y;
    double gap_center;
    OpticalPropertySetReference optics_mirror;

    // Receiver Characteristics
    double abs_diameter;
    double env_diameter;
    double env_thickness;
    // double receiver_length;
    OpticalPropertySetReference optics_absorber;
    OpticalPropertySetReference optics_env_out;
    OpticalPropertySetReference optics_env_in;

    // Solar Tracking
    // double tracking_angle;
    double tracking_limit_lower;
    double tracking_limit_upper;
    glm::dvec3 rotation_axis;
    glm::dvec3 neutral_normal;
    glm::dvec3 tracking_origin;

    std::vector<single_element_ptr> mirrors;
    std::vector<single_element_ptr> absorbers;
    std::vector<single_element_ptr> envelope;
};

} // namespace SolTrace::Data

/**
 * @}
 */

#endif
