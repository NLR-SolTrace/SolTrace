#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <stdexcept>

#include <nlohmann/json.hpp>

#include "common.hpp"

using SolTrace::Data::ELEMENT_ALREADY_REGISTERED;
using SolTrace::Data::ELEMENT_INVALID_SETUP;
using SolTrace::Data::ELEMENT_NULL;
using SolTrace::Data::ELEMENT_WRONG_MODEL;

namespace {

OpticalPropertySetReference add_optics(SimulationData &sd)
{
    OpticalPropertySet opt_set(InteractionType::REFLECTION, "test_optics");
    opt_set.set_ideal_reflection(OpticalSide::Both);
    opt_set.set_errors(OpticalSide::Both, DistributionType::NONE, 0.0, 0.0);
    return sd.add_optical_property_set(opt_set);
}

void configure(SingleElement &el, const OpticalPropertySetReference &optics)
{
    el.set_aperture(make_aperture<Rectangle>(1.0, 1.0));
    el.set_surface(make_surface<Flat>());
    el.set_optical_property_set(optics);
}

} // namespace

TEST(ElementCreation, CommitRegistersTheWholeTreeInCreationOrder)
{
    SimulationData sd;
    auto optics = add_optics(sd);

    auto stage = sd.create_element<StageElement>(0);
    auto comp = stage->create_child<CompositeElement>();
    auto child1 = comp->create_child<SingleElement>();
    auto child2 = comp->create_child<SingleElement>();
    auto direct = stage->create_child<SingleElement>();
    configure(*child1, optics);
    configure(*child2, optics);
    configure(*direct, optics);

    EXPECT_EQ(stage.id(), 1);
    EXPECT_EQ(comp->get_id(), 2);
    EXPECT_EQ(child1->get_id(), 3);
    EXPECT_EQ(child2->get_id(), 4);
    EXPECT_EQ(direct->get_id(), 5);

    // Nothing is registered until the stage is committed
    EXPECT_EQ(sd.get_element(1), nullptr);
    EXPECT_EQ(sd.get_number_of_elements(), 0);

    EXPECT_EQ(stage.commit(), 1);
    EXPECT_EQ(sd.get_number_of_elements(), 3);
    EXPECT_EQ(sd.get_element(2), comp);
    EXPECT_EQ(sd.get_element(3), child1);
    EXPECT_EQ(sd.get_element(4), child2);
    EXPECT_EQ(sd.get_element(5), direct);
    EXPECT_EQ(child1->get_stage(), 0);
}

TEST(ElementCreation, ChildrenAddedLateCountAndJoinTheStage)
{
    SimulationData sd;
    auto optics = add_optics(sd);

    auto stage = sd.create_element<StageElement>(3);
    auto comp = stage->create_child<CompositeElement>();
    EXPECT_EQ(stage->get_number_of_elements(), 0);

    auto child = comp->create_child<SingleElement>();
    configure(*child, optics);
    EXPECT_EQ(comp->get_number_of_elements(), 1);
    EXPECT_EQ(stage->get_number_of_elements(), 1);
    EXPECT_EQ(child->get_stage(), 3);

    EXPECT_NO_THROW(stage.commit());
    EXPECT_EQ(sd.get_number_of_elements(), 1);
}

TEST(ElementCreation, CreateChildRejectsBadNesting)
{
    SimulationData sd;

    auto comp = sd.create_element<CompositeElement>();
    EXPECT_THROW(comp->create_child<StageElement>(0), std::invalid_argument);
    EXPECT_THROW(comp->create_child<CompositeElement>(), std::invalid_argument);
    EXPECT_EQ(comp->get_number_of_elements(), 0);

    auto stage = sd.create_element<StageElement>(0);
    EXPECT_THROW(stage->create_child<StageElement>(1), std::invalid_argument);
    EXPECT_NO_THROW(stage->create_child<CompositeElement>());
}

TEST(ElementCreation, CreateChildThrowsAfterRegistrationOrClear)
{
    SimulationData sd;
    auto optics = add_optics(sd);

    auto comp = sd.create_element<CompositeElement>();
    configure(*comp->create_child<SingleElement>(), optics);
    comp.commit();
    EXPECT_THROW(comp->create_child<SingleElement>(), std::invalid_argument);

    auto other = sd.create_element<CompositeElement>();
    sd.clear();
    EXPECT_THROW(other->create_child<SingleElement>(), std::invalid_argument);
}

TEST(ElementCreation, AddChildReturnsTheChildIdAndEmptiesTheHandle)
{
    SimulationData sd;
    auto optics = add_optics(sd);

    auto comp = sd.create_element<CompositeElement>();
    auto child = sd.create_element<SingleElement>();
    configure(*child, optics);
    const element_id child_id = child.id();

    EXPECT_EQ(comp->add_child(std::move(child)), child_id);
    EXPECT_EQ(comp->get_number_of_elements(), 1);
    EXPECT_THROW(child.commit(), std::invalid_argument);
}

TEST(ElementCreation, AddChildFailuresLeaveTheHandleUsable)
{
    SimulationData sd;
    SimulationData other;
    auto optics = add_optics(sd);

    auto comp = sd.create_element<CompositeElement>();

    // An empty handle
    auto moved = sd.create_element<SingleElement>();
    auto sink = std::move(moved);
    EXPECT_EQ(comp->add_child(std::move(moved)), ELEMENT_NULL);

    // A child from another model
    auto foreign = other.create_element<SingleElement>();
    EXPECT_EQ(comp->add_child(std::move(foreign)), ELEMENT_WRONG_MODEL);
    EXPECT_NE(foreign.get(), nullptr);

    // A child that is already registered
    auto done = sd.create_element<SingleElement>();
    configure(*done, optics);
    done.commit();
    EXPECT_EQ(comp->add_child(std::move(done)), ELEMENT_ALREADY_REGISTERED);
    EXPECT_NE(done.get(), nullptr);

    // A stage cannot be a child, and the stage's handle is not consumed
    auto stage = sd.create_element<StageElement>(0);
    EXPECT_EQ(comp->add_child(std::move(stage)), ELEMENT_INVALID_SETUP);
    EXPECT_NE(stage.get(), nullptr);

    // A registered composite cannot take children
    auto registered = sd.create_element<CompositeElement>();
    configure(*registered->create_child<SingleElement>(), optics);
    registered.commit();
    auto late = sd.create_element<SingleElement>();
    EXPECT_EQ(registered->add_child(std::move(late)), ELEMENT_INVALID_SETUP);
    EXPECT_NE(late.get(), nullptr);

    EXPECT_EQ(comp->get_number_of_elements(), 0);
}

TEST(ElementCreation, FailedCommitRegistersNothingAndCanBeRetried)
{
    SimulationData sd;
    auto optics = add_optics(sd);

    auto comp = sd.create_element<CompositeElement>();
    auto good = comp->create_child<SingleElement>();
    auto bad = comp->create_child<SingleElement>();
    configure(*good, optics);

    EXPECT_THROW(comp.commit(), std::invalid_argument);
    EXPECT_EQ(sd.get_number_of_elements(), 0);
    EXPECT_EQ(sd.get_element(comp.id()), nullptr);
    EXPECT_EQ(sd.get_element(good->get_id()), nullptr);

    configure(*bad, optics);
    EXPECT_EQ(comp.commit(), comp.id());
    EXPECT_EQ(sd.get_number_of_elements(), 2);

    // A handle commits once
    EXPECT_THROW(comp.commit(), std::invalid_argument);
}

TEST(ElementCreation, RemovedElementKeepsItsIdAndCannotBeCommittedAgain)
{
    SimulationData sd;
    auto optics = add_optics(sd);

    auto el = sd.create_element<SingleElement>();
    configure(*el, optics);
    const element_id id = el.commit();

    EXPECT_EQ(sd.remove_element(id), 1);
    EXPECT_EQ(el->get_id(), id);
    EXPECT_THROW(el.commit(), std::invalid_argument);

    // The ID is not issued again
    auto next = sd.create_element<SingleElement>();
    EXPECT_GT(next.id(), id);
}

TEST(ElementCreation, RemoveElementMaintainsTheGroupIndex)
{
    SimulationData original;
    auto optics = add_optics(original);
    auto el = original.create_element<SingleElement>();
    configure(*el, optics);
    el.commit();

    const auto path = std::filesystem::temp_directory_path() /
                      "soltrace_element_creation_group.json";
    original.export_json_file(path.string());

    nlohmann::ordered_json root;
    {
        std::ifstream in(path);
        in >> root;
    }
    for (auto &[key, jelement] : root.at("elements").items())
    {
        jelement["group"] = 2;
    }
    {
        std::ofstream out(path);
        out << root.dump(4);
    }

    SimulationData sd;
    sd.import_json_file(path.string());
    std::filesystem::remove(path);

    ASSERT_EQ(sd.get_groups().size(), 3);
    ASSERT_EQ(sd.get_groups()[2].size(), 1);
    const element_id id = *sd.get_groups()[2].begin();

    EXPECT_EQ(sd.remove_element(id), 1);
    EXPECT_TRUE(sd.get_groups()[2].empty());
}
