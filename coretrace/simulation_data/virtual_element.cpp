
#include "virtual_element.hpp"

namespace SolTrace::Data {


VirtualElement::VirtualElement(ElementKey key)
    : SingleElement(key)
{
    this->set_optical_property_set(virtual_optics());
}

VirtualElement::VirtualElement(ElementKey key,
    const nlohmann::ordered_json& jnode,
    const OpticalPropertySetResolver& resolve_optics)
    : SingleElement(key, jnode, resolve_optics)
{
    this->set_optical_property_set(virtual_optics());
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
