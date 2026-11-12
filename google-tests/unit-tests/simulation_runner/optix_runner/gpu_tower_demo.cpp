#include <gtest/gtest.h>

#include <composite_element.hpp>
#include <constants.hpp>
#include <element.hpp>
#include <optical_properties.hpp>
#include <optix_runner.hpp>
#include <sun.hpp>
#include <simulation_data.hpp>
#include <simulation_data_export.hpp>
#include <single_element.hpp>
#include <stage_element.hpp>

#include <cmath>
#include <iostream>
#include <sstream>

#include "element_builders.hpp"

using SolTrace::Runner::RunnerStatus;

TEST(GpuTowerDemo, OptixRunnerWithStages)
{
    SimulationData sd;

    // Sun
    auto sun = make_ray_source<Sun>();
    sun->set_position(0.0, 0.0, 100.0);
    sd.add_ray_source(sun);

    // Absorber -- Flat
    auto absorber = new_element<SingleElement>(sd);
    absorber->set_origin(0.0, 0.0, 10.0);
    absorber->set_aim_vector(0.0, 5.0, 0.0);
    absorber->set_surface(make_surface<Flat>());
    absorber->set_aperture(make_aperture<Rectangle>(2.0, 2.0));

    SolTrace::Data::OpticalPropertySet absorber_optics(
        InteractionType::REFLECTION,
        0.0,
        0.0,
        "absorber_optics");
    absorber_optics.set_ideal_absorption(OpticalSide::Both);
    auto absorber_optics_ref = sd.add_optical_property_set(absorber_optics);
    absorber->set_optical_property_set(absorber_optics_ref);
    absorber->set_name("Absorber");

    // // Absorber -- Cylindrical -- MAY NOT WORK DUE TO UNIMPLEMENTED CODE!
    // auto absorber = make_element<SingleElement>();
    // const double r = 5.0;
    // absorber->set_origin(0.0, -r, 0.0);
    // absorber->set_zrot(0.0);
    // absorber->set_aim_vector(0.0, 100.0, -(r + 50.0));
    // // These are the default options but to show off the constructor
    // OpticalProperties op(REFLECTION, 0.0, 0.0, 0.0, 0.0);
    // // Alternative to getting the pointer and setting values
    // absorber->set_front_optical_properties(op);
    // absorber->set_surface(make_surface<Cylinder>());
    // absorber->set_aperture(make_aperture<SingleAxisCurvatureSection>());

    // Make stage 1 -- second stage -- these can be added to SimulationData
    // in any order but should be numbered in the desired order
    auto st1 = new_stage(sd, 1);
    // Origin is initialized to zero but set it explicitly
    st1->set_origin(0.0, 0.0, 0.0);
    // Set aim vector so stage and global coordinates are identical
    st1->set_aim_vector(0.0, 0.0, 1.0);
    attach_child(st1, absorber);
    // Optional -- to help the user identify things
    st1->set_name("Stage 1--Absorber");

    // Make stage 0 -- this will be the first stage if the runner uses stages
    auto st0 = new_stage(sd, 0);
    st0->set_origin(0.0, 0.0, 0.0);
    st0->set_aim_vector(0.0, 0.0, 1.0);
    st0->set_name("Stage 0--Reflectors");

    glm::dvec3 rvec, svec, avec;
    glm::dvec3 aim, pos;

    double spacing = PI / 4.0;

    SolTrace::Data::OpticalPropertySet reflector_optics(
        InteractionType::REFLECTION,
        0.0,
        0.0,
        "reflector_optics");
    reflector_optics.set_ideal_one_sided_reflector();
    auto reflector_optics_ref = sd.add_optical_property_set(reflector_optics);

    for (int i = -1; i < 4; ++i)
    {
        auto el = new_element<SingleElement>(sd);
        el->set_optical_property_set(reflector_optics_ref);

        pos = {5 * sin(i * spacing),
               5 * cos(i * spacing),
               0.0};
        el->set_origin(pos);
        rvec = glm::normalize(absorber->get_origin_global() - pos);
        svec = glm::normalize(sun->get_position());
        avec = 0.5 * rvec + 0.5 * svec;

        aim = pos + 100.0 * avec;
        el->set_aim_vector(aim);

        // TODO: Set zrot as in python file?
        el->set_zrot(30.0 * i);
        // // Could also be set in radians
        // el->set_zrot_radians(PI / 3.0 * i);

        el->set_surface(make_surface<Flat>());
        el->set_aperture(make_aperture<Rectangle>(1.0, 1.95));

        std::stringstream name;
        name << "Reflector " << i;
        el->set_name(name.str());

        attach_child(st0, el);
    }

    // Stages must have all elements present before adding to SimulationData!!
    register_stage(sd, st0);
    register_stage(sd, st1);

    // Set parameters
    SimulationParameters &params = sd.get_simulation_parameters();
    // params.number_of_rays = 1000000;
    params.number_of_rays = 100; // Above takes too long
    params.max_number_of_rays = params.number_of_rays * 100;
    params.include_optical_errors = false;
    params.include_sun_shape_errors = false;
    params.seed = 12345;

    // // We can go over all the elements added
    // for (auto iter = sd.get_iterator();
    //      !sd.is_at_end(iter);
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
    //               << "\nOrigin (local): " << el->get_origin_local()
    //               << "\nOrigin (stage): " << el->get_origin_stage()
    //               << "\nOrigin (global): " << el->get_origin_global()
    //               << "\n";
    //     // NOTE: The above iteration includes StageElements (e.g. stages),
    //     // CompositeElements (which are collections of elements), as well as
    //     // SingleElements (actual physical elements that interact with rays).
    //     // We can distinguish between them by calling the various `is_XXXX`
    //     // methods as seen above. For runners that put everything in global
    //     // coordinates, anything that is not a SingleElement (so `is_single`
    //     // returns true) can be ignored.
    // }

    OptixRunner runner;
    RunnerStatus sts = runner.initialize();
    ASSERT_EQ(sts, RunnerStatus::SUCCESS);
    // Setup runs but is not complete
    sts = runner.setup_simulation(&sd);
    ASSERT_EQ(sts, RunnerStatus::SUCCESS);
    // Run simulation runs but returns RunnerStatus::ERROR
    sts = runner.run_simulation();
    ASSERT_EQ(sts, RunnerStatus::SUCCESS);

    // sts = runner.report_simulation();
    ASSERT_EQ(sts, RunnerStatus::SUCCESS);
}

static void setup_tower_sd(SimulationData &sd, uint_fast64_t nrays)
{
    auto sun = make_ray_source<Sun>();
    sun->set_position(0.0, 0.0, 100.0);
    sd.add_ray_source(sun);

    auto absorber = new_element<SingleElement>(sd);
    absorber->set_origin(0.0, 0.0, 10.0);
    absorber->set_aim_vector(0.0, 5.0, 0.0);
    absorber->set_surface(make_surface<Flat>());
    absorber->set_aperture(make_aperture<Rectangle>(2.0, 2.0));
    SolTrace::Data::OpticalPropertySet absorber_optics(
        InteractionType::REFLECTION,
        0.0,
        0.0,
        "tower_absorber_optics");
    absorber_optics.set_ideal_absorption(OpticalSide::Both);
    auto absorber_optics_ref = sd.add_optical_property_set(absorber_optics);
    absorber->set_optical_property_set(absorber_optics_ref);

    auto st1 = new_stage(sd, 1);
    st1->set_origin(0.0, 0.0, 0.0);
    st1->set_aim_vector(0.0, 0.0, 1.0);
    attach_child(st1, absorber);

    auto st0 = new_stage(sd, 0);
    st0->set_origin(0.0, 0.0, 0.0);
    st0->set_aim_vector(0.0, 0.0, 1.0);

    const double spacing = PI / 4.0;
    SolTrace::Data::OpticalPropertySet reflector_optics(
        InteractionType::REFLECTION,
        0.0,
        0.0,
        "tower_reflector_optics");
    reflector_optics.set_ideal_one_sided_reflector();
    auto reflector_optics_ref = sd.add_optical_property_set(reflector_optics);
    for (int i = -1; i < 4; ++i)
    {
        auto el = new_element<SingleElement>(sd);
        el->set_optical_property_set(reflector_optics_ref);

        glm::dvec3 pos = {5 * sin(i * spacing), 5 * cos(i * spacing), 0.0};
        el->set_origin(pos);
        glm::dvec3 rvec = glm::normalize(absorber->get_origin_global() - pos);
        glm::dvec3 svec = glm::normalize(sun->get_position());
        glm::dvec3 avec = 0.5 * rvec + 0.5 * svec;
        el->set_aim_vector(pos + 100.0 * avec);
        el->set_zrot(30.0 * i);
        el->set_surface(make_surface<Flat>());
        el->set_aperture(make_aperture<Rectangle>(1.0, 1.95));
        attach_child(st0, el);
    }

    register_stage(sd, st0);
    register_stage(sd, st1);

    SimulationParameters &params = sd.get_simulation_parameters();
    params.number_of_rays = nrays;
    params.max_number_of_rays = nrays * 100;
    params.include_optical_errors = false;
    params.include_sun_shape_errors = false;
    params.seed = 12345;
}

TEST(OptixRunner, RaysLaunchedEqualsRequestedAfterRun)
{
    const uint_fast64_t NRAYS = 100;
    SimulationData sd;
    setup_tower_sd(sd, NRAYS);

    OptixRunner runner;
    RunnerStatus sts = runner.initialize();
    ASSERT_EQ(sts, RunnerStatus::SUCCESS);
    sts = runner.setup_simulation(&sd);
    ASSERT_EQ(sts, RunnerStatus::SUCCESS);

    // Before running, no rays have been launched yet
    EXPECT_EQ(runner.get_number_rays_launched(), static_cast<uint_fast64_t>(0));

    sts = runner.run_simulation();
    ASSERT_EQ(sts, RunnerStatus::SUCCESS);

    EXPECT_GE(runner.get_number_rays_launched(), NRAYS);

    const uint_fast64_t rays_traced = runner.get_number_rays_traced();
    EXPECT_GT(rays_traced, static_cast<uint_fast64_t>(0));
    EXPECT_LE(rays_traced, runner.get_number_rays_launched());
    EXPECT_EQ(rays_traced, NRAYS);
}
