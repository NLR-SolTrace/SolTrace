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
    auto unconfigured_element = SolTrace::Data::make_element<SingleElement>();

    // Adding an unconfigured element should throw an exception
    EXPECT_THROW(my_sim.add_element(unconfigured_element), std::invalid_argument);

    // The element should not be added to the simulation
    EXPECT_EQ(my_sim.get_number_of_elements(), 0);

    // The element should not have been assigned an ID
    EXPECT_EQ(unconfigured_element->get_id(), SolTrace::Data::ELEMENT_ID_UNASSIGNED);
}

TEST(SimulationData, AddPartiallyConfiguredSingleElementThrowsError)
{
    SimulationData my_sim;

    // Create a SingleElement with only aperture set (missing surface)
    auto partially_configured = SolTrace::Data::make_element<SingleElement>();
    auto aperture = SolTrace::Data::make_aperture<SolTrace::Data::Rectangle>(1.0, 1.0);
    partially_configured->set_aperture(aperture);

    // Adding a partially configured element should throw an exception
    EXPECT_THROW(my_sim.add_element(partially_configured), std::invalid_argument);

    // The element should not be added to the simulation
    EXPECT_EQ(my_sim.get_number_of_elements(), 0);

    // The element should not have been assigned an ID
    EXPECT_EQ(partially_configured->get_id(), SolTrace::Data::ELEMENT_ID_UNASSIGNED);
}

TEST(SimulationData, AddUnconfiguredCompositeElementThrowsError)
{
    SimulationData my_sim;

    // Create an empty CompositeElement (no subelements)
    auto empty_composite = SolTrace::Data::make_element<CompositeElement>();

    // Adding an empty composite element should throw an exception
    EXPECT_THROW(my_sim.add_element(empty_composite), std::invalid_argument);

    // The element should not be added to the simulation
    EXPECT_EQ(my_sim.get_number_of_elements(), 0);

    // The element should not have been assigned an ID
    EXPECT_EQ(empty_composite->get_id(), SolTrace::Data::ELEMENT_ID_UNASSIGNED);
}

TEST(SimulationData, AddProperlyConfiguredElementsSucceeds)
{
    SimulationData my_sim;

    // Make dummy optical properties
    OpticalPropertySet dummy_optics;
    auto optics_ref = my_sim.add_optical_property_set(dummy_optics);

    // Create a properly configured SingleElement
    auto configured_single = SolTrace::Data::make_element<SingleElement>();
    auto aperture = SolTrace::Data::make_aperture<SolTrace::Data::Rectangle>(1.0, 1.0);
    auto surface = SolTrace::Data::make_surface<SolTrace::Data::Flat>();
    configured_single->set_aperture(aperture);
    configured_single->set_surface(surface);
    configured_single->set_optical_property_set(optics_ref);

    // Adding a properly configured element should succeed
    EXPECT_NO_THROW(my_sim.add_element(configured_single));
    EXPECT_EQ(my_sim.get_number_of_elements(), 1);
    EXPECT_NE(configured_single->get_id(), SolTrace::Data::ELEMENT_ID_UNASSIGNED);

    // Create a properly configured CompositeElement
    auto configured_composite = SolTrace::Data::make_element<CompositeElement>();
    auto configured_sub1 = SolTrace::Data::make_element<SingleElement>();
    auto aperture2 = SolTrace::Data::make_aperture<SolTrace::Data::Rectangle>(2.0, 2.0);
    auto surface2 = SolTrace::Data::make_surface<SolTrace::Data::Flat>();
    configured_sub1->set_aperture(aperture2);
    configured_sub1->set_surface(surface2);
    configured_sub1->set_optical_property_set(optics_ref);
    configured_composite->add_element(configured_sub1);

    // Adding a properly configured composite should succeed
    EXPECT_NO_THROW(my_sim.add_element(configured_composite));
    EXPECT_EQ(my_sim.get_number_of_elements(), 2); // 1 + 1 subelement
    EXPECT_NE(configured_composite->get_id(), SolTrace::Data::ELEMENT_ID_UNASSIGNED);
    EXPECT_NE(configured_sub1->get_id(), SolTrace::Data::ELEMENT_ID_UNASSIGNED);
}

TEST(SimulationData, ValidationErrorPreservesSimulationState)
{
    SimulationData my_sim;

    // Add a properly configured element first
    auto good_element = make_configured_element(my_sim);
    my_sim.add_element(good_element);

    EXPECT_EQ(my_sim.get_number_of_elements(), 1);
    auto initial_count = my_sim.get_number_of_elements();

    // Try to add an unconfigured element
    auto bad_element = SolTrace::Data::make_element<SingleElement>();
    EXPECT_THROW(my_sim.add_element(bad_element), std::invalid_argument);

    // The simulation state should be unchanged
    EXPECT_EQ(my_sim.get_number_of_elements(), initial_count);
    EXPECT_EQ(bad_element->get_id(), SolTrace::Data::ELEMENT_ID_UNASSIGNED);

    // The good element should still be accessible
    EXPECT_EQ(my_sim.get_element(good_element->get_id()), good_element);
}

TEST(SimulationData, AddRemoveGetElements)
{
    SimulationData my_sim;

    auto my_reflector = make_configured_element(my_sim);
    auto id1 = my_sim.add_element(my_reflector);
    EXPECT_EQ(id1, my_reflector->get_id());
    EXPECT_EQ(my_sim.get_number_of_elements(), 1);

    auto my_comp = SolTrace::Data::make_element<CompositeElement>();
    auto sub1 = make_configured_element(my_sim);
    auto sub2 = make_configured_element(my_sim);
    my_comp->add_element(sub1);
    my_comp->add_element(sub2);
    EXPECT_EQ(my_comp->get_number_of_elements(), 2);

    auto id2 = my_sim.add_element(my_comp);
    EXPECT_EQ(id2, my_comp->get_id());
    EXPECT_NE(id1, id2);
    EXPECT_NE(sub1->get_id(), SolTrace::Data::ELEMENT_ID_UNASSIGNED);

    EXPECT_EQ(my_sim.get_number_of_elements(), 3);
    EXPECT_EQ(my_sim.get_element(id1), my_reflector);
    EXPECT_EQ(my_sim.get_element(id2), my_comp);
    EXPECT_EQ(my_sim.get_element(sub1->get_id()), sub1);

    EXPECT_EQ(my_sim.add_element(sub1), SolTrace::Data::ELEMENT_ALREADY_REGISTERED);
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

    EXPECT_EQ(my_reflector->get_id(), SolTrace::Data::ELEMENT_ID_UNASSIGNED);
    EXPECT_EQ(my_comp->get_id(), SolTrace::Data::ELEMENT_ID_UNASSIGNED);
    EXPECT_EQ(sub1->get_id(), SolTrace::Data::ELEMENT_ID_UNASSIGNED);
}

TEST(SimulationData, IteratorElement)
{
    SimulationData my_sim;

    auto el1 = make_configured_element(my_sim);
    auto el2 = make_configured_element(my_sim);
    auto cmp1 = SolTrace::Data::make_element<CompositeElement>();
    auto sub1 = make_configured_element(my_sim);
    auto sub2 = make_configured_element(my_sim);
    cmp1->add_element(sub1);
    cmp1->add_element(sub2);
    auto cmp2 = SolTrace::Data::make_element<CompositeElement>();
    auto sub3 = make_configured_element(my_sim);
    auto sub4 = make_configured_element(my_sim);
    auto sub5 = make_configured_element(my_sim);
    cmp2->add_element(sub3);
    cmp2->add_element(sub4);
    cmp2->add_element(sub5);

    my_sim.add_element(el1);
    my_sim.add_element(el2);
    my_sim.add_element(cmp1);
    my_sim.add_element(cmp2);
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
