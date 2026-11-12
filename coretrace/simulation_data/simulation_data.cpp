#include "simulation_data.hpp"

#include <cassert>
#include <sstream>

#include "composite_element.hpp"
#include "simdata_io.hpp"
#include "json_schema.hpp"

namespace SolTrace::Data {

SimulationData::SimulationData() : number_of_elements(0),
                                   my_elements(1),
                                   my_sources(0),
                                   my_optical_property_sets(0),
                                   model_state(std::make_shared<ModelState>())
{
    this->bind_model_state();
    return;
}

SimulationData::SimulationData(SimulationData&& other)
    : number_of_elements(other.number_of_elements),
      my_groups(std::move(other.my_groups)),
      my_elements(std::move(other.my_elements)),
      my_sources(std::move(other.my_sources)),
      my_parameters(std::move(other.my_parameters)),
      my_optical_property_sets(std::move(other.my_optical_property_sets)),
      model_state(std::move(other.model_state))
{
    this->bind_model_state();
    other.reset_after_move();
}

SimulationData& SimulationData::operator=(SimulationData&& other)
{
    if (this != &other)
    {
        this->number_of_elements = other.number_of_elements;
        this->my_groups = std::move(other.my_groups);
        this->my_elements = std::move(other.my_elements);
        this->my_sources = std::move(other.my_sources);
        this->my_parameters = std::move(other.my_parameters);
        this->my_optical_property_sets =
            std::move(other.my_optical_property_sets);
        // Contexts made for the model this object had before expire here
        this->model_state = std::move(other.model_state);
        this->bind_model_state();
        other.reset_after_move();
    }
    return *this;
}

SimulationData::~SimulationData()
{
    this->clear(true);
    return;
}

ModelContext SimulationData::context()
{
    return ModelContext(this->model_state);
}

void SimulationData::bind_model_state()
{
    this->model_state->registrar = [this](std::shared_ptr<Element> el)
    {
        return this->register_element(el);
    };
    this->model_state->is_registered = [this](element_id id)
    {
        return this->my_elements.get_item(id) != nullptr;
    };
}

void SimulationData::reset_after_move()
{
    this->model_state = std::make_shared<ModelState>();
    this->bind_model_state();
    this->clear(true);
}

namespace {

bool none_registered(const ElementContainer &registry, const element_ptr &el)
{
    if (registry.get_item(el->get_id()) != nullptr)
    {
        return false;
    }
    auto comp = std::dynamic_pointer_cast<CompositeElement>(el);
    if (comp != nullptr)
    {
        for (auto iter = comp->get_const_iterator();
             !comp->is_at_end(iter);
             ++iter)
        {
            if (!none_registered(registry, iter->second))
            {
                return false;
            }
        }
    }
    return true;
}

// A parent is computed before its children, which read its frame
void compute_rotations(const element_ptr &el)
{
    el->compute_coordinate_rotations();
    auto comp = std::dynamic_pointer_cast<CompositeElement>(el);
    if (comp != nullptr)
    {
        for (auto iter = comp->get_const_iterator();
             !comp->is_at_end(iter);
             ++iter)
        {
            compute_rotations(iter->second);
        }
    }
}

} // namespace

element_id SimulationData::register_element(const element_ptr &el)
{
    if (el == nullptr)
    {
        throw std::invalid_argument("Cannot register a null element");
    }
    if (!none_registered(this->my_elements, el))
    {
        throw std::invalid_argument(
            "The element or one of its subelements is already registered");
    }

    // Nothing is registered if either of these throws
    // Check that all fields required from the user have been specified
    el->enforce_user_fields_set();
    // Make sure coordinate stuff has been computed
    compute_rotations(el);

    this->insert_element(el);
    return el->get_id();
}

void SimulationData::insert_element(const element_ptr &el)
{
    const element_id id = el->get_id();
    const bool inserted = this->my_elements.insert_item(id, el);
    assert(inserted);
    (void)inserted;

    if (el->is_composite())
    {
        // The CompositeElement itself does not count toward the
        // number of elements since it does not impact the ray
        // tracing computation. So we do not increment the
        // number of elements here.
        auto cptr = std::dynamic_pointer_cast<CompositeElement>(el);
        assert(cptr != nullptr);
        for (auto iter = cptr->get_const_iterator();
             !cptr->is_at_end(iter);
             ++iter)
        {
            this->insert_element(iter->second);
        }
    }
    else
    {
        this->number_of_elements++;

        // only check groups on single elements
        int32_t group = el->get_group();

        if (group > -1)
        {
            // ensure that group index exists by adding empty groups if necessary
            size_t my_groups_size = this->my_groups.size();
            if (group >= my_groups_size)
            {
                for (size_t i = my_groups_size; i <= (size_t)group; ++i)
                {
                    this->my_groups.push_back(std::set<uint_fast64_t>());
                }
            }

            this->my_groups[group].insert(id);
        }
    }
}

element_ptr SimulationData::get_element(element_id id) const
{
    return this->my_elements.get_item(id);
    // element_ptr retval = nullptr;
    // auto iter1 = this->my_elements.find(id);
    // auto iter2 = this->composite_elements.find(id);
    // if (iter1 != this->my_elements.end())
    // {
    //     retval = iter1->second;
    // }
    // else if (iter2 != this->composite_elements.end())
    // {
    //     retval = iter2->second;
    // }
    // else
    // {
    //     // Intentional no-op
    // }
    // return retval;
}

uint_fast64_t SimulationData::remove_element(element_id id)
{

    uint_fast64_t nremoved = 0;
    element_ptr el = this->my_elements.get_item(id);

    if (el != nullptr)
    {
        this->my_elements.remove_item(id);
        if (el->is_composite())
        {
            // The CompositeElement itself does not count toward the number
            // of elements since it does not impact the ray tracing computation.
            // So we do not decrement the number of elements here.
            nremoved = this->remove_subelements(el);
            assert(nremoved == el->get_number_of_elements());
        }
        else
        {
            this->number_of_elements--;
            const int32_t group = el->get_group();
            if (group > -1 && static_cast<size_t>(group) < this->my_groups.size())
            {
                this->my_groups[group].erase(id);
            }
            nremoved = 1;
        }
    }

    return nremoved;

    // auto iter2 = this->composite_elements.find(id);
    // if (iter1 != this->my_elements.end())
    // {
    //     retval = remove_single_element(iter1);
    // }
    // else if (iter2 != this->composite_elements.end())
    // {
    //     retval = remove_composite_element(iter2);
    // }
    // else
    // {
    //     // Intentional no-op
    // }
    // return retval;
}

// uint_fast64_t SimulationData::remove_single_element(
//     SingleElementMap::iterator iter)
// {
//     iter->second->set_id(ELEMENT_ID_UNASSIGNED);
//     this->my_elements.erase(iter);
//     return 1;
// }

// uint_fast64_t SimulationData::remove_composite_element(element_ptr el)
// {
//     composite_element_ptr cptr = iter->second;
//     iter->second->set_id(ELEMENT_ID_UNASSIGNED);
//     this->composite_elements.erase(iter);
//     return this->remove_subelements(cptr);
// }

uint_fast64_t SimulationData::remove_subelements(element_ptr el)
{
    uint_fast64_t retval = 0;

    composite_element_ptr cptr =
        std::dynamic_pointer_cast<CompositeElement>(el);
    assert(cptr != nullptr);

    element_ptr ptr = nullptr;
    for (auto iter = cptr->get_iterator(); !cptr->is_at_end(iter); ++iter)
    {
        ptr = iter->second;
        retval += this->remove_element(ptr->get_id());
    }

    assert(retval == cptr->get_number_of_elements());

    return retval;
}

OpticalPropertySetReference SimulationData::add_optical_property_set(const OpticalPropertySet& opt_set)
{
    std::shared_ptr<OpticalPropertySet> ptr = std::make_shared<OpticalPropertySet>(opt_set);
    const optics_id id = this->my_optical_property_sets.add_item(ptr);
    return { id, ptr };
}

OpticalPropertySetReference SimulationData::find_or_add_optical_property_set(const OpticalPropertySet& opt_set)
{
    // Check if set already exists
    for (auto it = this->get_optics_iterator(); !this->is_optics_at_end(it); ++it)
    {
        const OpticalPropertySet& existing = *it->second;
        if (existing == opt_set)
        {
            const optics_id id = it->first;
            auto ptr = it->second;
            return { id, ptr };

        }
    }

    // Add if it doesn't exist
    return add_optical_property_set(opt_set);
}

const OpticalPropertySet* SimulationData::get_optical_property_set(const Element& el) const
{
    return get_optical_property_set(el.get_optical_property_set_id());
}

OpticalPropertySet* SimulationData::get_mutable_optical_property_set(const Element& el)
{
    return this->get_optical_property_set(el.get_optical_property_set_id());
}

const OpticalPropertySet* SimulationData::get_optical_property_set(optics_id id) const
{
    auto ptr = this->my_optical_property_sets.get_item(id);
    return ptr == nullptr ? nullptr : ptr.get();
}

OpticalPropertySet* SimulationData::get_optical_property_set(optics_id id)
{
    auto ptr = this->my_optical_property_sets.get_item(id);
    return ptr == nullptr ? nullptr : ptr.get();
}

int SimulationData::update_simulation_positions()
{
    int sts = 0;
    // TODO: Implement this
    return sts;
}

bool SimulationData::import_from_file(const char *file_name)
{
    return load_stinput_file(*this, file_name);
}

bool SimulationData::import_from_file(const std::string file_name)
{
    return this->import_from_file(file_name.c_str());
}

void SimulationData::import_json_file(const std::string file_name, std::string* upgrade_log)
{
    load_json_file(*this, file_name, upgrade_log);
}

void SimulationData::export_json_file(const std::string file_name)
{
    write_json_file(*this, file_name);
}

void SimulationData::clear(bool reset_parameters)
{
    this->my_elements.clear();
    this->my_sources.clear();
    this->my_groups.clear();
    this->number_of_elements = 0;

    this->my_optical_property_sets.reset(0);

    // Everything created for the model before this point no longer belongs to it
    if (this->model_state != nullptr)
    {
        this->model_state->reset();
    }

    if (reset_parameters)
        this->my_parameters = SimulationParameters();   // Reset
}

} // namespace SolTrace::Data
