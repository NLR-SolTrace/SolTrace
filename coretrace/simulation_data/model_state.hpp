/**
 * @file model_state.hpp
 * @brief State shared by a model, its contexts, and the elements created for it
 *
 * Has no dependency on SimulationData or on the element classes.
 */

#ifndef SOLTRACE_MODEL_STATE_H
#define SOLTRACE_MODEL_STATE_H

#include <functional>
#include <memory>
#include <set>

#include "element_id.hpp"
#include "model_identity.hpp"

namespace SolTrace::Data {

class Element;

// Issues element IDs for one model. IDs are 1 or greater, and an ID is never
// issued twice, even if the element that had it is removed.
class ElementIdAllocator
{
public:
    // The next ID, one above the highest ID issued or reserved so far. The
    // first is 1. Never fills a gap.
    // Throws std::overflow_error if no larger ID exists.
    element_id next();

    // Marks a saved ID as used, for loaders that restore IDs.
    // Throws std::invalid_argument if `id` is below 1 or already in use.
    void reserve(element_id id);

    // Forgets every ID. Used only when the model is cleared.
    void reset();

private:
    std::set<element_id> used_;
    element_id highest_ = 0;
};

class ModelState
{
public:
    ModelState();

    ElementIdAllocator ids;

    // Both are set by the SimulationData that owns this state. An empty
    // `registrar` means the model cannot register elements.
    std::function<element_id(std::shared_ptr<Element>)> registrar;
    std::function<bool(element_id)> is_registered;

    ModelToken token() const { return this->identity_; }

    // Gives the model a new identity and resets the ID allocator. Everything
    // created before this call no longer belongs to the model.
    void reset();

private:
    std::shared_ptr<const ModelIdentity> identity_;
};

} // namespace SolTrace::Data

#endif
