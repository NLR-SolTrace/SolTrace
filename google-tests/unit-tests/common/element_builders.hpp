#ifndef SOLTRACE_UNITTESTS_ELEMENT_BUILDERS_H
#define SOLTRACE_UNITTESTS_ELEMENT_BUILDERS_H

#include <functional>
#include <map>
#include <memory>
#include <utility>

#include <simulation_data_export.hpp>

// Tests build elements through these functions instead of calling the
// creation, registration, and attach API directly, so a change to that API
// only has to be made here. Every function takes the model the element belongs
// to.
//
// An element is created in the model with `sd.create_element`, which returns a
// handle. The tests configure the element through a plain pointer and register
// or attach it later, so these functions keep the handle until it is used:
// `register_element` commits it and `attach_child` hands it to the parent.

namespace element_builders_detail {

struct Pending
{
    std::function<element_id()> commit;
    std::function<element_id(CompositeElement &)> attach;
};

inline std::map<const Element *, Pending> &pending()
{
    static std::map<const Element *, Pending> handles;
    return handles;
}

} // namespace element_builders_detail

// Creates an element of type `T` for the model `sd`. The extra arguments are
// passed to the element's constructor.
template <class T, class... Args>
std::shared_ptr<T> new_element(SimulationData &sd, Args &&...args)
{
    auto handle = std::make_shared<PendingElement<T>>(
        sd.create_element<T>(std::forward<Args>(args)...));
    std::shared_ptr<T> el = handle->get();
    element_builders_detail::pending()[el.get()] = {
        [handle]() { return handle->commit(); },
        [handle](CompositeElement &parent)
        { return parent.add_child(std::move(*handle)); }};
    return el;
}

// Creates a stage for the model `sd`.
template <class... Args>
std::shared_ptr<StageElement> new_stage(SimulationData &sd, Args &&...args)
{
    return new_element<StageElement>(sd, std::forward<Args>(args)...);
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
// element's ID, or ELEMENT_ALREADY_REGISTERED if it was registered or attached
// before. Throws std::invalid_argument if it cannot be registered, and then
// the element can be fixed and registered again.
inline element_id register_element(SimulationData & /* sd */,
                                   const element_ptr &el)
{
    auto &handles = element_builders_detail::pending();
    auto it = handles.find(el.get());
    if (it == handles.end())
    {
        return ELEMENT_ALREADY_REGISTERED;
    }
    const element_id id = it->second.commit();
    handles.erase(it);
    return id;
}

// Registers a stage with the model. Returns the stage's ID, or
// ELEMENT_ALREADY_REGISTERED if it was registered before.
inline element_id register_stage(SimulationData &sd, const element_ptr &stage)
{
    return register_element(sd, stage);
}

// Attaches `child` to a composite element or stage. Returns the child's key in
// the parent, or a negative status, and leaves `child` unattached on failure.
template <class Parent>
element_id attach_child(const Parent &parent, const element_ptr &child)
{
    auto &handles = element_builders_detail::pending();
    auto it = handles.find(child.get());
    if (it == handles.end())
    {
        return ELEMENT_ALREADY_REGISTERED;
    }
    const element_id id = it->second.attach(*parent);
    if (!Element::is_success(id))
    {
        return id;
    }
    handles.erase(it);

    for (auto iter = parent->get_const_iterator(); !parent->is_at_end(iter);
         ++iter)
    {
        if (iter->second == child)
        {
            return iter->first;
        }
    }
    return ELEMENT_ERROR;
}

#endif
