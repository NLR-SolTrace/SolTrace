/**
 * @file simulation_data.hpp
 * @brief Main simulation data container and management
 *
 * Defines the SimulationData class which serves as the main container
 * for all optical elements, stages, and simulation parameters.
 * Manages the complete optical system definition and provides
 * interfaces for ray tracing execution.
 *
 * @defgroup simulation Simulation Management
 * @{
 */

#ifndef SOLTRACE_SIMULATION_DATA_H
#define SOLTRACE_SIMULATION_DATA_H

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <set>
#include <vector>

#include "container.hpp"
#include "element.hpp"
#include "model_context.hpp"
#include "model_state.hpp"
#include "ray_source.hpp"
#include "simulation_parameters.hpp"

namespace SolTrace::Data {

class SimulationData
{
    friend void load_json_file(SimulationData& sd, std::string filename, std::string* upgrade_log);

public:
    SimulationData();
    virtual ~SimulationData();

    // Copying is not allowed. A move keeps every context, element token, and
    // reference that was made for the original valid for the new object. The
    // moved-from object is empty, belongs to a new model, and can still be
    // used or destroyed.
    SimulationData(const SimulationData&) = delete;
    SimulationData& operator=(const SimulationData&) = delete;
    SimulationData(SimulationData&& other);
    SimulationData& operator=(SimulationData&& other);

    /// @brief Get a context for creating elements that belong to this model
    ModelContext context();

    /// @brief Create an element that belongs to this model. Equivalent to
    ///        `context().create<T>(args...)`.
    template <class T, class... Args>
    [[nodiscard]] PendingElement<T> create_element(Args&&... args)
    {
        return this->context().template create<T>(std::forward<Args>(args)...);
    }

    /// @brief Create an element with an ID that was saved elsewhere.
    template <class T, class... Args>
    [[nodiscard]] PendingElement<T> create_element_with_id(element_id id,
                                                           Args&&... args)
    {
        return this->context().template create_with_id<T>(
            id, std::forward<Args>(args)...);
    }

    /// @brief Add the given RaySource to the simulation data
    /// @param src RaySource to add
    /// @return unique identifier for the RaySource
    ray_source_id add_ray_source(ray_source_ptr src)
    {
        return this->my_sources.add_item(src);
    }

    /// @brief Remove the RaySource corresponding to the unique identifier `id`
    /// @param id unique identifier of the RaySource to remove
    /// @return true if the element was removed, false otherwise
    auto remove_ray_source(ray_source_id id)
    {
        return this->my_sources.remove_item(id);
    }

    /// @brief Get the RaySource corresponding to the unique identifier `id`
    /// @param id unique identifier of the RaySource to get
    /// @return requested RaySource if found, nullptr otherwise
    ray_source_ptr get_ray_source(ray_source_id id) const
    {
        return this->my_sources.get_item(id);
    }

    ray_source_ptr get_ray_source() const
    {
        return this->my_sources.get_const_iterator()->second;
    }

    uint_fast64_t get_number_of_ray_sources() const
    {
        return this->my_sources.get_number_of_items();
    }

    /// @brief Get an iterator that can be used to access all ray sources owned
    ///        by this SimulationData object.
    /// @return iterator
    RaySourceContainer::iterator get_ray_source_iterator()
    {
        return this->my_sources.get_iterator();
    }

    /// @brief Tests whether the given iterator is at the end
    /// @param iter iterator to test
    /// @return true if at end, false otherwise
    bool is_ray_source_at_end(RaySourceContainer::iterator it)
    {
        return this->my_sources.is_at_end(it);
    }

    /// @brief Elements are added with `create_element(...)` followed by
    ///        `commit()` on the returned handle. A CompositeElement is
    ///        committed with all of its subelements, so they must be created
    ///        with `create_child` or attached with `add_child` before then.

    /// @brief Remove the Element corresponding to the unique identifier `id`
    ///        (and, for a CompositeElement, its subelements). The IDs are not
    ///        reused, and a removed element is not registered again.
    /// @param id unique identifier of element to remove
    /// @return number of elements removed
    uint_fast64_t remove_element(element_id id);

    /// @brief Get the Element corresponding to the unique identifier `id`
    /// @param id unique identifier of element to get
    /// @return requested Element (pointer) if found, nullptr otherwise
    element_ptr get_element(element_id id) const;

    /// @brief Gives the number of elements owned by the SimulationData.
    ///        CompositeElements do not count toward this number.
    /// @return Number of elements owned by the SimulationData object
    uint_fast64_t get_number_of_elements() const
    {
        // return this->my_elements.size();
        return this->number_of_elements;
    }

    /// @brief Get an iterator that can be used to access all elements owned
    ///        by this SimulationData object.
    /// @return iterator
    ElementContainer::iterator get_iterator()
    {
        return this->my_elements.get_iterator();
        // return this->my_elements.begin();
    }

    /// @brief Get an const_iterator that can be used to access all elements
    ///        owned by this SimulationData object.
    /// @return iterator
    ElementContainer::const_iterator get_const_iterator() const
    {
        return this->my_elements.get_const_iterator();
        // return this->my_elements.cbegin();
    }

    /// @brief Tests whether the given iterator is at the end
    /// @param iter iterator to test
    /// @return true if at end, false otherwise
    bool is_at_end(ElementContainer::iterator iter)
    {
        return this->my_elements.is_at_end(iter);
        // return iter == this->my_elements.end();
    }
    bool is_at_end(ElementContainer::const_iterator citer) const
    {
        return this->my_elements.is_at_end(citer);
        // return citer == this->my_elements.end();
    }

    OpticalPropertySetReference add_optical_property_set(const OpticalPropertySet& opt_set);
    OpticalPropertySetReference find_or_add_optical_property_set(const OpticalPropertySet& opt_set);

    const OpticalPropertySet* get_optical_property_set(const Element& el) const;
    OpticalPropertySet* get_mutable_optical_property_set(const Element& el);

    /// @brief Get an iterator that can be used to access all 
    ///  optical property sets owned by this SimulationData object.
    /// @return iterator
    OpticalPropertySetContainer::iterator get_optics_iterator()
    {
        return this->my_optical_property_sets.get_iterator();
    }

    /// @brief Tests whether the given iterator is at the end
    /// @param iter iterator to test
    /// @return true if at end, false otherwise
    bool is_optics_at_end(OpticalPropertySetContainer::iterator it)
    {
        return this->my_optical_property_sets.is_at_end(it);
    }

    /// @brief Set the number of rays to trace
    /// @param nrays number of rays to trace
    void set_number_of_rays(uint_fast64_t nrays)
    {
        this->my_parameters.number_of_rays = nrays;
        return;
    }

    /// @brief Get the number of rays to trace
    /// @return number of rays to trace
    uint_fast64_t get_number_of_rays() const
    {
        return this->my_parameters.number_of_rays;
    }

    void set_max_rays_traced(uint_fast64_t nrays)
    {
        this->my_parameters.max_number_of_rays = nrays;
        return;
    }

    uint_fast64_t get_max_number_rays_traced() const
    {
        return this->my_parameters.max_number_of_rays;
    }

    void set_tolerance(double tolerance)
    {
        this->my_parameters.tolerance = tolerance;
        return;
    }
    double get_tolerance() const
    {
        return this->my_parameters.tolerance;
    }

    /// @brief Set the seed used for the random number generation
    /// @param seed seed to set
    void set_seed(uint_fast64_t seed)
    {
        this->my_parameters.seed = seed;
        return;
    }

    /// @brief Get the seed used for random number generation
    /// @return current seed
    uint_fast64_t get_seed() const
    {
        return this->my_parameters.seed;
    }

    void set_latitude(double latitude)
    {
        this->my_parameters.latitude = latitude;
        return;
    }
    double get_latitude() const
    {
        return this->my_parameters.latitude;
    }

    void set_longitude(double longitude)
    {
        this->my_parameters.longitude = longitude;
        return;
    }
    double get_longitude() const
    {
        return this->my_parameters.longitude;
    }

    SimulationParameters &get_simulation_parameters()
    {
        return this->my_parameters;
    }
    const SimulationParameters &get_simulation_parameters() const
    {
        return this->my_parameters;
    }

    const std::vector<std::set<uint_fast64_t>>& get_groups() const
    {
        return this->my_groups;
    }

    int update_simulation_positions();

    bool import_from_file(const char *file_name);
    bool import_from_file(const std::string file_name);

    /// @brief Import simulation data from a JSON file.
    /// @param file_name Path to the JSON file to import.
    /// @param upgrade_log Optional pointer to a string that will be populated
    ///        with a human-readable description of any schema upgrades applied
    ///        during import. Left untouched if no upgrade was needed. Pass
    ///        nullptr (default) if this information is not needed.
    /// @throws std::runtime_error if the file cannot be read or parsed.
    /// Loads simulation data from the specified JSON file and updates the current simulation state.
    void import_json_file(const std::string file_name, std::string* upgrade_log = nullptr);

    /// @brief Export simulation data to a JSON file.
    /// @param file_name Path to the JSON file to write.
    /// @throws std::runtime_error if the file cannot be written.
    /// Serializes the current simulation data and writes it to the specified JSON file.
    void export_json_file(const std::string file_name);

    /// @brief Clear elements, ray sources, and optical property sets. 
    /// Designed for use by the file stinput and json file readers when there is an error.
    /// @param reset_parameters Optional bool to reset simulation parameters
    void clear(bool reset_parameters = false);

private:
    // mutable element_id next_element_id;

    uint_fast64_t number_of_elements;
    // index is the group number and the value is a set containing the element ids of the group
    std::vector<std::set<uint_fast64_t>> my_groups;

    ElementContainer my_elements;
    RaySourceContainer my_sources;
    SimulationParameters my_parameters;
    OpticalPropertySetContainer my_optical_property_sets;

    // Shared with the contexts, elements, and references made for this model
    std::shared_ptr<ModelState> model_state;

    // Points the callbacks in `model_state` at this object
    void bind_model_state();
    // Puts a moved-from object back in a usable, empty state
    void reset_after_move();

    // The only way an element enters the registry; reached through the
    // `commit()` of a PendingElement. Validates the element and its
    // subelements first, so that nothing is registered if it throws.
    element_id register_element(const element_ptr &el);
    void insert_element(const element_ptr &el);
    uint_fast64_t remove_subelements(element_ptr el);

    const OpticalPropertySet* get_optical_property_set(optics_id id) const;
    OpticalPropertySet* get_optical_property_set(optics_id id);
};

} // namespace SolTrace::Data

/**
 * @}
 */

#endif
