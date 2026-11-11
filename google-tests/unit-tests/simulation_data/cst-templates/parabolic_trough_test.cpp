#include <gtest/gtest.h>

#include <native_runner.hpp>
#include <native_runner_types.hpp>
#include <optical_properties.hpp>
#include <simulation_data.hpp>
#include <sun.hpp>
#include <utilities.hpp>

#include <cst_templates/arclength.hpp>
#include <cst_templates/parabolic_trough.hpp>

#include "common.hpp"
#include "count_absorbed_native.h"

#include <glm/gtx/io.hpp>

using ParabolicTrough = SolTrace::Data::ParabolicTrough;

using SolTrace::Runner::RunnerStatus;
using SolTrace::NativeRunner::NativeRunner;
using SolTrace::NativeRunner::TRayData;
using SolTrace::NativeRunner::TSystem;

// Error Checking Tests for ParabolicTrough
TEST(ParabolicTrough, ErrorChecking_SetApertureSize)
{
    SimulationData sd;
    auto pt = new_element<ParabolicTrough>(sd);

    // Test negative aperture size
    EXPECT_THROW(pt->set_aperture_size(-1.0, 5.0), std::invalid_argument);
    EXPECT_THROW(pt->set_aperture_size(5.0, -1.0), std::invalid_argument);
    EXPECT_THROW(pt->set_aperture_size(-1.0, -1.0), std::invalid_argument);

    // Test zero aperture size
    EXPECT_THROW(pt->set_aperture_size(0.0, 5.0), std::invalid_argument);
    EXPECT_THROW(pt->set_aperture_size(5.0, 0.0), std::invalid_argument);

    // Test valid aperture sizes
    EXPECT_NO_THROW(pt->set_aperture_size(5.774, 11.96));
    EXPECT_NO_THROW(pt->set_aperture_size(10.5, 15.3));
}

TEST(ParabolicTrough, ErrorChecking_SetAngles)
{
    SimulationData sd;
    auto pt = new_element<ParabolicTrough>(sd);

    // Test invalid azimuth angles
    EXPECT_THROW(pt->set_angles(-181.0, 45.0), std::invalid_argument);
    EXPECT_THROW(pt->set_angles(181.0, 45.0), std::invalid_argument);

    // Test invalid tilt angles
    EXPECT_THROW(pt->set_angles(0.0, -91.0), std::invalid_argument);
    EXPECT_THROW(pt->set_angles(0.0, 91.0), std::invalid_argument);

    // Test valid angles
    EXPECT_NO_THROW(pt->set_angles(-180.0, 0.0));
    EXPECT_NO_THROW(pt->set_angles(180.0, 90.0));
    EXPECT_NO_THROW(pt->set_angles(0.0, 45.0));
}

TEST(ParabolicTrough, ErrorChecking_SetFocalLength)
{
    SimulationData sd;
    auto pt = new_element<ParabolicTrough>(sd);

    // Test negative focal length
    EXPECT_THROW(pt->set_focal_length(-1.71), std::invalid_argument);
    EXPECT_THROW(pt->set_focal_length(-0.1), std::invalid_argument);

    // Test zero focal length
    EXPECT_THROW(pt->set_focal_length(0.0), std::invalid_argument);

    // Test valid focal lengths
    EXPECT_NO_THROW(pt->set_focal_length(1.71));
    EXPECT_NO_THROW(pt->set_focal_length(5.0));
}

TEST(ParabolicTrough, ErrorChecking_SetNumberPanels)
{
    SimulationData sd;
    auto pt = new_element<ParabolicTrough>(sd);

    // Test invalid panel counts
    EXPECT_THROW(pt->set_number_panels(0, 7), std::invalid_argument);
    EXPECT_THROW(pt->set_number_panels(4, 0), std::invalid_argument);
    EXPECT_THROW(pt->set_number_panels(-1, 7), std::invalid_argument);
    EXPECT_THROW(pt->set_number_panels(4, -1), std::invalid_argument);

    // Test odd number of panels in x direction (not allowed)
    EXPECT_THROW(pt->set_number_panels(3, 7), std::invalid_argument);
    EXPECT_THROW(pt->set_number_panels(5, 7), std::invalid_argument);

    // Test valid panel counts
    EXPECT_NO_THROW(pt->set_number_panels(2, 7));
    EXPECT_NO_THROW(pt->set_number_panels(4, 1));
    EXPECT_NO_THROW(pt->set_number_panels(6, 10));
}

TEST(ParabolicTrough, ErrorChecking_SetGaps)
{
    SimulationData sd;
    auto pt = new_element<ParabolicTrough>(sd);

    // Test negative gap values
    EXPECT_THROW(pt->set_gaps(-0.02, 0.01, 0.08), std::invalid_argument);
    EXPECT_THROW(pt->set_gaps(0.02, -0.01, 0.08), std::invalid_argument);
    EXPECT_THROW(pt->set_gaps(0.02, 0.01, -0.08), std::invalid_argument);

    // Test valid gap values (including zero)
    EXPECT_NO_THROW(pt->set_gaps(0.0, 0.0, 0.0));
    EXPECT_NO_THROW(pt->set_gaps(0.02, 0.01, 0.08));
}

TEST(ParabolicTrough, ErrorChecking_SetReceiverDimensions)
{
    SimulationData sd;
    auto pt = new_element<ParabolicTrough>(sd);

    // Test invalid absorber diameter
    EXPECT_THROW(pt->set_receiver_dimensions(0.0, 0.115, 0.003), std::invalid_argument);
    EXPECT_THROW(pt->set_receiver_dimensions(-0.07, 0.115, 0.003), std::invalid_argument);

    // Test invalid envelope diameter
    EXPECT_THROW(pt->set_receiver_dimensions(0.07, 0.0, 0.003), std::invalid_argument);
    EXPECT_THROW(pt->set_receiver_dimensions(0.07, -0.115, 0.003), std::invalid_argument);

    // Test invalid envelope thickness
    EXPECT_THROW(pt->set_receiver_dimensions(0.07, 0.115, -0.003), std::invalid_argument);
    EXPECT_THROW(pt->set_receiver_dimensions(0.07, 0.115, 0.0), std::invalid_argument);

    // Test absorber diameter >= envelope diameter
    EXPECT_THROW(pt->set_receiver_dimensions(0.115, 0.115, 0.003), std::invalid_argument);
    EXPECT_THROW(pt->set_receiver_dimensions(0.12, 0.115, 0.003), std::invalid_argument);

    // Test valid receiver dimensions
    EXPECT_NO_THROW(pt->set_receiver_dimensions(0.07, 0.115, 0.003));
}

TEST(ParabolicTrough, ErrorChecking_CreateGeometryWithoutParameters)
{
    SimulationData sd;
    auto pt = new_element<ParabolicTrough>(sd);

    auto optics = OpticalPropertySet();
    auto optics_ref = sd.add_optical_property_set(optics);
    pt->set_optics(optics_ref, optics_ref, optics_ref, optics_ref);

    // Test create_geometry without setting required parameters
    EXPECT_THROW(pt->create_geometry(), std::invalid_argument);

    // Set aperture size and test again
    pt->set_aperture_size(5.774, 11.96);
    EXPECT_THROW(pt->create_geometry(), std::invalid_argument);

    // Set focal length and test again
    pt->set_focal_length(1.71);
    EXPECT_THROW(pt->create_geometry(), std::invalid_argument);

    // Set number of panels and test again
    pt->set_number_panels(4, 7);
    EXPECT_THROW(pt->create_geometry(), std::invalid_argument);

    // Set receiver dimensions and it should work
    pt->set_receiver_dimensions(0.07, 0.115, 0.003);
    EXPECT_NO_THROW(pt->create_geometry());
}

TEST(ParabolicTrough, Build)
{
    SimulationData sd;
    auto optics = OpticalPropertySet();
    auto optics_ref = sd.add_optical_property_set(optics);
    
    auto pt = new_element<ParabolicTrough>(sd);
    pt->set_optics(optics_ref, optics_ref, optics_ref, optics_ref);
    pt->set_origin(20.0, -20.0, 30.0);
    pt->set_aperture_size(5.774, 11.96);
    pt->set_number_panels(4, 7);
    pt->set_gaps(0.02, 0.01, 0.08);
    pt->set_focal_length(1.71);
    pt->set_receiver_dimensions(0.07, 0.115, 0.003);
    pt->set_angles(0.0, 0.0);
    pt->create_geometry();

    pt = new_element<ParabolicTrough>(sd);
    pt->set_optics(optics_ref, optics_ref, optics_ref, optics_ref);
    pt->set_origin(20.0, -20.0, 30.0);
    pt->set_aperture_size(5.774, 11.96);
    pt->set_number_panels(1, 7);
    pt->set_gaps(0.0, 0.01, 0.0);
    pt->set_focal_length(1.71);
    pt->set_receiver_dimensions(0.07, 0.115, 0.003);
    pt->set_angles(0.0, 0.0);
    pt->create_geometry();

    // TODO: Check that everything ends up in the proper position
}

TEST(ParabolicTrough, Tracing)
{
    constexpr uint_fast64_t NRAYS = 10000;
    constexpr uint_fast64_t N_ABSORBED_THRESH = NRAYS / 10;

    SimulationData my_sim;
    // Set parameters
    SimulationParameters &params = my_sim.get_simulation_parameters();
    params.number_of_rays = NRAYS;
    params.max_number_of_rays = params.number_of_rays * 100;
    params.include_optical_errors = true;
    params.include_sun_shape_errors = true;
    params.seed = 123;

    NativeRunner my_runner;
    my_runner.disable_power_tower();
    my_runner.disable_point_focus();

    SolTrace::Data::OpticalPropertySet mirror_optics(SolTrace::Data::InteractionType::REFLECTION,
        0.0, 0.0, "Mirror");
    mirror_optics.set_ideal_one_sided_reflector(SolTrace::Data::OpticalSide::Front);
    mirror_optics.set_errors(SolTrace::Data::OpticalSide::Front, SolTrace::Data::DistributionType::NONE, 1.5, 0.5);
    mirror_optics.set_errors(SolTrace::Data::OpticalSide::Back, SolTrace::Data::DistributionType::NONE, 1e-5, 1e-5);
    auto mirror_opt_ref = my_sim.add_optical_property_set(mirror_optics);

    auto absorber_optics = SolTrace::Data::OpticalPropertySet(SolTrace::Data::InteractionType::REFLECTION, "Absorber");
    absorber_optics.set_ideal_absorption(SolTrace::Data::OpticalSide::Both);
    absorber_optics.set_errors(SolTrace::Data::OpticalSide::Both, SolTrace::Data::DistributionType::NONE, 1e-5, 1e-5);
    auto abs_opt_ref = my_sim.add_optical_property_set(absorber_optics);

    auto envelop_outer_optics = SolTrace::Data::OpticalPropertySet(SolTrace::Data::InteractionType::REFRACTION, "EnvelopeOuter");
    envelop_outer_optics.set_ideal_transmission(1.46, 1.0);
    envelop_outer_optics.set_errors(SolTrace::Data::OpticalSide::Both, SolTrace::Data::DistributionType::NONE, 1e-4, 1e-4);
    auto env_out_opt_ref = my_sim.add_optical_property_set(envelop_outer_optics);

    auto envelop_inner_optics = SolTrace::Data::OpticalPropertySet(SolTrace::Data::InteractionType::REFRACTION, "EnvelopeInner");
    envelop_inner_optics.set_ideal_transmission(1.0, 1.46);
    envelop_inner_optics.set_errors(SolTrace::Data::OpticalSide::Both, SolTrace::Data::DistributionType::NONE, 1e-4, 1e-4);
    auto env_in_opt_ref = my_sim.add_optical_property_set(envelop_inner_optics);

    auto pt = new_element<ParabolicTrough>(my_sim);
    pt->set_optics(mirror_opt_ref, abs_opt_ref, env_out_opt_ref, env_in_opt_ref);
    pt->set_origin(20.0, -20.0, 30.0);
    pt->set_angles(0.0, 0.0);
    pt->set_aperture_size(6.0, 12.0);
    pt->set_number_panels(4, 7);
    pt->set_focal_length(1.71);
    pt->set_gaps(0.02, 0.01, 0.08);
    pt->set_receiver_dimensions(0.07, 0.115, 0.003);
    pt->create_geometry();
    pt->set_name("Parabolic Trough");

    auto sun = SolTrace::Data::make_ray_source<Sun>();
    sun->set_position(0.0, 0.0, 1000.0);
    // double NaN = std::numeric_limits<double>::quiet_NaN();
    sun->set_shape(SolTrace::Data::SunShape::PILLBOX, 0.0, 1.0, 0.0);
    my_sim.add_ray_source(sun);

    // Assumes that reference and global coordinates are the same
    // glm::dvec3 pt_aim_point;
    // vector_add(1.0, sun->get_position(), -1.0, pt->get_origin_ref(), pt_aim_point);
    // pt->set_aim_vector(pt_aim_point);
    pt->set_aim_vector(sun->get_position());
    pt->set_zrot(0.0);
    pt->compute_coordinate_rotations();
    pt->enable();
    register_element(my_sim, pt);

    // // We can go over all the elements added
    // for (auto iter = my_sim.get_iterator();
    //      !my_sim.is_at_end(iter);
    //      ++iter)
    // {
    //     // iter is a iterator over the storing container which is a map
    //     // so that the iterator gives the key value pair
    //     element_id id = iter->first;
    //     // `element_ptr` is a std::shared_pointer to an Element
    //     element_ptr el = iter->second;
    //     std::cout << "------------\n"
    //               << "Element ID: " << id
    //               << "\nElement name: " << el->get_name()
    //               << "\nIs Stage: " << el->is_stage()
    //               << "\nIs Composite: " << el->is_composite()
    //               << "\nIs Single: " << el->is_single()
    //               // Below are all the same in this case
    //               << "\nOrigin (ref): " << el->get_origin_ref()
    //               << "\nOrigin (stage): " << el->get_origin_stage()
    //               << "\nOrigin (global): " << el->get_origin_global()
    //               << "\nAim (ref): " << el->get_aim_vector_ref()
    //               << "\nAim (stage): " << el->get_aim_vector_stage()
    //               << "\nAim (global): " << el->get_aim_vector_global()
    //               << "\n";
    // }

    RunnerStatus sts = my_runner.initialize();
    EXPECT_EQ(sts, RunnerStatus::SUCCESS);
    // Setup runs but is not complete
    sts = my_runner.setup_simulation(&my_sim);
    EXPECT_EQ(sts, RunnerStatus::SUCCESS);
    // Run simulation runs but returns RunnerStatus::ERROR
    sts = my_runner.run_simulation();
    EXPECT_EQ(sts, RunnerStatus::SUCCESS);

    const TSystem *sys = my_runner.get_system();
    // sys->RayData.Print();
    const TRayData *ray_data = &(sys->RayData);
    size_t n = ray_data->Count();
    uint_fast64_t num_absorbed = count_absorbed_native(ray_data);

    std::cout << "Number Absorbed: " << num_absorbed << std::endl;
    std::cout << "Number Interactions: " << n << std::endl;

    // ray_data->Print();

    EXPECT_TRUE(n >= NRAYS);
    EXPECT_TRUE(num_absorbed > N_ABSORBED_THRESH);
}

TEST(ParabolicTrough, UpdateGeometry)
{
    constexpr uint_fast64_t NRAYS = 10000;
    constexpr uint_fast64_t N_ABSORBED_THRESH = NRAYS / 10;

    const double sun_az = 180.0;
    const double sun_el = 45.0;

    const double TOL = 1e-12;

    SimulationData my_sim;
    // Set parameters
    SimulationParameters &params = my_sim.get_simulation_parameters();
    params.number_of_rays = NRAYS;
    params.max_number_of_rays = params.number_of_rays * 100;
    params.include_optical_errors = true;
    params.include_sun_shape_errors = true;
    params.seed = 123;

    NativeRunner my_runner;
    my_runner.disable_power_tower();
    my_runner.disable_point_focus();

    SolTrace::Data::OpticalPropertySet mirror_optics(SolTrace::Data::InteractionType::REFLECTION,
        0.0, 0.0, "Mirror");
    mirror_optics.set_ideal_one_sided_reflector(SolTrace::Data::OpticalSide::Front);
    mirror_optics.set_errors(SolTrace::Data::OpticalSide::Front, SolTrace::Data::DistributionType::NONE, 1.5, 0.5);
    mirror_optics.set_errors(SolTrace::Data::OpticalSide::Back, SolTrace::Data::DistributionType::NONE, 1e-5, 1e-5);

    auto mirror_opt_ref = my_sim.add_optical_property_set(mirror_optics);

    auto absorber_optics = SolTrace::Data::OpticalPropertySet(SolTrace::Data::InteractionType::REFLECTION, "Absorber");
    absorber_optics.set_ideal_absorption(SolTrace::Data::OpticalSide::Both);
    absorber_optics.set_errors(SolTrace::Data::OpticalSide::Both, SolTrace::Data::DistributionType::NONE, 1e-5, 1e-5);

    auto abs_opt_ref = my_sim.add_optical_property_set(absorber_optics);

    auto envelop_outer_optics = SolTrace::Data::OpticalPropertySet(SolTrace::Data::InteractionType::REFRACTION, "EnvelopeOuter");
    envelop_outer_optics.set_ideal_transmission(1.46, 1.0);
    envelop_outer_optics.set_errors(SolTrace::Data::OpticalSide::Both, SolTrace::Data::DistributionType::NONE, 1e-4, 1e-4);

    auto env_out_opt_ref = my_sim.add_optical_property_set(envelop_outer_optics);

    auto envelop_inner_optics = SolTrace::Data::OpticalPropertySet(SolTrace::Data::InteractionType::REFRACTION, "EnvelopeInner");
    envelop_inner_optics.set_ideal_transmission(1.0, 1.46);
    envelop_inner_optics.set_errors(SolTrace::Data::OpticalSide::Both, SolTrace::Data::DistributionType::NONE, 1e-4, 1e-4);
    
    auto env_in_opt_ref = my_sim.add_optical_property_set(envelop_inner_optics);

    auto pt = new_element<ParabolicTrough>(my_sim);
    pt->set_optics(mirror_opt_ref, abs_opt_ref, env_out_opt_ref, env_in_opt_ref);
    pt->set_origin(10.0, 0.0, 0.0);
    // pt->set_origin(0.0, 0.0, 0.0);
    pt->set_angles(30.0, 10.0);
    // pt->set_angles(30.0, 0.0);
    pt->set_tracking_limits(-90.0, 90.0);
    pt->set_aperture_size(6.0, 12.0);
    pt->set_number_panels(2, 2);
    pt->set_focal_length(1.71);
    pt->set_gaps(0.02, 0.01, 0.08);
    pt->set_receiver_dimensions(0.07, 0.115, 0.003);
    pt->create_geometry();
    pt->set_name("Parabolic Trough");

    pt->enable();
    register_element(my_sim, pt);

    pt->update_geometry(sun_az, sun_el);

    glm::dvec3 sun_pos;
    SolTrace::Data::sun_position_vector_degrees(sun_pos, sun_az, sun_el);
    sun_pos *= 1000.0;
    auto sun = SolTrace::Data::make_ray_source<Sun>();
    sun->set_position(sun_pos);
    sun->set_shape(SolTrace::Data::SunShape::PILLBOX, 0.0, 1.0, 0.0);
    my_sim.add_ray_source(sun);

    std::cout << "Sun Position: " << sun_pos
              << "\nAim Point: " << pt->get_aim_vector_ref()
              << "\nTracking Origin: " << pt->get_tracking_origin()
              << "\nRotation Vector: " << pt->get_rotation_vector()
              << "\nNeutral Normal: " << pt->get_neutral_normal()
              << "\nTracking Angle: " << pt->get_tracking_angle_degrees()
              << "\nZ-Rotation: " << pt->get_zrot()
              << std::endl;

    EXPECT_NEAR(glm::dot(pt->get_tracking_origin(), pt->get_rotation_vector()), 0.0, TOL);
    EXPECT_NEAR(glm::dot(pt->get_tracking_origin(), pt->get_neutral_normal()), 0.0, TOL);
    EXPECT_NEAR(glm::dot(pt->get_rotation_vector(), pt->get_neutral_normal()), 0.0, TOL);

    glm::dvec3 temp, result;
    SolTrace::Data::rotate_vector_degrees(pt->get_rotation_vector(),
                          pt->get_tracking_origin(),
                          pt->get_tracking_angle_degrees(),
                          temp);
    pt->convert_vector_global_to_local(result, temp);
    // std::cout << "Result: " << result << std::endl;
    EXPECT_NEAR(result[0], 1.0, TOL);
    EXPECT_NEAR(result[1], 0.0, TOL);
    EXPECT_NEAR(result[2], 0.0, TOL);

    pt->convert_vector_global_to_local(result, pt->get_rotation_vector());
    // std::cout << "Result: " << result << std::endl;
    EXPECT_NEAR(result[0], 0.0, TOL);
    EXPECT_NEAR(result[1], 1.0, TOL);
    EXPECT_NEAR(result[2], 0.0, TOL);

    SolTrace::Data::rotate_vector_degrees(pt->get_rotation_vector(),
                          pt->get_neutral_normal(),
                          pt->get_tracking_angle_degrees(),
                          temp);
    pt->convert_vector_global_to_local(result, temp);
    // std::cout << "Result: " << result << std::endl;
    EXPECT_NEAR(result[0], 0.0, TOL);
    EXPECT_NEAR(result[1], 0.0, TOL);
    EXPECT_NEAR(result[2], 1.0, TOL);

    // // We can go over all the elements added
    // for (auto iter = my_sim.get_iterator();
    //      !my_sim.is_at_end(iter);
    //      ++iter)
    // {
    //     // iter is a iterator over the storing container which is a map
    //     // so that the iterator gives the key value pair
    //     element_id id = iter->first;
    //     // `element_ptr` is a std::shared_pointer to an Element
    //     element_ptr el = iter->second;
    //     std::cout << "------------\n"
    //               << "Element ID: " << id
    //               << "\nElement name: " << el->get_name()
    //               << "\nIs Stage: " << el->is_stage()
    //               << "\nIs Composite: " << el->is_composite()
    //               << "\nIs Single: " << el->is_single()
    //               // Below are all the same in this case
    //               << "\nOrigin (ref): " << el->get_origin_ref()
    //               << "\nOrigin (stage): " << el->get_origin_stage()
    //               << "\nOrigin (global): " << el->get_origin_global()
    //               << "\nAim (ref): " << el->get_aim_vector_ref()
    //               << "\nAim (stage): " << el->get_aim_vector_stage()
    //               << "\nAim (global): " << el->get_aim_vector_global()
    //               << "\n";
    // }

    RunnerStatus sts = my_runner.initialize();
    EXPECT_EQ(sts, RunnerStatus::SUCCESS);
    // Setup runs but is not complete
    sts = my_runner.setup_simulation(&my_sim);
    EXPECT_EQ(sts, RunnerStatus::SUCCESS);
    // Run simulation runs but returns RunnerStatus::ERROR
    sts = my_runner.run_simulation();
    EXPECT_EQ(sts, RunnerStatus::SUCCESS);

    const TSystem *sys = my_runner.get_system();
    // sys->RayData.Print();
    const TRayData *ray_data = &(sys->RayData);
    size_t n = ray_data->Count();
    uint_fast64_t num_absorbed = count_absorbed_native(ray_data);

    std::cout << "Number Absorbed: " << num_absorbed << std::endl;
    std::cout << "Number Interactions: " << n << std::endl;

    // ray_data->Print();

    EXPECT_TRUE(n >= NRAYS);
    EXPECT_TRUE(num_absorbed > N_ABSORBED_THRESH);
}

TEST(ParabolicTrough, UpdateGeometry_TrackingLimits)
{
    using SolTrace::Data::D2R;

    constexpr uint_fast64_t NRAYS = 10000;
    constexpr uint_fast64_t N_ABSORBED_THRESH = NRAYS / 10;

    const double TOL = 1e-12;
    const double LOWER = -5.0;
    const double UPPER = 10.0;

    SimulationData sd;
    auto pt = new_element<ParabolicTrough>(sd);

    auto optics = OpticalPropertySet();
    auto optics_ref = sd.add_optical_property_set(optics);
    pt->set_optics(optics_ref, optics_ref, optics_ref, optics_ref);

    pt->set_origin(0.0, 0.0, 0.0);
    pt->set_angles(0.0, 0.0);
    pt->set_tracking_limits(LOWER, UPPER);
    pt->set_aperture_size(6.0, 12.0);
    pt->set_number_panels(2, 2);
    pt->set_focal_length(1.71);
    pt->set_gaps(0.02, 0.01, 0.08);
    pt->set_receiver_dimensions(0.07, 0.115, 0.003);
    pt->create_geometry();
    pt->set_name("Parabolic Trough");
    pt->enable();

    double sun_az = 90.0;
    double sun_el = 20.0;
    pt->update_geometry(sun_az, sun_el);
    // std::cout << "Aim Point: " << pt->get_aim_vector_ref()
    //           << "\nTracking Lower: " << pt->get_tracking_limit_lower()
    //           << "\nTracking Upper: " << pt->get_tracking_limit_upper()
    //           << "\nTracking Origin: " << pt->get_tracking_origin()
    //           << "\nRotation Vector: " << pt->get_rotation_vector()
    //           << "\nNeutral Normal: " << pt->get_neutral_normal()
    //           << "\nTracking Angle: " << pt->get_tracking_angle_degrees()
    //           << "\nZ-Rotation: " << pt->get_zrot()
    //           << std::endl;
    EXPECT_NEAR(pt->get_tracking_angle_degrees(), UPPER, TOL);
    glm::dvec3 normal = glm::normalize(pt->get_aim_vector_global());
    EXPECT_NEAR(normal[0], cos(UPPER * D2R), TOL);
    EXPECT_NEAR(normal[1], 0.0, TOL);
    EXPECT_NEAR(normal[2], sin(UPPER * D2R), TOL);
    glm::dvec3 upper = pt->get_tracking_limit_upper();
    for (unsigned k = 0; k < 3; ++k)
    {
        EXPECT_NEAR(normal[k], upper[k], TOL);
    }

    pt->update_geometry(-sun_az, sun_el);
    EXPECT_NEAR(pt->get_tracking_angle_degrees(), LOWER, TOL);
    normal = glm::normalize(pt->get_aim_vector_global());
    EXPECT_NEAR(normal[0], cos(LOWER * D2R), TOL);
    EXPECT_NEAR(normal[1], 0.0, TOL);
    EXPECT_NEAR(normal[2], sin(LOWER * D2R), TOL);
    glm::dvec3 lower = pt->get_tracking_limit_lower();
    for (unsigned k = 0; k < 3; ++k)
    {
        EXPECT_NEAR(normal[k], lower[k], TOL);
    }
}

TEST(ParabolicTrough, ErrorChecking_UpdateGeometry)
{
    SimulationData sd;
    auto pt = new_element<ParabolicTrough>(sd);

    auto optics = OpticalPropertySet();
    auto optics_ref = sd.add_optical_property_set(optics);
    pt->set_optics(optics_ref, optics_ref, optics_ref, optics_ref);

    pt->set_origin(10.0, 0.0, 0.0);
    pt->set_angles(30.0, 10.0);
    pt->set_tracking_limits(-90.0, 90.0);
    pt->set_aperture_size(6.0, 12.0);
    pt->set_number_panels(2, 2);
    pt->set_focal_length(1.71);
    pt->set_gaps(0.02, 0.01, 0.08);
    pt->set_receiver_dimensions(0.07, 0.115, 0.003);
    pt->set_name("Parabolic Trough");

    EXPECT_THROW(pt->update_geometry(0.0, 0.0), std::invalid_argument);

    pt->create_geometry();
    pt->enable();

    EXPECT_THROW(pt->update_geometry(-720.0, 45.0), std::invalid_argument);
    EXPECT_THROW(pt->update_geometry(720.0, 45.0), std::invalid_argument);
    EXPECT_THROW(pt->update_geometry(0.0, 101.0), std::invalid_argument);
    EXPECT_THROW(pt->update_geometry(0.0, -1.0), std::invalid_argument);
}

TEST(ParabolicTrough, ErrorChecking_InvalidTrackingLimits)
{
    SimulationData sd;
    auto pt = new_element<ParabolicTrough>(sd);
    // Lower limit > upper limit
    EXPECT_THROW(pt->set_tracking_limits(90.0, 0.0), std::invalid_argument);
}
