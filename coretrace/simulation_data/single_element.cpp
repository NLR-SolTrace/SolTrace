
#include "single_element.hpp"

#include <exception>
#include <memory>
#include <sstream>

#include "aperture.hpp"
#include "element.hpp"

namespace SolTrace::Data {

SingleElement::SingleElement(ElementKey key) : ElementBase(key),
                                               aperture(nullptr),
                                               surface(nullptr),
                                               group(-1)
{
    return;
}

SingleElement::SingleElement(ElementKey key,
    const nlohmann::ordered_json& jnode,
    const OpticalPropertySetResolver& resolve_optics) : ElementBase(key, jnode),
                                                        aperture(nullptr),
                                                        surface(nullptr)
{
    this->set_aperture(Aperture::make_aperture_from_json(jnode.at("aperture")));
    this->set_surface(make_surface_from_json(jnode.at("surface")));

    const optics_id opt_id = jnode.at("opt_id").get<optics_id>();
    this->set_optical_property_set(resolve_optics(opt_id));

    // don't need to require every element have a group
    if (jnode.contains("group"))
    {
        this->group = jnode.at("group").get<int32_t>();
        // if user set a single element to something < -1
        // it falls out of ungrouped -> == -1 or grouped -> >= 0
        // assume user meant ungrouped and force to = -1
        if (this->group < -1) this->group = -1;
    } 
    else
    {
        this->group = -1;
    }
}

SingleElement::~SingleElement()
{
    this->aperture = nullptr;
    this->surface = nullptr;
    return;
}

void SingleElement::set_optical_property_set(const OpticsRef& optics)
{
    if (optics.is_assigned() && !optics.usable_in(this->get_model_token()))
    {
        std::stringstream ss;
        ss << "Element (Name: " << this->get_name()
           << ", UUID: " << this->get_id()
           << "): optical property set " << optics.id()
           << " belongs to another model, or to this model before it was "
              "cleared.";
        throw std::invalid_argument(ss.str());
    }
    this->optics = optics;
}

void SingleElement::enforce_user_fields_set() const
{
    ElementBase::enforce_user_fields_set();

    if (this->aperture == nullptr)
    {
        std::stringstream ss;
        ss << "Element (Name: " << this->get_name()
           << ", UUID: " << this->get_id()
           << ") has no aperture.";
        throw std::invalid_argument(ss.str());
    }

    if (this->surface == nullptr)
    {
        std::stringstream ss;
        ss << "Element (Name: " << this->get_name()
           << ", UUID: " << this->get_id()
           << ") has no surface.";
        throw std::invalid_argument(ss.str());
    }

    // Also catches a reference that expired after it was assigned
    if (!this->optics.is_assigned() ||
        !this->optics.usable_in(this->get_model_token()) ||
        !this->optics.valid())
    {
        std::stringstream ss;
        ss << "Element (Name: " << this->get_name()
            << ", UUID: " << this->get_id()
            << ") has no valid optical property set assigned.";
        throw std::invalid_argument(ss.str());
    }

    return;
}

void SingleElement::write_json(nlohmann::ordered_json& jnode) const
{
    using json = nlohmann::ordered_json;
    
    // Write shared properties
    this->write_common_json(jnode);
    
    // Optical Properties
    jnode["opt_id"] = this->optics.id();

    // Aperture
    json japerture;
    this->aperture->write_json(japerture);
    jnode["aperture"] = japerture;

    // Surface
    json jsurface;
    this->surface->write_json(jsurface);
    jnode["surface"] = jsurface;

    // Write element type
    jnode["is_single"] = true;
}

} // namespace SolTrace::Data
