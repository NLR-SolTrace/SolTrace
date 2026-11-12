/**
 * @file model_context.hpp
 * @brief The way elements are created for a model: context, passkey, handle
 *
 * A ModelContext creates elements for one model. Each new element is returned
 * in a PendingElement handle and is registered only when the handle is
 * committed, and only with the model it was created for.
 */

#ifndef SOLTRACE_MODEL_CONTEXT_H
#define SOLTRACE_MODEL_CONTEXT_H

#include <cassert>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include "element_id.hpp"
#include "model_identity.hpp"
#include "model_state.hpp"

namespace SolTrace::Data {

template <class T>
class PendingElement;

// Cheap to copy. Holds a weak reference to the state of one model, so it does
// not keep the model alive.
class ModelContext
{
public:
    explicit ModelContext(const std::shared_ptr<ModelState> &state)
        : state_(state)
    {
    }

    // Creates a `T`, issuing the next element ID. `T` must have a constructor
    // that takes an ElementKey followed by `args`.
    // Throws std::invalid_argument if the model no longer exists.
    template <class T, class... Args>
    [[nodiscard]] PendingElement<T> create(Args &&...args);

    // Creates a `T` with an ID that was saved elsewhere. Throws
    // std::invalid_argument if `id` is below 1 or already in use, or if the
    // model no longer exists.
    template <class T, class... Args>
    [[nodiscard]] PendingElement<T> create_with_id(element_id id,
                                                   Args &&...args);

    // False if the model no longer exists.
    bool is_registered(element_id id) const
    {
        auto state = this->state_.lock();
        return state != nullptr && state->is_registered &&
               state->is_registered(id);
    }

    // The model's current identity, or an expired token if it is gone.
    ModelToken token() const
    {
        auto state = this->state_.lock();
        return state != nullptr ? state->token() : ModelToken();
    }

private:
    template <class>
    friend class PendingElement;

    std::shared_ptr<ModelState> lock() const
    {
        auto state = this->state_.lock();
        if (state == nullptr)
        {
            throw std::invalid_argument(
                "ModelContext: the model no longer exists");
        }
        return state;
    }

    template <class T, class... Args>
    PendingElement<T> build(element_id id,
                            const ModelToken &token,
                            Args &&...args);

    std::weak_ptr<ModelState> state_;
};

// Passed to an element's constructor so that an element cannot be built
// without a model: only a ModelContext can make one.
class ElementKey
{
public:
    element_id id() const { return this->id_; }
    const ModelContext &context() const { return this->context_; }
    const ModelToken &token() const { return this->token_; }

private:
    friend class ModelContext;

    ElementKey(element_id id, ModelContext context, ModelToken token)
        : id_(id), context_(std::move(context)), token_(std::move(token))
    {
    }

    element_id id_;
    ModelContext context_;
    ModelToken token_;
};

// Owns a new element until it is committed. Move-only.
template <class T>
class PendingElement
{
public:
    PendingElement(const PendingElement &) = delete;
    PendingElement &operator=(const PendingElement &) = delete;

    PendingElement(PendingElement &&other) noexcept = default;
    PendingElement &operator=(PendingElement &&other) noexcept = default;

    // For example, PendingElement<SingleElement> to PendingElement<Element>.
    template <class U,
              class = std::enable_if_t<std::is_convertible_v<U *, T *>>>
    PendingElement(PendingElement<U> &&other) noexcept
        : element_(std::move(other.element_)),
          context_(std::move(other.context_)),
          token_(std::move(other.token_)),
          id_(other.id_),
          committed_(other.committed_)
    {
    }

    T *operator->()
    {
        assert(this->element_ != nullptr && "empty PendingElement");
        return this->element_.get();
    }

    T &operator*()
    {
        assert(this->element_ != nullptr && "empty PendingElement");
        return *this->element_;
    }

    std::shared_ptr<T> get() const { return this->element_; }

    // Known before the element is committed.
    element_id id() const { return this->id_; }

    // Registers the element with the model it was created for and returns its
    // ID. Throws std::invalid_argument if the handle is empty or already
    // committed, if the model is gone or was cleared after this element was
    // created, if the model cannot register elements, or if registration
    // fails. Nothing is registered when it throws, and a handle that failed
    // can be committed again.
    element_id commit()
    {
        if (this->element_ == nullptr)
        {
            throw std::invalid_argument(
                "PendingElement::commit: the handle is empty");
        }
        if (this->committed_)
        {
            throw std::invalid_argument(
                "PendingElement::commit: the element was already committed");
        }

        auto state = this->context_.state_.lock();
        if (state == nullptr)
        {
            throw std::invalid_argument(
                "PendingElement::commit: the model no longer exists");
        }
        if (!same_model(this->token_, state->token()))
        {
            throw std::invalid_argument(
                "PendingElement::commit: the model was cleared after this "
                "element was created");
        }
        if (!state->registrar)
        {
            throw std::invalid_argument(
                "PendingElement::commit: the model cannot register elements");
        }

        const element_id id = state->registrar(this->element_);
        this->committed_ = true;
        return id;
    }

private:
    friend class ModelContext;
    template <class>
    friend class PendingElement;

    PendingElement(std::shared_ptr<T> element,
                   ModelContext context,
                   ModelToken token,
                   element_id id)
        : element_(std::move(element)),
          context_(std::move(context)),
          token_(std::move(token)),
          id_(id)
    {
    }

    std::shared_ptr<T> element_;
    ModelContext context_;
    ModelToken token_;
    element_id id_ = ELEMENT_ID_UNASSIGNED;
    bool committed_ = false;
};

template <class T, class... Args>
PendingElement<T> ModelContext::build(element_id id,
                                      const ModelToken &token,
                                      Args &&...args)
{
    ElementKey key(id, *this, token);
    auto element = std::make_shared<T>(key, std::forward<Args>(args)...);
    return PendingElement<T>(std::move(element), *this, token, id);
}

template <class T, class... Args>
PendingElement<T> ModelContext::create(Args &&...args)
{
    auto state = this->lock();
    const element_id id = state->ids.next();
    return this->build<T>(id, state->token(), std::forward<Args>(args)...);
}

template <class T, class... Args>
PendingElement<T> ModelContext::create_with_id(element_id id, Args &&...args)
{
    auto state = this->lock();
    state->ids.reserve(id);
    return this->build<T>(id, state->token(), std::forward<Args>(args)...);
}

} // namespace SolTrace::Data

#endif
