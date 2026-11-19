#include <gtest/gtest.h>

#include <cmath>
#include <limits>
#include <vector>

#include <nlohmann/json.hpp>

#include <simulation_data_export.hpp>


#include "common.hpp"

TEST(OpticalProperties, DangleOpticalPointer)
{
    SimulationData sd;

    // Make optical property set
    auto opt_set = OpticalPropertySet();
    opt_set.set_ideal_reflection(OpticalSide::Both);

    // Add to simulation data
    auto opt_ref = sd.add_optical_property_set(opt_set);

    // Make element with the optical property reference attached
    auto test_element = make_single(sd, make_aperture<Circle>(1), make_surface<Flat>(), opt_ref);

    // Add element to simulation data
    ASSERT_NO_THROW(register_element(sd, test_element));

    sd.clear();

    EXPECT_EQ(sd.get_optical_property_set(*test_element), nullptr);
}

TEST(OpticalProperties, MissingOptical)
{
    SimulationData sd;

    // Make element
    auto test_element = new_element<SingleElement>(sd);
    set_element_shape(sd, test_element, make_aperture<Circle>(1), make_surface<Flat>());

    // Add element to simulation data
    EXPECT_THROW(register_element(sd, test_element), std::invalid_argument);
}

TEST(OpticalProperties, InvalidErrorSettingsThrow)
{
    OpticalPropertySet optics(InteractionType::REFLECTION, "invalid_errors");
    optics.set_ideal_reflection(OpticalSide::Both);

    EXPECT_THROW(optics.set_errors(
                     OpticalSide::Both, DistributionType::UNKNOWN, 1.0, 1.0),
                 std::invalid_argument);
    EXPECT_THROW(optics.set_errors(
                     OpticalSide::Both, DistributionType::GAUSSIAN, -1.0, 1.0),
                 std::invalid_argument);
    EXPECT_THROW(optics.set_errors(OpticalSide::Both,
                                   DistributionType::GAUSSIAN,
                                   1.0,
                                   std::numeric_limits<double>::quiet_NaN()),
                 std::invalid_argument);
}

TEST(OpticalProperties, MutateOptics)
{
    SimulationData sd;

    // Make optical property set
    auto opt_set = OpticalPropertySet();
    opt_set.set_ideal_reflection(OpticalSide::Both);

    // Add to simulation data
    auto opt_ref = sd.add_optical_property_set(opt_set);

    // Make element 1 with the optical property reference attached
    auto test_element_1 = make_single(sd, make_aperture<Circle>(1), make_surface<Flat>(), opt_ref);

    // Make element 2 with the same optical properties attached
    auto test_element_2 = make_single(sd, make_aperture<Circle>(1), make_surface<Flat>(), opt_ref);

    // Modify optical properties directly from simulation data
    auto* optics_ptr = sd.get_mutable_optical_property_set(*test_element_1);
    optics_ptr->set_reflectivity(OpticalSide::Front, 0.5);

    // Confirm reflectivity is modified for both
    EXPECT_EQ(test_element_1->get_optical_property_set()->get_reflectivity(
                  OpticalSide::Front),
              0.5);
    EXPECT_EQ(test_element_2->get_optical_property_set()->get_reflectivity(
                  OpticalSide::Front),
              0.5);

    EXPECT_EQ(test_element_1->get_optical_property_set(),
              test_element_2->get_optical_property_set());
}

TEST(OpticalProperties, RemovedOpticsExpire)
{
    SimulationData sd;

    OpticalPropertySet opt_set;
    opt_set.set_ideal_reflection(OpticalSide::Both);
    auto ref = sd.add_optical_property_set(opt_set);

    auto test_element = make_single(sd, make_aperture<Circle>(1), make_surface<Flat>(), ref);
    register_element(sd, test_element);

    // Allowed while the element still uses the entry
    EXPECT_TRUE(sd.remove_optical_property_set(ref.id()));
    EXPECT_FALSE(sd.remove_optical_property_set(ref.id()));

    EXPECT_FALSE(ref.valid());
    EXPECT_EQ(test_element->get_optical_property_set(), nullptr);
    EXPECT_EQ(test_element->get_optical_property_set_id(), ref.id());
    EXPECT_THROW(test_element->enforce_user_fields_set(), std::invalid_argument);

    // An element given the expired reference later cannot be committed
    auto late = sd.create_element<SingleElement>();
    late->set_aperture(make_aperture<Circle>(1));
    late->set_surface(make_surface<Flat>());
    late->set_optical_property_set(ref);
    EXPECT_THROW(late.commit(), std::invalid_argument);
}

TEST(OpticalProperties, RemovedOpticsExpireWhileTheValueIsStillHeld)
{
    SimulationData sd;

    OpticalPropertySet opt_set;
    opt_set.set_ideal_reflection(OpticalSide::Both);
    auto ref = sd.add_optical_property_set(opt_set);

    auto test_element = make_single(sd, make_aperture<Circle>(1), make_surface<Flat>(), ref);
    register_element(sd, test_element);

    const auto held = ref.get();
    ASSERT_NE(held, nullptr);
    ASSERT_TRUE(sd.remove_optical_property_set(ref.id()));

    // The value outlives the entry, but no reference resolves to it
    EXPECT_FALSE(ref.valid());
    EXPECT_FALSE(sd.get_optical_property_set(ref.id()).valid());
    EXPECT_EQ(test_element->get_optical_property_set(), nullptr);
    EXPECT_THROW(test_element->enforce_user_fields_set(), std::invalid_argument);

    // A new entry gets a new ID, and old references stay expired
    auto replacement = sd.add_optical_property_set(opt_set);
    EXPECT_NE(replacement.id(), ref.id());
    EXPECT_FALSE(ref.valid());
}

TEST(OpticalProperties, ClearedOpticsKeepIdCounter)
{
    SimulationData sd;

    OpticalPropertySet opt_set;
    opt_set.set_ideal_reflection(OpticalSide::Both);

    EXPECT_EQ(sd.add_optical_property_set(opt_set).id(), 0);
    EXPECT_EQ(sd.add_optical_property_set(opt_set).id(), 1);

    sd.clear_optical_property_sets();
    EXPECT_EQ(sd.get_number_of_optical_property_sets(), 0);
    EXPECT_EQ(sd.add_optical_property_set(opt_set).id(), 2);

    // Clearing the whole model starts the numbering again
    sd.clear();
    EXPECT_EQ(sd.add_optical_property_set(opt_set).id(), 0);
}

TEST(OpticalProperties, MutableAccessIsLimitedToThisModel)
{
    SimulationData sd;
    SimulationData other_sd;

    OpticalPropertySet opt_set;
    opt_set.set_ideal_reflection(OpticalSide::Both);
    auto ref = sd.add_optical_property_set(opt_set);
    other_sd.add_optical_property_set(opt_set);

    auto test_element = make_single(sd, make_aperture<Circle>(1), make_surface<Flat>(), ref);

    EXPECT_NE(sd.get_mutable_optical_property_set(*test_element), nullptr);
    // other_sd has an entry with the same ID, but it is not the element's
    EXPECT_EQ(other_sd.get_mutable_optical_property_set(*test_element), nullptr);
    EXPECT_EQ(other_sd.get_optical_property_set(*test_element), nullptr);
}

TEST(OpticalProperties, VirtualElementsShareBuiltInOptics)
{
    SimulationData sd;

    auto first = sd.create_element<VirtualElement>();
    auto second = sd.create_element<VirtualElement>();

    EXPECT_EQ(first->get_optical_property_set_id(), OPTICS_ID_VIRTUAL);
    EXPECT_EQ(first->get_optical_property_set(), second->get_optical_property_set());
    EXPECT_EQ(first->get_optical_property_set(), virtual_optics().get());

    // Built-ins are not entries of the model and cannot be changed through it
    EXPECT_EQ(sd.get_number_of_optical_property_sets(), 0);
    EXPECT_EQ(sd.get_mutable_optical_property_set(*first), nullptr);
    EXPECT_EQ(sd.get_optical_property_set(*first), virtual_optics().get().get());

    // Every model accepts them
    SimulationData other_sd;
    auto other = other_sd.create_element<SingleElement>();
    EXPECT_NO_THROW(other->set_optical_property_set(virtual_optics()));
}

TEST(OpticalProperties, ElementCanAccessSimulationOwnedOptics)
{
    SimulationData sd;

    OpticalPropertySet opt_set;
    opt_set.set_ideal_reflection(OpticalSide::Both);

    auto opt_ref = sd.add_optical_property_set(opt_set);

    auto test_element = make_single(sd, make_aperture<Circle>(1), make_surface<Flat>(), opt_ref);

    EXPECT_NE(test_element->get_optical_property_set(), nullptr);
    EXPECT_EQ(test_element->get_optical_property_set()->get_reflectivity(
                  OpticalSide::Front),
              1.0);
}

TEST(OpticalProperties, RemovingElementDoesNotRemoveOptics)
{
    SimulationData sd;

    OpticalPropertySet opt_set;
    opt_set.set_ideal_reflection(OpticalSide::Both);

    auto opt_ref = sd.add_optical_property_set(opt_set);

    auto test_element = make_single(sd, make_aperture<Circle>(1), make_surface<Flat>(), opt_ref);

    auto id = register_element(sd, test_element);
    ASSERT_TRUE(SolTrace::Data::Element::is_success(id));

    sd.remove_element(id);

    EXPECT_NE(sd.get_optical_property_set(*test_element), nullptr);
}

TEST(OpticalProperties, FindOrAddDeduplicatesEquivalentOptics)
{
    SimulationData sd;

    OpticalPropertySet opt_set_1;
    opt_set_1.set_ideal_reflection(OpticalSide::Both);

    OpticalPropertySet opt_set_2;
    opt_set_2.set_ideal_reflection(OpticalSide::Both);

    auto ref_1 = sd.find_or_add_optical_property_set(opt_set_1);
    auto ref_2 = sd.find_or_add_optical_property_set(opt_set_2);

    EXPECT_EQ(ref_1.id(), ref_2.id());
    EXPECT_EQ(ref_1.get(), ref_2.get());
}

TEST(OpticalProperties, FindOrAddKeepsDifferentOpticsSeparate)
{
    SimulationData sd;

    OpticalPropertySet opt_set_1;
    opt_set_1.set_ideal_reflection(OpticalSide::Both);

    OpticalPropertySet opt_set_2;
    opt_set_2.set_ideal_reflection(OpticalSide::Both);
    opt_set_2.set_reflectivity(OpticalSide::Front, 0.5);

    auto ref_1 = sd.find_or_add_optical_property_set(opt_set_1);
    auto ref_2 = sd.find_or_add_optical_property_set(opt_set_2);

    EXPECT_NE(ref_1.id(), ref_2.id());
    EXPECT_NE(ref_1.get(), ref_2.get());
}

TEST(OpticalProperties, OpticsFromAnotherModelAreRejected)
{
    SimulationData sd;

    OpticalPropertySet opt_set;
    opt_set.set_ideal_reflection(OpticalSide::Both);
    auto own_ref = sd.add_optical_property_set(opt_set);

    auto test_element = new_element<SingleElement>(sd);
    set_element_shape(sd, test_element, make_aperture<Circle>(1), make_surface<Flat>());
    test_element->set_optical_property_set(own_ref);

    {
        SimulationData temporary_sd;
        auto other_ref = temporary_sd.add_optical_property_set(opt_set);

        // Rejected even though the ID matches an entry of `sd`
        ASSERT_EQ(other_ref.id(), own_ref.id());
        EXPECT_THROW(test_element->set_optical_property_set(other_ref),
                     std::invalid_argument);
    }

    // The rejected assignment left the element's optics unchanged
    EXPECT_EQ(test_element->get_optical_property_set(), own_ref.get());
    EXPECT_NO_THROW(register_element(sd, test_element));
}

TEST(OpticalProperties, OpticsFromBeforeClearAreRejected)
{
    SimulationData sd;

    OpticalPropertySet opt_set;
    opt_set.set_ideal_reflection(OpticalSide::Both);
    auto old_ref = sd.add_optical_property_set(opt_set);

    sd.clear();
    // The new entry has the same ID as the old one
    auto new_ref = sd.add_optical_property_set(opt_set);
    ASSERT_EQ(new_ref.id(), old_ref.id());

    auto test_element = sd.create_element<SingleElement>();
    EXPECT_FALSE(old_ref.valid());
    EXPECT_THROW(test_element->set_optical_property_set(old_ref),
                 std::invalid_argument);
    EXPECT_NO_THROW(test_element->set_optical_property_set(new_ref));
}

TEST(OpticalProperties, AngularTableInterpolatesExactAtTablePoints)
{
    OpticalPropertySet optics(InteractionType::REFLECTION,
                              "table_exact_points");
    optics.set_errors(OpticalSide::Both, DistributionType::NONE, 0.0, 0.0);

    std::vector<AngularTablePoint> table = { { 0.0, 0.9 },
                                             { 500.0, 0.5 },
                                             { 1000.0, 0.1 } };
    optics.set_reflectivity_table(OpticalSide::Front, table);

    EXPECT_NEAR(
        optics.get_reflectivity(OpticalSide::Front, std::cos(0.0 * 1.0e-3)),
        0.9, 1e-12);
    EXPECT_NEAR(
        optics.get_reflectivity(OpticalSide::Front, std::cos(500.0 * 1.0e-3)),
        0.5, 1e-12);
    EXPECT_NEAR(
        optics.get_reflectivity(OpticalSide::Front, std::cos(1000.0 * 1.0e-3)),
        0.1, 1e-12);
}

TEST(OpticalProperties, AngularTableInterpolation)
{
    OpticalPropertySet optics(InteractionType::REFLECTION,
                              "table_cosine_interp");
    optics.set_errors(OpticalSide::Both, DistributionType::NONE, 0.0, 0.0);

    constexpr double               angle0      = 0.0;
    constexpr double               angle1      = 1500.0;
    constexpr double               query_angle = 750.0;
    std::vector<AngularTablePoint> table = { { angle0, 1.0 }, { angle1, 0.0 } };
    optics.set_reflectivity_table(OpticalSide::Front, table);

    const double c0 = std::cos(angle0 * 1.0e-3);
    const double c1 = std::cos(angle1 * 1.0e-3);
    const double cq = std::cos(query_angle * 1.0e-3);
    const double expected_cosine_interp =
        1.0 + (cq - c0) / (c1 - c0) * (0.0 - 1.0);
    const double naive_angle_interp =
        0.5; // midpoint if interpolating linearly in angle instead

    const double actual = optics.get_reflectivity(OpticalSide::Front, cq);

    EXPECT_NEAR(actual, expected_cosine_interp, 1e-12);
    EXPECT_GT(std::fabs(actual - naive_angle_interp), 0.1);
}

TEST(OpticalProperties,
     AngularTableInterpolationSatisfiesSecondDerivativeErrorBound)
{
    // g(u) stands in for whatever function is tabulated against u = cos(theta),
    // since that's the variable the table is actually interpolated linearly in.
    // The standard normal pdf is used only because it is smooth with a simple,
    // exact second derivative -- this isn't meant to model Fresnel physics,
    // just to check the cosine-space interpolation formula against the classic
    // linear-interpolation error bound |g(u) - p1(u)| <= (h^2 / 8) * max|g''|.
    auto g = [](double u)
    { return std::exp(-0.5 * u * u) / std::sqrt(2.0 * PI); };

    // g''(u) = (u^2 - 1) * g(u); its only critical points are u = 0 and
    // u = +-sqrt(3), and |g''(0)| = g(0) is the largest of the three, so it
    // bounds |g''(u)| for every u -- no per-interval sampling needed.
    const double max_abs_g2 = g(0.0);

    constexpr double          kMradToRad  = 1.0e-3;
    const std::vector<double> angles_mrad = { 0.0,   200.0,  400.0,  600.0,
                                              800.0, 1000.0, 1200.0, 1400.0 };

    std::vector<AngularTablePoint> table;
    for (double angle : angles_mrad)
    {
        table.push_back(
            AngularTablePoint { angle, g(std::cos(angle * kMradToRad)) });
    }

    OpticalPropertySet optics(InteractionType::REFLECTION, "table_error_bound");
    optics.set_errors(OpticalSide::Both, DistributionType::NONE, 0.0, 0.0);
    optics.set_reflectivity_table(OpticalSide::Front, table);

    constexpr int kQueriesPerInterval = 25;

    for (std::size_t k = 1; k < angles_mrad.size(); ++k)
    {
        const double theta0 = angles_mrad[k - 1] * kMradToRad;
        const double theta1 = angles_mrad[k] * kMradToRad;
        const double h = std::cos(theta0) -
                         std::cos(theta1); // u decreases as theta increases

        const double error_bound = (h * h / 8.0) * max_abs_g2;

        for (int i = 1; i < kQueriesPerInterval; ++i)
        {
            const double t = static_cast<double>(i) / kQueriesPerInterval;
            const double query_theta      = theta0 + t * (theta1 - theta0);
            const double query_angle_mrad = query_theta / kMradToRad;

            const double actual = optics.get_reflectivity(
                OpticalSide::Front, std::cos(query_theta));
            const double exact = g(std::cos(query_theta));

            EXPECT_LE(std::fabs(actual - exact), error_bound)
                << "interval [" << angles_mrad[k - 1] << ", " << angles_mrad[k]
                << "] mrad, query angle " << query_angle_mrad << " mrad";
        }
    }
}

TEST(OpticalProperties, AngularTableClampsOutsideRange)
{
    OpticalPropertySet optics(InteractionType::REFRACTION, "table_clamp");
    optics.set_errors(OpticalSide::Both, DistributionType::NONE, 0.0, 0.0);


    std::vector<AngularTablePoint> table = { { 100.0, 0.8 }, { 900.0, 0.2 } };
    optics.set_transmissivity_table(OpticalSide::Front, table);

    EXPECT_DOUBLE_EQ(
        optics.get_transmissivity(OpticalSide::Front, std::cos(0.0 * 1.0e-3)),
        0.8);
    EXPECT_DOUBLE_EQ(optics.get_transmissivity(OpticalSide::Front,
                                               std::cos(2000.0 * 1.0e-3)),
                     0.2);
}

TEST(OpticalProperties, SinglePointAngularTableReturnsConstantValue)
{
    OpticalPropertySet optics(InteractionType::REFLECTION,
                              "table_single_point");
    optics.set_errors(OpticalSide::Both, DistributionType::NONE, 0.0, 0.0);

    std::vector<AngularTablePoint> table = { { 300.0, 0.42 } };
    optics.set_reflectivity_table(OpticalSide::Front, table);

    EXPECT_DOUBLE_EQ(
        optics.get_reflectivity(OpticalSide::Front, std::cos(0.0 * 1.0e-3)),
        0.42);
    EXPECT_DOUBLE_EQ(
        optics.get_reflectivity(OpticalSide::Front, std::cos(300.0 * 1.0e-3)),
        0.42);
    EXPECT_DOUBLE_EQ(
        optics.get_reflectivity(OpticalSide::Front, std::cos(5000.0 * 1.0e-3)),
        0.42);
}

TEST(OpticalProperties, EnablingEmptyAngularTableThrows)
{
    OpticalPropertySet optics(InteractionType::REFLECTION, "table_empty");
    optics.set_errors(OpticalSide::Both, DistributionType::NONE, 0.0, 0.0);

    EXPECT_THROW(optics.enable_reflectivity_table(OpticalSide::Front),
                 std::invalid_argument);
}

TEST(OpticalProperties, AngularTableRejectsNonAscendingAngles)
{
    OpticalPropertySet optics(InteractionType::REFLECTION,
                              "table_non_ascending");
    optics.set_errors(OpticalSide::Both, DistributionType::NONE, 0.0, 0.0);

    std::vector<AngularTablePoint> descending_table = { { 500.0, 0.5 },
                                                        { 100.0, 0.9 } };
    EXPECT_THROW(
        optics.set_reflectivity_table(OpticalSide::Front, descending_table),
        std::invalid_argument);

    std::vector<AngularTablePoint> duplicate_angle_table = { { 100.0, 0.9 },
                                                             { 100.0, 0.5 } };
    EXPECT_THROW(optics.set_reflectivity_table(OpticalSide::Front,
                                               duplicate_angle_table),
                 std::invalid_argument);
}

TEST(OpticalProperties, AngularTableRejectsInvalidValues)
{
    OpticalPropertySet optics(InteractionType::REFLECTION,
                              "table_invalid_values");
    optics.set_errors(OpticalSide::Both, DistributionType::NONE, 0.0, 0.0);

    std::vector<AngularTablePoint> negative_angle_table = { { -1.0, 0.9 } };
    EXPECT_THROW(
        optics.set_reflectivity_table(OpticalSide::Front, negative_angle_table),
        std::invalid_argument);

    std::vector<AngularTablePoint> nan_value_table = {
        { 100.0, std::numeric_limits<double>::quiet_NaN() }
    };
    EXPECT_THROW(
        optics.set_reflectivity_table(OpticalSide::Front, nan_value_table),
        std::invalid_argument);
}

TEST(OpticalProperties, DisablingAngularTableFallsBackToScalarValue)
{
    OpticalPropertySet optics(InteractionType::REFLECTION, "table_disable");
    optics.set_errors(OpticalSide::Both, DistributionType::NONE, 0.0, 0.0);
    optics.set_reflectivity(OpticalSide::Front, 0.75);

    std::vector<AngularTablePoint> table = { { 0.0, 0.9 }, { 1000.0, 0.1 } };
    optics.set_reflectivity_table(OpticalSide::Front, table);
    EXPECT_DOUBLE_EQ(
        optics.get_reflectivity(OpticalSide::Front, std::cos(0.0 * 1.0e-3)),
        0.9);

    optics.disable_reflectivity_table(OpticalSide::Front);
    EXPECT_DOUBLE_EQ(
        optics.get_reflectivity(OpticalSide::Front, std::cos(0.0 * 1.0e-3)),
        0.75);
    EXPECT_DOUBLE_EQ(optics.get_reflectivity(OpticalSide::Front), 0.75);

    // Re-enabling re-uses the previously-set table without needing to supply it
    // again.
    optics.enable_reflectivity_table(OpticalSide::Front);
    EXPECT_DOUBLE_EQ(
        optics.get_reflectivity(OpticalSide::Front, std::cos(0.0 * 1.0e-3)),
        0.9);
}

TEST(OpticalProperties, UsesAngularTableReflectsPerSideAndPerTableFlags)
{
    OpticalPropertySet optics(InteractionType::REFRACTION, "table_flags");
    optics.set_errors(OpticalSide::Both, DistributionType::NONE, 0.0, 0.0);

    EXPECT_FALSE(optics.uses_angular_table(OpticalSide::Front));
    EXPECT_FALSE(optics.uses_angular_table(OpticalSide::Back));

    std::vector<AngularTablePoint> table = { { 0.0, 0.9 }, { 1000.0, 0.1 } };
    optics.set_transmissivity_table(OpticalSide::Front, table);

    EXPECT_TRUE(optics.uses_angular_table(OpticalSide::Front));
    EXPECT_FALSE(optics.uses_angular_table(OpticalSide::Back));

    optics.disable_transmissivity_table(OpticalSide::Front);
    EXPECT_FALSE(optics.uses_angular_table(OpticalSide::Front));
}

TEST(OpticalProperties, AngularTableRoundTripsThroughJson)
{
    OpticalPropertySet optics(InteractionType::REFRACTION,
                              "table_json_roundtrip");
    optics.set_ideal_transmission(1.0, 1.5);

    std::vector<AngularTablePoint> table = { { 0.0, 1.0 },
                                             { 500.0, 0.8 },
                                             { 1000.0, 0.2 } };
    optics.set_transmissivity_table(OpticalSide::Front, table);

    nlohmann::ordered_json jnode;
    optics.write_json(jnode);

    OpticalPropertySet roundtripped(jnode);

    EXPECT_EQ(optics, roundtripped);
    EXPECT_DOUBLE_EQ(roundtripped.get_transmissivity(OpticalSide::Front,
                                                     std::cos(500.0 * 1.0e-3)),
                     0.8);
}

TEST(OpticalProperties, MissingTableKeysInJsonDefaultToDisabled)
{
    nlohmann::ordered_json jnode;
    jnode["my_type"] =
        SolTrace::Data::InteractionTypeMap.at(InteractionType::REFLECTION);
    jnode["my_name"]                = "no_table_keys";
    jnode["refraction_index_front"] = 1.0;
    jnode["refraction_index_back"]  = 1.0;

    nlohmann::ordered_json jside;
    jside["error_distribution_type"] =
        SolTrace::Data::DistributionTypeMap.at(DistributionType::NONE);
    jside["transmissivity"]    = 0.0;
    jside["reflectivity"]      = 0.6;
    jside["slope_error"]       = 0.0;
    jside["specularity_error"] = 0.0;

    jnode["front"] = jside;
    jnode["back"]  = jside;

    OpticalPropertySet optics(jnode);

    EXPECT_FALSE(optics.uses_angular_table(OpticalSide::Front));
    EXPECT_FALSE(optics.uses_angular_table(OpticalSide::Back));
    EXPECT_DOUBLE_EQ(
        optics.get_reflectivity(OpticalSide::Front, std::cos(123.0 * 1.0e-3)),
        0.6);
}
