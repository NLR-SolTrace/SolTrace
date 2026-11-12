
#include "virtual_element.hpp"

namespace SolTrace::Data {


VirtualElement::VirtualElement(ElementKey key)
    : SingleElement(key)
{
    auto optics = std::make_shared<OpticalPropertySet>(
        InteractionType::REFRACTION,
        1.0,
        1.0,
        "VirtualProp");

    optics->set_properties(
        OpticalSide::Both,
        DistributionType::NONE,
        1.0,
        0.0,
        0.0,
        0.0);

    this->owned_optical_property_set = optics;
    OpticalPropertySetReference optics_ref = { OPTICS_ID_VIRTUAL, 
        this->owned_optical_property_set };

    this->set_optical_property_set(optics_ref);
}

VirtualElement::VirtualElement(ElementKey key,
    const nlohmann::ordered_json& jnode,
    const OpticalPropertySetResolver& resolve_optics)
    : SingleElement(key, jnode, resolve_optics)
{
    auto optics = std::make_shared<OpticalPropertySet>(
        InteractionType::REFRACTION,
        1.0,
        1.0,
        "VirtualProp");

    optics->set_properties(
        OpticalSide::Both,
        DistributionType::NONE,
        1.0,
        0.0,
        0.0,
        0.0);

    this->owned_optical_property_set = optics;
    OpticalPropertySetReference optics_ref = { OPTICS_ID_VIRTUAL,
        this->owned_optical_property_set };

    this->set_optical_property_set(optics_ref);
}

VirtualElement::~VirtualElement()
{
}

VirtualPlane::VirtualPlane(ElementKey key, double x_len, double y_len)
    : VirtualElement(key)
{
    this->aperture = make_aperture<Rectangle>(x_len, y_len);
    this->surface = make_surface<Flat>();
}

VirtualPlane::~VirtualPlane()
{
}

} // namespace SolTrace::Data
