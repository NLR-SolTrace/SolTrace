#include <gtest/gtest.h>

#include <composite_element.hpp>
#include <single_element.hpp>
#include <sun.hpp>
#include <simulation_data.hpp>

#include "common.hpp"

TEST(SimulationData, AddUnconfiguredSingleElementThrowsError)
{
    SimulationData my_sim;

    // Create an unconfigured SingleElement (no aperture or surface set)
    auto unconfigured_element = new_element<SingleElement>(my_sim);

    // Adding an unconfigured element should throw an exception
    EXPECT_THROW(register_element(my_sim, unconfigured_element), std::invalid_argument);

    // The element should not be added to the simulation
    EXPECT_EQ(my_sim.get_number_of_elements(), 0);

    // The element has its ID but is not registered
    EXPECT_GT(unconfigured_element->get_id(), 0);
    EXPECT_EQ(my_sim.get_element(unconfigured_element->get_id()), nullptr);
}

TEST(SimulationData, AddPartiallyConfiguredSingleElementThrowsError)
{
    SimulationData my_sim;

    // Create a SingleElement with only aperture set (missing surface)
    auto partially_configured = new_element<SingleElement>(my_sim);
    auto aperture = SolTrace::Data::make_aperture<SolTrace::Data::Rectangle>(1.0, 1.0);
    partially_configured->set_aperture(aperture);

    // Adding a partially configured element should throw an exception
    EXPECT_THROW(register_element(my_sim, partially_configured), std::invalid_argument);

    // The element should not be added to the simulation
    EXPECT_EQ(my_sim.get_number_of_elements(), 0);

    // The element has its ID but is not registered
    EXPECT_GT(partially_configured->get_id(), 0);
    EXPECT_EQ(my_sim.get_element(partially_configured->get_id()), nullptr);
}

TEST(SimulationData, AddUnconfiguredCompositeElementThrowsError)
{
    SimulationData my_sim;

    // Create an empty CompositeElement (no subelements)
    auto empty_composite = new_element<CompositeElement>(my_sim);

    // Adding an empty composite element should throw an exception
    EXPECT_THROW(register_element(my_sim, empty_composite), std::invalid_argument);

    // The element should not be added to the simulation
    EXPECT_EQ(my_sim.get_number_of_elements(), 0);

    // The element has its ID but is not registered
    EXPECT_GT(empty_composite->get_id(), 0);
    EXPECT_EQ(my_sim.get_element(empty_composite->get_id()), nullptr);
}

TEST(SimulationData, AddProperlyConfiguredElementsSucceeds)
{
    SimulationData my_sim;

    // Make dummy optical properties
    OpticalPropertySet dummy_optics;
    auto optics_ref = my_sim.add_optical_property_set(dummy_optics);

    // Create a properly configured SingleElement
    auto aperture = SolTrace::Data::make_aperture<SolTrace::Data::Rectangle>(1.0, 1.0);
    auto surface = SolTrace::Data::make_surface<SolTrace::Data::Flat>();
    auto configured_single = make_single(my_sim, aperture, surface, optics_ref);

    // Adding a properly configured element should succeed
    EXPECT_NO_THROW(register_element(my_sim, configured_single));
    EXPECT_EQ(my_sim.get_number_of_elements(), 1);
    EXPECT_EQ(my_sim.get_element(configured_single->get_id()), configured_single);

    // Create a properly configured CompositeElement
    auto configured_composite = new_element<CompositeElement>(my_sim);
    auto aperture2 = SolTrace::Data::make_aperture<SolTrace::Data::Rectangle>(2.0, 2.0);
    auto surface2 = SolTrace::Data::make_surface<SolTrace::Data::Flat>();
    auto configured_sub1 = make_single(my_sim, aperture2, surface2, optics_ref);
    attach_child(configured_composite, configured_sub1);

    // Adding a properly configured composite should succeed
    EXPECT_NO_THROW(register_element(my_sim, configured_composite));
    EXPECT_EQ(my_sim.get_number_of_elements(), 2); // 1 + 1 subelement
    EXPECT_EQ(my_sim.get_element(configured_composite->get_id()), configured_composite);
    EXPECT_EQ(my_sim.get_element(configured_sub1->get_id()), configured_sub1);
}

TEST(SimulationData, ValidationErrorPreservesSimulationState)
{
    SimulationData my_sim;

    // Add a properly configured element first
    auto good_element = make_configured_element(my_sim);
    register_element(my_sim, good_element);

    EXPECT_EQ(my_sim.get_number_of_elements(), 1);
    auto initial_count = my_sim.get_number_of_elements();

    // Try to add an unconfigured element
    auto bad_element = new_element<SingleElement>(my_sim);
    EXPECT_THROW(register_element(my_sim, bad_element), std::invalid_argument);

    // The simulation state should be unchanged
    EXPECT_EQ(my_sim.get_number_of_elements(), initial_count);
    EXPECT_EQ(my_sim.get_element(bad_element->get_id()), nullptr);

    // The good element should still be accessible
    EXPECT_EQ(my_sim.get_element(good_element->get_id()), good_element);
}

TEST(SimulationData, AddRemoveGetElements)
{
    SimulationData my_sim;

    auto my_reflector = make_configured_element(my_sim);
    auto id1 = register_element(my_sim, my_reflector);
    EXPECT_EQ(id1, my_reflector->get_id());
    EXPECT_EQ(my_sim.get_number_of_elements(), 1);

    auto my_comp = new_element<CompositeElement>(my_sim);
    auto sub1 = make_configured_element(my_sim);
    auto sub2 = make_configured_element(my_sim);
    attach_child(my_comp, sub1);
    attach_child(my_comp, sub2);
    EXPECT_EQ(my_comp->get_number_of_elements(), 2);

    auto id2 = register_element(my_sim, my_comp);
    EXPECT_EQ(id2, my_comp->get_id());
    EXPECT_NE(id1, id2);

    EXPECT_EQ(my_sim.get_number_of_elements(), 3);
    EXPECT_EQ(my_sim.get_element(id1), my_reflector);
    EXPECT_EQ(my_sim.get_element(id2), my_comp);
    EXPECT_EQ(my_sim.get_element(sub1->get_id()), sub1);

    EXPECT_EQ(register_element(my_sim, sub1), SolTrace::Data::ELEMENT_ALREADY_REGISTERED);
    EXPECT_EQ(my_sim.get_number_of_elements(), 3);

    element_id nonexistant = id2 + 1000;
    EXPECT_EQ(my_sim.get_element(nonexistant), nullptr);
    EXPECT_EQ(my_sim.remove_element(nonexistant), 0);
    EXPECT_EQ(my_sim.get_number_of_elements(), 3);

    EXPECT_EQ(my_sim.remove_element(id1), 1);
    EXPECT_EQ(my_sim.get_number_of_elements(), 2);
    EXPECT_EQ(my_sim.get_element(id1), nullptr);

    auto id3 = sub1->get_id();
    EXPECT_EQ(my_sim.remove_element(id2), 2);
    EXPECT_EQ(my_sim.get_number_of_elements(), 0);
    EXPECT_EQ(my_sim.get_element(id2), nullptr);
    EXPECT_EQ(my_sim.get_element(id3), nullptr);

    // Removing does not change the IDs, and the elements are no longer registered
    EXPECT_EQ(my_reflector->get_id(), id1);
    EXPECT_EQ(my_comp->get_id(), id2);
    EXPECT_EQ(sub1->get_id(), id3);
}

TEST(SimulationData, IteratorElement)
{
    SimulationData my_sim;

    auto el1 = make_configured_element(my_sim);
    auto el2 = make_configured_element(my_sim);
    auto cmp1 = new_element<CompositeElement>(my_sim);
    auto sub1 = make_configured_element(my_sim);
    auto sub2 = make_configured_element(my_sim);
    attach_child(cmp1, sub1);
    attach_child(cmp1, sub2);
    auto cmp2 = new_element<CompositeElement>(my_sim);
    auto sub3 = make_configured_element(my_sim);
    auto sub4 = make_configured_element(my_sim);
    auto sub5 = make_configured_element(my_sim);
    attach_child(cmp2, sub3);
    attach_child(cmp2, sub4);
    attach_child(cmp2, sub5);

    register_element(my_sim, el1);
    register_element(my_sim, el2);
    register_element(my_sim, cmp1);
    register_element(my_sim, cmp2);
    EXPECT_EQ(my_sim.get_number_of_elements(), 7);

    std::set<element_id> ids;
    std::set<element_id> cids;
    for (auto iter = my_sim.get_iterator();
         !my_sim.is_at_end(iter);
         ++iter)
    {
        if (iter->second->is_single())
        {
            ids.insert(iter->first);
        }
        if (iter->second->is_composite())
        {
            cids.insert(iter->first);
        }
        EXPECT_EQ(iter->first, iter->second->get_id());
    }
    EXPECT_EQ(ids.size(), my_sim.get_number_of_elements());
    EXPECT_EQ(cids.size(), 2);

    ids.clear();
    cids.clear();
    for (auto iter = my_sim.get_const_iterator();
         !my_sim.is_at_end(iter);
         ++iter)
    {
        if (iter->second->is_single())
        {
            ids.insert(iter->first);
        }
        if (iter->second->is_composite())
        {
            cids.insert(iter->first);
        }
        EXPECT_EQ(iter->first, iter->second->get_id());
    }
    EXPECT_EQ(ids.size(), my_sim.get_number_of_elements());
    EXPECT_EQ(cids.size(), 2);
}

TEST(SimulationData, RaySourceInterface)
{
    SimulationData my_sim;

    auto src1 = SolTrace::Data::make_ray_source<Sun>();
    auto id1 = my_sim.add_ray_source(src1);
    auto src2 = SolTrace::Data::make_ray_source<Sun>();
    auto id2 = my_sim.add_ray_source(src2);

    EXPECT_EQ(my_sim.get_ray_source(id1), src1);
    EXPECT_EQ(my_sim.get_ray_source(id2), src2);
    EXPECT_NE(id1, id2);

    ray_source_id nonexistant = id2 + 1000;
    EXPECT_EQ(my_sim.get_ray_source(nonexistant), nullptr);
    EXPECT_EQ(my_sim.remove_ray_source(nonexistant), 0);

    EXPECT_EQ(my_sim.remove_ray_source(id1), 1);
}

TEST(SimulationData, ParametersInterface)
{
    SimulationData my_sim;

    // auto params = my_sim.get_simulation_parameters();
    const SimulationParameters &params = my_sim.get_simulation_parameters();
    SimulationParameters &mutable_params = my_sim.get_simulation_parameters();

    const int NRAYS = 10;
    my_sim.set_number_of_rays(NRAYS);
    EXPECT_EQ(my_sim.get_number_of_rays(), NRAYS);

    const double TOL = 1e-8;
    my_sim.set_tolerance(TOL);
    EXPECT_EQ(my_sim.get_tolerance(), TOL);

    const int SEED = 12345;
    my_sim.set_seed(SEED);
    EXPECT_EQ(my_sim.get_seed(), SEED);

    const double LAT = 1.25;
    my_sim.set_latitude(LAT);
    EXPECT_EQ(my_sim.get_latitude(), LAT);

    const double LONG = 0.75;
    my_sim.set_longitude(LONG);
    EXPECT_EQ(my_sim.get_longitude(), LONG);

    // TODO: Add datetime accessor tests

    EXPECT_EQ(params.seed, SEED);
    EXPECT_EQ(mutable_params.seed, SEED);
    mutable_params.seed = 0;
    EXPECT_EQ(my_sim.get_seed(), 0);
}
