/**
 * @file single_element.hpp
 * @brief Single optical element implementation
 *
 * Defines the SingleElement class for individual optical surfaces
 * with associated apertures, surfaces, and optical properties.
 * Represents the fundamental building block of optical systems
 * in SolTrace ray tracing simulations.
 *
 * @defgroup elements Optical Elements
 * @{
 */

#ifndef SOLTRACE_PLANE_H
#define SOLTRACE_PLANE_H

#include "element.hpp"

#include <memory>

#include "aperture.hpp"
#include "surface.hpp"

namespace SolTrace::Data {

class SingleElement : public ElementBase
{
public:
    explicit SingleElement(ElementKey key);
    SingleElement(ElementKey key,
        const nlohmann::ordered_json& jnode,
        const OpticalPropertySetResolver& resolve_optics);
    virtual ~SingleElement();

    virtual bool is_single() const override { return true; }

    virtual const aperture_ptr get_aperture() const override
    {
        return this->aperture;
    }
    virtual aperture_ptr get_aperture() override
    {
        return this->aperture;
    }
    virtual void set_aperture(aperture_ptr ap) override
    {
        this->aperture = ap;
        return;
    }

    virtual const surface_ptr get_surface() const override
    {
        return this->surface;
    }
    virtual surface_ptr get_surface() override
    {
        return this->surface;
    }
    virtual void set_surface(surface_ptr sp) override
    {
        this->surface = sp;
        return;
    }



    optics_id get_optical_property_set_id() const override
    {
        return this->optics.id();
    }

    std::shared_ptr<const OpticalPropertySet> get_optical_property_set() const override
    {
        return this->optics.get();
    }

    // Throws std::invalid_argument if `optics` belongs to another model, or
    // to this model before it was cleared. An unassigned reference removes
    // the element's optics.
    void set_optical_property_set(const OpticsRef& optics) override;

    /*const OpticalProperties *get_back_optical_properties() const override
    {
        return &(this->optics_back);
    }
    OpticalProperties *get_back_optical_properties() override
    {
        return &(this->optics_back);
    }
    void set_back_optical_properties(const OpticalProperties &op) override
    {
        this->optics_back = op;
    }*/

    virtual int32_t get_group() const override { return this->group; }

    virtual void enforce_user_fields_set() const override;

    virtual void write_json(nlohmann::ordered_json& jnode) const override;

protected:
    aperture_ptr aperture;
    surface_ptr surface;

    OpticsRef optics;

    // group number of element. 
    // -2 is an element that does not interact with the trace, ie stage or composite elements. 
    // -1 is an ungrouped element, assigned explicitly or if "group" key is missing from JSON. 
    // 0 and above are grouped elements. 
    int32_t group;
};

using single_element_ptr = typename std::shared_ptr<SingleElement>;

} // namespace SolTrace::Data

/**
 * @}
 */

#endif
