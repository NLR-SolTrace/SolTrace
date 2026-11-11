/**
 * @file container.hpp
 * @brief Generic container utilities for simulation data
 *
 * Provides template-based container classes and utilities for
 * managing collections of simulation objects. Includes specialized
 * containers for elements, apertures, surfaces, and other components
 * with shared pointer management.
 */

#ifndef SOLTRACE_ELEMENT_CONTAINER_H
#define SOLTRACE_ELEMENT_CONTAINER_H

#include <cstdint>
#include <map>
#include <memory>

namespace SolTrace::Data {

template <typename K, typename V>
class Container
{
public:
    using value_pointer = typename std::shared_ptr<V>;
    using iterator = typename std::map<K, value_pointer>::iterator;
    using const_iterator = typename std::map<K, value_pointer>::const_iterator;

    /**
     * @brief Create a shared pointer to value type V
     * @tparam Args Constructor argument types
     * @param args Constructor arguments
     * @return Shared pointer to constructed object
     */
    template<typename... Args>
    inline static value_pointer make_pointer(Args&&... args)
    {
        return std::make_shared<V>(std::forward<Args>(args)...);
    }

    /**
     * @brief Create a shared pointer to derived type C
     * @tparam C Derived type to construct
     * @tparam Args Constructor argument types
     * @param args Constructor arguments
     * @return Shared pointer to constructed object of type C
     */
    template<typename C, typename... Args>
    inline static std::shared_ptr<C> make_pointer(Args&&... args)
    {
        return std::make_shared<C>(std::forward<Args>(args)...);
    }

    /**
     * @brief Constructor with starting ID
     * @param start Starting value for ID assignment (default 0)
     */
    Container(K start=0) : next_id(start) {}

    /**
     * @brief Destructor - clears all items
     */
    ~Container() { this->clear(); }

    /**
     * @brief Add an item to the container
     * @param item Shared pointer to item to add
     * @return Assigned key/ID for the item (-1 if insertion failed)
     */
    K add_item(value_pointer item)
    {
        auto key = this->next_id;
        typename std::map<K, value_pointer>::value_type to_insert(key, item);
        auto result = this->container.insert(to_insert);
        if (result.second == true)
        {
            ++next_id;
        }
        else
        {
            // Insertion failed. The only obvious reason for this is that
            // the key is a duplicate. However, given the key is assigned
            // by the container itself and is a 64-bit integer, where the
            // next id is just incrementing the integer, this should never
            // happen. But just in case...
            key = -1;
        }
        return key;
    }

    /**
     * @brief Remove an item from the container by key
     * @param id Key of item to remove
     * @return Number of items removed (0 or 1)
     */
    auto remove_item(K id)
    {
        return this->container.erase(id);
    }

    /**
     * @brief Get an item from the container by key
     * @param id Key of item to retrieve
     * @return Shared pointer to item, or nullptr if not found
     */
    value_pointer get_item(K id) const
    {
        // return this->container[id];
        auto item = this->container.find(id);
        // value_pointer ptr = (item == this->container.end()
        //                          ? std::nullptr_t
        //                          : item->second);
        // return ptr;
        return (item == this->container.end() ? nullptr : item->second);
    }

    uint_fast64_t get_number_of_items() const
    {
        return this->container.size();
    }
    // uint64_t get_total_number_of_items() const
    // {
    //     // TODO: Implement this if needed. For elements, a value can
    //     // be a CompositeElement. In this case, it has its own
    //     // collection of elements which need to be counted.
    //     return 0;
    // }

    void clear()
    {
        this->container.clear();
        return;
    }

    void reset(K start = 0)
    {
        this->container.clear();
        this->next_id = start;
    }

    iterator get_iterator() { return container.begin(); }
    const_iterator get_const_iterator() const { return container.cbegin(); }
    bool is_at_end(iterator iter) const { return iter == container.end(); }
    bool is_at_end(const_iterator citer) const { return citer == container.cend(); }

    bool insert_item(K id, value_pointer item)
    {
        typename std::map<K, value_pointer>::value_type to_insert(id, item);
        auto result = this->container.insert(to_insert);
        return result.second;
    }

    void recompute_next_id(K minimum = 0)
    {
        K next = minimum;
        for (const auto& [id, item] : this->container)
        {
            if (id >= next)
            {
                next = id + 1;
            }
        }

        this->next_id = next;
    }

private:
    std::map<K, value_pointer> container;
    mutable K next_id;
};

} // namespace SolTrace::Data

#endif
