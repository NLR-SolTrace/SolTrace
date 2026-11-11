#ifndef SOLTRACE_UNITTESTS_ELEMENT_BUILDERS_H
#define SOLTRACE_UNITTESTS_ELEMENT_BUILDERS_H

#include <memory>
#include <utility>

#include <simulation_data_export.hpp>

// Tests build elements through these functions instead of calling the
// creation, registration, and attach API directly, so a change to that API
// only has to be made here. Every function takes the model the element belongs
// to. Creation does not use it yet, but it will once elements are created
// through a model.

// Creates an element of type `T` for the model `sd`. The extra arguments are
// passed to the element's constructor.
template <class T, class... Args>
std::shared_ptr<T> new_element(SimulationData & /* sd */, Args &&...args)
{
    return make_element<T>(std::forward<Args>(args)...);
}

// Creates a stage for the model `sd`.
template <class... Args>
auto new_stage(SimulationData & /* sd */, Args &&...args)
{
    return make_stage(std::forward<Args>(args)...);
}

// Gives `el` its aperture and surface.
inline void set_element_shape(SimulationData & /* sd */,
                              const element_ptr &el,
                              aperture_ptr ap,
                              surface_ptr surf)
{
    el->set_aperture(ap);
    el->set_surface(surf);
}

// Creates a single element with its aperture, surface, and optics set.
inline std::shared_ptr<SingleElement>
make_single(SimulationData &sd,
            aperture_ptr ap,
            surface_ptr surf,
            const OpticalPropertySetReference &optics)
{
    auto el = new_element<SingleElement>(sd);
    set_element_shape(sd, el, ap, surf);
    el->set_optical_property_set(optics);
    return el;
}

// Registers `el`, and the elements it contains, with the model. Returns the
// element's ID, or a negative status.
inline element_id register_element(SimulationData &sd, const element_ptr &el)
{
    return sd.add_element(el);
}

// Registers a stage with the model. Returns the stage's ID, or a negative
// status.
inline element_id register_stage(SimulationData &sd, const element_ptr &stage)
{
    return sd.add_stage(stage);
}

// Attaches `child` to a composite element or stage. Returns the child's key in
// the parent, or a negative status.
template <class Parent>
element_id attach_child(const Parent &parent, const element_ptr &child)
{
    return parent->add_element(child);
}

#endif
