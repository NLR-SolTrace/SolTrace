
#include "composite_element.hpp"

#include <sstream>

#include "single_element.hpp"
#include "virtual_element.hpp"

namespace SolTrace::Data
{

    CompositeElement::CompositeElement(ElementKey key)
        : ElementBase(key),
          my_elements()
    {
        return;
    }

    CompositeElement::CompositeElement(ElementKey key,
        const nlohmann::ordered_json &jnode,
        const OpticalPropertySetResolver& resolve_optics) 
        : ElementBase(key, jnode), my_elements()
    {
        using json = nlohmann::ordered_json;

        // Common parameters are set in ElementBase constructor before here

        json jelements = jnode.at("elements");
        for (auto &[jkey, jelement] : jelements.items())
        {
            bool is_single = jelement.at("is_single");
            if (is_single)
            {
                if (jelement.at("virtual_flag") == true)
                {
                    this->create_child<VirtualElement>(jelement, resolve_optics);
                }
                else
                {
                    this->create_child<SingleElement>(jelement, resolve_optics);
                }
            }
            // Composites inside a composite are skipped, as they always have
            // been: this object is not yet a StageElement, so the nesting rule
            // rejected them. The writer also writes every composite at the top
            // level, which is where they are loaded.
        }
        return;
    }

    CompositeElement::~CompositeElement()
    {
        this->clear();
        return;
    }

    void CompositeElement::disable() const
    {
        this->active = false;
        for (auto iter = this->get_const_iterator();
             !this->is_at_end(iter);
             ++iter)
        {
            iter->second->disable();
        }
        return;
    }

    void CompositeElement::enable() const
    {
        this->active = true;
        for (auto iter = this->get_const_iterator();
             !this->is_at_end(iter);
             ++iter)
        {
            iter->second->enable();
        }
        return;
    }

    void CompositeElement::mark_virtual() const
    {
        this->ElementBase::mark_virtual();
        for (auto iter = this->get_const_iterator();
             !this->is_at_end(iter);
             ++iter)
        {
            iter->second->mark_virtual();
        }
        return;
    }

    void CompositeElement::unmark_virtual() const
    {
        this->ElementBase::unmark_virtual();
        for (auto iter = this->get_const_iterator();
             !this->is_at_end(iter);
             ++iter)
        {
            iter->second->unmark_virtual();
        }
        return;
    }

    void CompositeElement::set_stage(int_fast64_t stage)
    {
        this->stage = stage;
        for (auto iter = this->get_iterator(); !this->is_at_end(iter); ++iter)
        {
            iter->second->set_stage(stage);
        }
        return;
    }

    uint_fast64_t CompositeElement::get_number_of_elements() const
    {
        uint_fast64_t n = 0;
        for (auto iter = this->get_const_iterator();
             !this->is_at_end(iter);
             ++iter)
        {
            n += iter->second->get_number_of_elements();
        }
        return n;
    }

    element_id CompositeElement::attach_child(element_ptr el)
    {
        // Cannot nest a StageElement in another CompositeElement, and a
        // CompositeElement can be added only to a StageElement
        if (el->is_stage() || (el->is_composite() && !this->is_stage()))
        {
            return ELEMENT_INVALID_SETUP;
        }

        element_id key = this->my_elements.add_item(el);
        if (!is_success(key))
        {
            return ELEMENT_ERROR;
        }

        el->set_reference_element(this);
        // A child belongs to the same stage as its parent, whether it is added
        // before or after the parent is put in a stage
        if (this->stage != -1)
        {
            el->set_stage(this->stage);
        }
        // Mark any added elements as virtual if needed
        if (this->is_virtual())
        {
            el->mark_virtual();
        }
        return el->get_id();
    }

    uint_fast64_t CompositeElement::remove_element(element_id id)
    {
        element_ptr el = this->my_elements.get_item(id);
        uint_fast64_t nremoved = this->my_elements.remove_item(id);

        if (nremoved > 0)
        {
            nremoved = el->get_number_of_elements();
        }

        return nremoved;
    }

    element_ptr CompositeElement::get_element(element_id id)
    {
        return this->my_elements.get_item(id);
    }

    void CompositeElement::clear()
    {
        this->my_elements.clear();
        return;
    }

    void CompositeElement::enforce_user_fields_set() const
    {
        ElementBase::enforce_user_fields_set();

        if (this->get_number_of_elements() == 0)
        {
            std::stringstream ss;
            ss << "CompositeElement (Name: " << this->get_name()
               << ", UUID: " << this->get_id()
               << ") has no subelements.";
            throw std::invalid_argument(ss.str());
        }

        for (auto iter = this->get_const_iterator();
             !this->is_at_end(iter);
             ++iter)
        {
            iter->second->enforce_user_fields_set();
        }
        return;
    }

    // Stage and Composite should have the same function
    void CompositeElement::write_json(nlohmann::ordered_json &jnode) const
    {
        using json = nlohmann::ordered_json;

        // Composite/stage specific info
        jnode["is_composite"] = true;
        jnode["is_single"] = false;
        jnode["number_of_elements"] = this->get_number_of_elements();

        // Common properties of all elements
        this->write_common_json(jnode);

        // Loop through each element, writing json node
        json jelements;
        for (auto it = this->get_const_iterator(); !this->is_at_end(it); ++it)
        {
            json jelement;

            SolTrace::Data::element_id i = it->first;
            auto element = it->second;

            element->write_json(jelement);

            jelements[std::to_string(i)] = jelement;
        }
        jnode["elements"] = jelements;
    }

} // namespace SolTrace::Data
