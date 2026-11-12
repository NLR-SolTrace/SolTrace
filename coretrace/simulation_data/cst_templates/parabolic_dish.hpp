/**
 * @file parabolic_dish.hpp
 * @brief Parabolic dish CST template for dish-Stirling systems
 *
 * Defines the ParabolicDish class which generates parabolic dish
 * concentrator systems with configurable panel layouts, focal receivers,
 * and tracking capabilities for dish-Stirling solar applications.
 * Supports radial and angular panel subdivision strategies.
 *
 * @defgroup cst_templates CST Templates
 * @{
 */

#ifndef SOLTRACE_PARABOLIC_DISH_H
#define SOLTRACE_PARABOLIC_DISH_H

#include "composite_element.hpp"

#include <vector>

#include "single_element.hpp"

namespace SolTrace::Data {

class ParabolicDish : public CompositeElement
{
public:
    explicit ParabolicDish(ElementKey key);
    ~ParabolicDish();

    void set_aperture_size(double diameter);
    void set_focal_length(double flen);
    void set_gaps(double radial, double angular, double center_radius);
    void set_number_of_panels(int_fast64_t nradial, int_fast64_t nangular);
    void set_optics(const OpticalPropertySetReference &mirror,
                    const OpticalPropertySetReference &absorber);
    void set_receiver_dimensions(double diameter, double distance);
    void set_tracking_limits(double az_lower, double az_upper,
                             double el_lower, double el_upper);

    // Call before adding element to simulation data or stage
    void create_geometry();
    // Call after adding element to simulation data or stage
    void update_geometry(double azimuth, double elevation);

    virtual void enforce_user_fields_set() const override;

    const glm::dvec3 &get_elevation_axis() const
    {
        return this->elevation_axis;
    }

private:
    bool initialized;

    // Reflector Characteristics
    double aperture_diameter;
    double aperture_radius;
    double focal_length;
    double cx;
    double gap_r;
    double gap_a;
    double gap_center;
    int_fast64_t num_panels_r;
    int_fast64_t num_panels_a;
    OpticalPropertySetReference optics_mirror;

    // Receiver Characteristics
    double abs_diameter;
    double abs_distance;
    OpticalPropertySetReference optics_absorber;

    // Follows solar azimuth, elevation convention:
    // Azimuth is angle clockwise from north (y-axis) in degrees
    // Elevation is angle above the horizon (xy-plane) in degrees
    double tracking_elevation;
    double tracking_azimuth;
    // Defines the direction about which the dish can change its elevation
    glm::dvec3 elevation_axis;
    glm::dvec3 sun_position;

    std::vector<single_element_ptr> mirrors;
    std::vector<single_element_ptr> absorbers;

    double determine_x_coordinate(double x0, double arc_length);
};

} // namespace SolTrace::Data

/**
 * @}
 */

#endif
