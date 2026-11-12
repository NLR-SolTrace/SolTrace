/**
 * @file sun.hpp
 * @brief Solar source modeling and sun shape definitions
 *
 * Defines solar source properties including sun shape models,
 * solar position calculations, and ray generation from solar disk.
 * Includes models for different sun shape distributions and
 * solar tracking calculations for CST systems.
 */

#ifndef SOLTRACE_SUN_H
#define SOLTRACE_SUN_H

#include <nlohmann/json.hpp>

#include "ray_source.hpp"

#include <glm/vec3.hpp>

namespace SolTrace::Data {

class Sun : public RaySource
{
public:
    Sun() : my_shape(SunShape::UNKNOWN),
            my_position(0.0),
            my_gen_type(GenType::RANDOM)
    { }

    Sun(const nlohmann::ordered_json& jnode);

    virtual ~Sun() {}

    virtual const glm::dvec3 &get_position() const override
    {
        return this->my_position;
    }
 
    virtual glm::dvec3 &get_position() override
    {
        return this->my_position;
    }
    virtual void set_position(const glm::dvec3 &pos) override
    {
        this->my_position = pos;
        return;
    }
    virtual void set_position(double x, double y, double z) override
    {
        this->my_position = glm::dvec3(x, y, z);
        return;
    }
    virtual SunShape get_shape() const override
    {
        return this->my_shape;
    }
    virtual void set_shape(SunShape shape,
                           double _sigma,
                           double _half_width,
                           double _csr,        
                           std::vector<double> _user_angle = {},
                           std::vector<double> _user_intensity = {}) override;
    virtual void calculate_buie_parameters(double& kappa, double& gamma) override;
    virtual double get_max_sun_angle(double gaussian_coverage = 0.999) const override;  //  [mrad]
    virtual double get_max_intensity() const override;
    virtual void set_gen_type(GenType type) override { my_gen_type = type; }
    virtual GenType get_gen_type() const override { return my_gen_type; }

    void write_json(nlohmann::ordered_json& jnode);

private:
    void set_gaussian_distribution(double _sigma);
    void set_pillbox_distribution(double _half_width);
    void set_buie_csr_distribution(double _csr);
    void set_user_defined_distribution(std::vector<double> _user_angle,
                                       std::vector<double> _user_intensity);

    SunShape my_shape;
    glm::dvec3 my_position;
    GenType my_gen_type;
};

} // namespace SolTrace::Data

#endif
