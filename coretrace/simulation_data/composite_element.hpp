/**
 * @file composite_element.hpp
 * @brief Composite element class for complex optical systems
 *
 * Defines the CompositeElement class which can contain multiple
 * sub-elements, allowing for hierarchical optical system definitions.
 * Enables grouping of related optical elements for easier management
 * and coordinate transformations.
 *
 * @defgroup elements Optical Elements
 * @{
 */

#ifndef SOLTRACE_COMPOSITE_ELEMENT_H
#define SOLTRACE_COMPOSITE_ELEMENT_H

#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include "container.hpp"
#include "element.hpp"

namespace SolTrace::Data
{

    class CompositeElement : public ElementBase
    {
    public:
        /**
         * @brief Constructor for composite element
         * @param key Passkey from the ModelContext that creates the element
         */
        explicit CompositeElement(ElementKey key);
        CompositeElement(ElementKey key,
            const nlohmann::ordered_json& jnode,
            const OpticalPropertySetResolver& resolve_optics);
        virtual ~CompositeElement();

        /**
         * @brief Disable this composite element and all sub-elements
         */
        virtual void disable() const override;

        /**
         * @brief Enable this composite element and all sub-elements
         */
        virtual void enable() const override;

        /**
         * @brief Check if this element is composite
         * @return Always returns true for composite elements
         */
        virtual bool is_composite() const override
        {
            return true;
        }

        virtual void mark_virtual() const override;
        virtual void unmark_virtual() const override;

        /**
         * @brief Set the stage number for this composite element
         * @param stage Stage number to assign
         */
        virtual void set_stage(int_fast64_t stage) override;

        /**
         * @brief Get the number of sub-elements in this composite
         * @return Number of elements contained in this composite
         */
        virtual uint_fast64_t get_number_of_elements() const override;

        // Element interface functions
        /**
         * @brief Get aperture pointer (always null for composite elements)
         * @return nullptr (composite elements don't have apertures)
         */
        virtual const aperture_ptr get_aperture() const override { return nullptr; }

        /**
         * @brief Get aperture pointer (always null for composite elements)
         * @return nullptr (composite elements don't have apertures)
         */
        virtual aperture_ptr get_aperture() override { return nullptr; }

        /**
         * @brief Set aperture (no-op for composite elements)
         * @param aperture_ptr Ignored for composite elements
         */
        virtual void set_aperture(aperture_ptr) override {}

        /**
         * @brief Get surface pointer (always null for composite elements)
         * @return nullptr (composite elements don't have surfaces)
         */
        virtual const surface_ptr get_surface() const override { return nullptr; }

        /**
         * @brief Get surface pointer (always null for composite elements)
         * @return nullptr (composite elements don't have surfaces)
         */
        virtual surface_ptr get_surface() override { return nullptr; }

        /**
         * @brief Set surface (no-op for composite elements)
         * @param surface_ptr Ignored for composite elements
         */
        virtual void set_surface(surface_ptr) override {}

        /**
         * @brief Get optical propertiy set id (always undefined for composite elements)
         * @return unassigned id (composite elements don't have optical properties)
         */
        virtual optics_id get_optical_property_set_id() const override
        {
            return OPTICS_ID_UNASSIGNED;
        }

        /**
         * @brief Get optical propertiy set pointer
         * @return nullptr (composite elements don't have optical properties)
         */
        virtual std::shared_ptr<const OpticalPropertySet> get_optical_property_set() const override
        {
            return nullptr;
        }

        /**
         * @brief Set optical propertiy set id (does not apply for composite elements)
         */
        virtual void set_optical_property_set(const OpticalPropertySetReference&) override 
        {
            assert(false && "CompositeElement does not support optical property sets");
        }

        // CompositeElement accessors
        /**
         * @brief Create an element of type `T` as a child of this composite.
         *
         * The child is created in this composite's model, receives its ID
         * immediately, and is attached before the function returns. Nothing
         * is registered until the root of the tree is committed. Configure the
         * child through the returned pointer.
         *
         * @param args Constructor arguments for `T`, after the ElementKey
         * @return Pointer to the attached child
         * @throws std::invalid_argument if this composite is already
         *         registered, if the model no longer exists or was cleared, or
         *         if the child cannot be nested here (a stage, or a composite
         *         in anything but a stage). The ID issued is not reused.
         */
        template <class T, class... Args>
        std::shared_ptr<T> create_child(Args &&...args)
        {
            static_assert(std::is_base_of_v<Element, T>,
                          "a child must be an Element");

            if (this->model_context.is_registered(this->my_id))
            {
                throw std::invalid_argument(
                    "CompositeElement: cannot add a child to a registered "
                    "composite");
            }
            if (!same_model(this->model_token, this->model_context.token()))
            {
                throw std::invalid_argument(
                    "CompositeElement: the model no longer exists or was "
                    "cleared");
            }

            auto pending = this->model_context.template create<T>(
                std::forward<Args>(args)...);
            std::shared_ptr<T> child = pending.release();
            if (!Element::is_success(this->attach_child(child)))
            {
                throw std::invalid_argument(
                    "CompositeElement: a stage cannot be a child, and a "
                    "composite can be a child only of a stage");
            }
            return child;
        }

        /**
         * @brief Attach an element that was created earlier to this composite
         *
         * For importers that create elements before they know the parent.
         * Prefer `create_child`. The handle is emptied only on success.
         *
         * @param child Handle of the element to attach
         * @return ID of the child on success. ELEMENT_NULL for an empty
         *         handle, ELEMENT_WRONG_MODEL for a child from another or a
         *         destroyed model, ELEMENT_ALREADY_REGISTERED if the child is
         *         registered, ELEMENT_INVALID_SETUP if this composite is
         *         registered or the child cannot be nested here.
         */
        template <class U>
        element_id add_child(PendingElement<U> &&child)
        {
            if (child.element_ == nullptr)
            {
                return ELEMENT_NULL;
            }
            if (!same_model(child.token_, this->model_token) ||
                !same_model(this->model_token, this->model_context.token()))
            {
                return ELEMENT_WRONG_MODEL;
            }
            if (child.committed_ ||
                this->model_context.is_registered(child.id_))
            {
                return ELEMENT_ALREADY_REGISTERED;
            }
            if (this->model_context.is_registered(this->my_id))
            {
                return ELEMENT_INVALID_SETUP;
            }

            const element_id id = this->attach_child(child.element_);
            if (Element::is_success(id))
            {
                child.release();
            }
            return id;
        }

        /**
         * @brief Remove an element from this composite
         * @param id Element ID to remove
         * @return Number of elements removed (0 or 1)
         */
        uint_fast64_t remove_element(element_id id);

        /**
         * @brief Get an element by ID
         * @param id Element ID to retrieve
         * @return Shared pointer to element, or null if not found
         */
        element_ptr get_element(element_id id);

        /**
         * @brief Remove all elements from this composite
         */
        void clear();

        // uint64_t get_total_number_of_elements() const
        // {
        //     return this->my_elements.get_total_number_of_items();
        // }

        /**
         * @brief Get iterator to beginning of element container
         * @return Iterator to first element
         */
        virtual ElementContainer::iterator get_iterator()
        {
            return this->my_elements.get_iterator();
        }
        virtual ElementContainer::const_iterator get_const_iterator() const
        {
            return this->my_elements.get_const_iterator();
        }
        virtual bool is_at_end(ElementContainer::iterator iter)
        {
            return this->my_elements.is_at_end(iter);
        }
        virtual bool is_at_end(ElementContainer::const_iterator citer) const
        {
            return this->my_elements.is_at_end(citer);
        }
        
        // only single elements have groups
        virtual int32_t get_group() const override { return -2; }

        virtual void enforce_user_fields_set() const override;

        virtual void write_json(nlohmann::ordered_json& jnode) const override;

    private:
        // Adds `el` to this composite. Returns ELEMENT_INVALID_SETUP if the
        // nesting rule is broken, otherwise the ID of `el`.
        element_id attach_child(element_ptr el);
        ElementContainer my_elements;
    };

    using composite_element_ptr = std::shared_ptr<CompositeElement>;

} // namespace SolTrace::Data

#endif

/**
 * @}
 */
