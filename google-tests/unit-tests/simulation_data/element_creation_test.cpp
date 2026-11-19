#include <gtest/gtest.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <vector>

#include <nlohmann/json.hpp>

#include <json_schema.hpp>

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

namespace {

// A model with a stage that holds a composite (two children) and one more
// single element. An element is created and dropped before `direct`, so the
// saved IDs have a gap.
struct SavedModel
{
    SimulationData sd;
    element_id stage_id  = 0;
    element_id comp_id   = 0;
    element_id child1_id = 0;
    element_id child2_id = 0;
    element_id direct_id = 0;

    SavedModel()
    {
        auto optics = add_optics(sd);

        auto stage = sd.create_element<StageElement>(0);
        auto comp  = stage->create_child<CompositeElement>();
        auto c1    = comp->create_child<SingleElement>();
        auto c2    = comp->create_child<SingleElement>();
        configure(*c1, optics);
        configure(*c2, optics);

        auto gap = stage->create_child<SingleElement>();
        configure(*gap, optics);
        auto direct = stage->create_child<SingleElement>();
        configure(*direct, optics);

        // Leave the gap's ID unused, so the saved IDs are not contiguous
        stage->remove_element(gap->get_id());

        stage_id  = stage.commit();
        comp_id   = comp->get_id();
        child1_id = c1->get_id();
        child2_id = c2->get_id();
        direct_id = direct->get_id();
    }
};

std::filesystem::path temp_json(const char *name)
{
    return std::filesystem::temp_directory_path() / name;
}

nlohmann::ordered_json read_json(const std::filesystem::path &path)
{
    nlohmann::ordered_json root;
    std::ifstream in(path);
    in >> root;
    return root;
}

void write_json(const std::filesystem::path &path,
                const nlohmann::ordered_json &root)
{
    std::ofstream out(path);
    out << root.dump(4);
}

std::vector<element_id> registered_ids(SimulationData &sd)
{
    std::vector<element_id> ids;
    for (auto it = sd.get_iterator(); !sd.is_at_end(it); ++it)
    {
        ids.push_back(it->first);
    }
    return ids;
}

} // namespace

TEST(ElementIdsJson, IdsSurviveWriteReadWrite)
{
    SavedModel model;
    const auto first  = temp_json("soltrace_ids_first.json");
    const auto second = temp_json("soltrace_ids_second.json");

    model.sd.export_json_file(first.string());

    SimulationData loaded;
    loaded.import_json_file(first.string());

    // Same IDs, including the gap
    EXPECT_EQ(registered_ids(loaded), registered_ids(model.sd));
    EXPECT_NE(loaded.get_element(model.direct_id), nullptr);
    EXPECT_EQ(loaded.get_element(model.direct_id - 1), nullptr);

    // Writing again gives the same file
    loaded.export_json_file(second.string());
    EXPECT_EQ(read_json(first), read_json(second));

    std::filesystem::remove(first);
    std::filesystem::remove(second);
}

TEST(ElementIdsJson, LegacyParabolaUpgradePreservesNestedIds)
{
    SavedModel model;
    const auto path = temp_json("soltrace_ids_legacy_parabola.json");
    model.sd.export_json_file(path.string());

    auto root = read_json(path);
    root["schema_version"] = "2026.07.15";
    ASSERT_EQ(root["elements"].size(), 1u);
    auto &stage = root["elements"].begin().value();
    auto &comp = stage.at("elements").at(std::to_string(model.comp_id));
    auto &child = comp.at("elements").at(std::to_string(model.child1_id));
    const auto parabola_type =
        SolTrace::Data::SurfaceTypeMap.at(SolTrace::Data::SurfaceType::PARABOLA);
    child["surface"] = {
        {"surface_type", parabola_type},
        {"focal_length_x", 2.0},
        {"focal_length_y", 4.0}
    };
    write_json(path, root);

    SimulationData loaded;
    ASSERT_NO_THROW(loaded.import_json_file(path.string()));
    EXPECT_EQ(registered_ids(loaded), registered_ids(model.sd));

    auto loaded_child = std::dynamic_pointer_cast<SingleElement>(
        loaded.get_element(model.child1_id));
    ASSERT_NE(loaded_child, nullptr);
    auto parabola = std::dynamic_pointer_cast<Parabola>(
        loaded_child->get_surface());
    ASSERT_NE(parabola, nullptr);
    EXPECT_DOUBLE_EQ(parabola->vertex_curv_x, 0.25);
    EXPECT_DOUBLE_EQ(parabola->vertex_curv_y, 0.125);

    loaded.export_json_file(path.string());
    const auto upgraded = read_json(path);
    EXPECT_EQ(upgraded.at("schema_version"), SolTrace::Data::kSchemaVersion);
    const auto &upgraded_stage = upgraded.at("elements").begin().value();
    const auto &upgraded_comp =
        upgraded_stage.at("elements").at(std::to_string(model.comp_id));
    const auto &upgraded_child =
        upgraded_comp.at("elements").at(std::to_string(model.child1_id));
    const auto &surface = upgraded_child.at("surface");
    EXPECT_EQ(upgraded_child.at("my_id"), model.child1_id);
    EXPECT_DOUBLE_EQ(surface.at("vertex_curv_x").get<double>(), 0.25);
    EXPECT_DOUBLE_EQ(surface.at("vertex_curv_y").get<double>(), 0.125);
    std::filesystem::remove(path);
}

TEST(ElementIdsJson, IdsIssuedAfterReloadDoNotCollide)
{
    SavedModel model;
    const auto path = temp_json("soltrace_ids_after_reload.json");
    model.sd.export_json_file(path.string());

    SimulationData loaded;
    loaded.import_json_file(path.string());
    std::filesystem::remove(path);

    element_id highest = 0;
    for (const element_id id : registered_ids(loaded))
    {
        highest = std::max(highest, id);
    }

    auto fresh = loaded.create_element<SingleElement>();
    EXPECT_GT(fresh.id(), highest);
}

TEST(ElementIdsJson, CompositeInsideStageIsWrittenOnceAndLoads)
{
    SavedModel model;
    const auto path = temp_json("soltrace_ids_nested.json");
    model.sd.export_json_file(path.string());

    // The composite appears once, inside the stage, and not at the top level
    const auto root = read_json(path);
    int composites_at_top = 0;
    for (auto &[key, jelement] : root.at("elements").items())
    {
        if (jelement.at("is_single") == false && !jelement.contains("is_stage"))
        {
            ++composites_at_top;
        }
    }
    EXPECT_EQ(composites_at_top, 0);

    SimulationData loaded;
    loaded.import_json_file(path.string());
    std::filesystem::remove(path);

    EXPECT_EQ(loaded.get_number_of_elements(), 3);
    auto comp = std::dynamic_pointer_cast<CompositeElement>(
        loaded.get_element(model.comp_id));
    ASSERT_NE(comp, nullptr);
    EXPECT_EQ(comp->get_number_of_elements(), 2);
    EXPECT_NE(comp->get_element(model.child1_id), nullptr);
    EXPECT_NE(comp->get_element(model.child2_id), nullptr);
}

TEST(ElementIdsJson, ChildrenAreKeyedAndOrderedByElementId)
{
    SimulationData sd;
    auto optics = add_optics(sd);

    auto stage = sd.create_element<StageElement>(0);
    auto comp  = stage->create_child<CompositeElement>();
    auto a     = comp->create_child<SingleElement>();
    auto b     = comp->create_child<SingleElement>();
    configure(*a, optics);
    configure(*b, optics);

    std::vector<element_id> keys;
    for (auto it = comp->get_const_iterator(); !comp->is_at_end(it); ++it)
    {
        EXPECT_EQ(it->first, it->second->get_id());
        keys.push_back(it->first);
    }
    EXPECT_EQ(keys, (std::vector<element_id>{ a->get_id(), b->get_id() }));
}

TEST(ElementIdsJson, MissingIdsAreAllocatedAboveTheSavedOnes)
{
    SavedModel model;
    const auto path = temp_json("soltrace_ids_missing.json");
    model.sd.export_json_file(path.string());

    // Remove the saved ID of the single element that is directly in the stage
    auto root = read_json(path);
    for (auto &[key, jstage] : root["elements"].items())
    {
        for (auto &[ckey, jchild] : jstage["elements"].items())
        {
            if (jchild.at("is_single") == true)
            {
                jchild.erase("my_id");
            }
        }
    }
    write_json(path, root);

    SimulationData loaded;
    loaded.import_json_file(path.string());
    std::filesystem::remove(path);

    // The stage, composite, and its children keep their IDs. The one without a
    // saved ID gets a new one above all the saved IDs.
    EXPECT_EQ(loaded.get_number_of_elements(), 3);
    EXPECT_NE(loaded.get_element(model.child1_id), nullptr);
    EXPECT_NE(loaded.get_element(model.child2_id), nullptr);
    EXPECT_EQ(loaded.get_element(model.direct_id), nullptr);

    int single_with_new_id = 0;
    for (const element_id id : registered_ids(loaded))
    {
        if (id > model.child2_id && loaded.get_element(id)->is_single())
        {
            ++single_with_new_id;
            EXPECT_GT(id, model.stage_id);
        }
    }
    EXPECT_EQ(single_with_new_id, 1);
}

TEST(ElementIdsJson, BadSavedIdsAreErrors)
{
    SavedModel model;
    const auto path = temp_json("soltrace_ids_bad.json");
    model.sd.export_json_file(path.string());
    const auto original = read_json(path);

    auto set_direct_id = [&](const nlohmann::ordered_json &value) {
        auto root = original;
        for (auto &[key, jstage] : root["elements"].items())
        {
            for (auto &[ckey, jchild] : jstage["elements"].items())
            {
                if (jchild.at("my_id") == model.direct_id)
                {
                    jchild["my_id"] = value;
                }
            }
        }
        write_json(path, root);
    };

    SimulationData loaded;

    // Duplicate of another element's saved ID
    set_direct_id(model.child1_id);
    EXPECT_THROW(loaded.import_json_file(path.string()), std::invalid_argument);

    // Below 1, and not an integer
    set_direct_id(0);
    EXPECT_THROW(loaded.import_json_file(path.string()), std::invalid_argument);
    set_direct_id(-2);
    EXPECT_THROW(loaded.import_json_file(path.string()), std::invalid_argument);
    set_direct_id("seven");
    EXPECT_THROW(loaded.import_json_file(path.string()), std::invalid_argument);
    set_direct_id(2.5);
    EXPECT_THROW(loaded.import_json_file(path.string()), std::invalid_argument);

    std::filesystem::remove(path);
}
