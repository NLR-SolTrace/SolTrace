/**
 * @file resource_registry.hpp
 * @brief Shared resources owned by a model, and references to them
 *
 * A ResourceRegistry stores values, such as optical property sets, that many
 * elements can share. Elements hold a ResourceRef, which identifies the entry
 * and the model it belongs to.
 */

#ifndef SOLTRACE_RESOURCE_REGISTRY_H
#define SOLTRACE_RESOURCE_REGISTRY_H

#include <cstdint>
#include <limits>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <utility>

#include "container.hpp"
#include "model_identity.hpp"

namespace SolTrace::Data {

using resource_id = std::int_fast64_t;

// The ID of a reference that has never been assigned to a resource
inline constexpr resource_id RESOURCE_ID_UNASSIGNED = -2;

class SimulationData;

template <class T>
class ResourceRegistry;

// Empty. A registry owns one for each entry, and references to the entry keep
// a weak reference to it, so that removing the entry expires them even while
// other code still holds the value.
struct ResourceEntryLife
{
};

// Read-only reference to one entry of a model's registry, or to a built-in
// value that every model accepts. Cheap to copy. Does not keep the entry
// alive: it expires when the entry is removed or the model is cleared or
// destroyed, even if other code still holds the value.
template <class T>
class ResourceRef
{
public:
    // Unassigned: refers to nothing and belongs to no model
    ResourceRef() = default;

    resource_id id() const { return this->id_; }

    // nullptr if the reference is unassigned or has expired
    std::shared_ptr<const T> get() const
    {
        if (this->builtin_ != nullptr)
        {
            return this->builtin_;
        }
        if (this->life_.expired() || this->token_.expired())
        {
            return nullptr;
        }
        return this->value_.lock();
    }

    // True if `get()` returns a value
    bool valid() const { return this->get() != nullptr; }

    // False only for a default-constructed reference
    bool is_assigned() const { return this->id_ != RESOURCE_ID_UNASSIGNED; }

    bool is_builtin() const { return this->builtin_ != nullptr; }

    // The model whose registry issued the reference. Empty for a built-in.
    const ModelToken &model() const { return this->token_; }

    // True if an element of the model with token `model` may use this
    // reference: it is a built-in, or it was issued by that model's registry
    // and the model has not been cleared since.
    bool usable_in(const ModelToken &model) const
    {
        return this->is_builtin() || same_model(this->token_, model);
    }

    // A value that belongs to no model, is accepted by every model, and is
    // never written to a model's resource table. `reserved_id` must be
    // negative, so that it cannot collide with a registry ID.
    // Throws std::invalid_argument if `reserved_id` is not negative, is the
    // unassigned ID, or `value` is null.
    static ResourceRef builtin(resource_id reserved_id,
                               std::shared_ptr<const T> value)
    {
        if (reserved_id >= 0 || reserved_id == RESOURCE_ID_UNASSIGNED)
        {
            std::stringstream ss;
            ss << "ResourceRef::builtin: invalid reserved ID " << reserved_id;
            throw std::invalid_argument(ss.str());
        }
        if (value == nullptr)
        {
            throw std::invalid_argument(
                "ResourceRef::builtin: the value is null");
        }
        ResourceRef ref;
        ref.id_ = reserved_id;
        ref.builtin_ = std::move(value);
        return ref;
    }

private:
    friend class ResourceRegistry<T>;

    ResourceRef(resource_id id,
                std::weak_ptr<const T> value,
                std::weak_ptr<const ResourceEntryLife> life,
                ModelToken token)
        : id_(id),
          value_(std::move(value)),
          life_(std::move(life)),
          token_(std::move(token))
    {
    }

    resource_id id_ = RESOURCE_ID_UNASSIGNED;
    std::weak_ptr<const T> value_;
    std::weak_ptr<const ResourceEntryLife> life_;
    std::shared_ptr<const T> builtin_;
    ModelToken token_;
};

// The entries of one resource type for one model. IDs start at 0 and are not
// reused, even after an entry is removed or the registry is cleared.
template <class T>
class ResourceRegistry
{
public:
    using container_type = Container<resource_id, T>;
    using iterator = typename container_type::iterator;
    using const_iterator = typename container_type::const_iterator;

    explicit ResourceRegistry(ModelToken token = ModelToken())
        : items_(0), token_(std::move(token))
    {
    }

    // A copy would share entry lifetimes with the original, so that removing
    // an entry from one would not expire references to the other
    ResourceRegistry(const ResourceRegistry &) = delete;
    ResourceRegistry &operator=(const ResourceRegistry &) = delete;
    ResourceRegistry(ResourceRegistry &&) = default;
    ResourceRegistry &operator=(ResourceRegistry &&) = default;

    // Stores a copy of `value` as a new entry, even if an equal entry exists
    ResourceRef<T> add(const T &value)
    {
        auto ptr = std::make_shared<T>(value);
        const resource_id id = this->items_.add_item(ptr);
        if (id < 0)
        {
            throw std::runtime_error("ResourceRegistry::add: insertion failed");
        }
        return this->make_ref(id, ptr, this->start_life(id));
    }

    // Returns the first entry equal to `value`, or adds one. Elements that
    // share an entry see each other's changes to it, so use `add` to keep
    // entries separate.
    ResourceRef<T> find_or_add(const T &value)
    {
        for (auto it = this->items_.get_const_iterator();
             !this->items_.is_at_end(it);
             ++it)
        {
            if (*it->second == value)
            {
                return this->get(it->first);
            }
        }
        return this->add(value);
    }

    // The entry with ID `id`. The reference is not valid if there is none.
    ResourceRef<T> get(resource_id id) const
    {
        const auto life = this->lives_.find(id);
        if (life == this->lives_.end())
        {
            return this->make_ref(id, {}, {});
        }
        return this->make_ref(id, this->items_.get_item(id), life->second);
    }

    // A change affects every element that uses the entry. nullptr if there is
    // no entry with ID `id`.
    T *get_mutable(resource_id id)
    {
        return this->items_.get_item(id).get();
    }

    // Allowed while elements use the entry. Their references expire, and they
    // fail validation the next time they are committed or set up for a run.
    bool remove(resource_id id)
    {
        this->lives_.erase(id);
        return this->items_.remove_item(id) > 0;
    }

    // Removes every entry, with the same effect on references as `remove`.
    // IDs are still not reused.
    void clear()
    {
        this->lives_.clear();
        this->items_.clear();
    }

    std::size_t size() const { return this->items_.get_number_of_items(); }

    iterator get_iterator() { return this->items_.get_iterator(); }
    const_iterator get_const_iterator() const
    {
        return this->items_.get_const_iterator();
    }
    bool is_at_end(iterator it) const { return this->items_.is_at_end(it); }
    bool is_at_end(const_iterator it) const
    {
        return this->items_.is_at_end(it);
    }

private:
    friend class SimulationData;

    // Recreates an entry with an ID that was saved elsewhere, for loaders.
    // Later `add` calls issue IDs above it.
    // Throws std::invalid_argument if `id` is negative or already in use.
    // Throws std::overflow_error if no larger ID can be represented.
    ResourceRef<T> restore(resource_id id, const T &value)
    {
        if (id < 0)
        {
            std::stringstream ss;
            ss << "Resource IDs must be 0 or greater, got " << id;
            throw std::invalid_argument(ss.str());
        }
        if (id == std::numeric_limits<resource_id>::max())
        {
            throw std::overflow_error("ResourceRegistry: no larger ID exists");
        }
        auto ptr = std::make_shared<T>(value);
        if (!this->items_.insert_item(id, ptr))
        {
            std::stringstream ss;
            ss << "Resource ID " << id << " is already in use";
            throw std::invalid_argument(ss.str());
        }
        this->items_.skip_past(id);
        return this->make_ref(id, ptr, this->start_life(id));
    }

    std::shared_ptr<const ResourceEntryLife> start_life(resource_id id)
    {
        auto life = std::make_shared<const ResourceEntryLife>();
        this->lives_[id] = life;
        return life;
    }

    ResourceRef<T> make_ref(resource_id id,
                            std::weak_ptr<const T> value,
                            std::weak_ptr<const ResourceEntryLife> life) const
    {
        return ResourceRef<T>(
            id, std::move(value), std::move(life), this->token_);
    }

    container_type items_;
    std::map<resource_id, std::shared_ptr<const ResourceEntryLife>> lives_;
    ModelToken token_;
};

} // namespace SolTrace::Data

#endif
