#include <gtest/gtest.h>

// #include <constants.hpp>
// #include <element.hpp>
#include <native_runner.hpp>
// #include <sun.hpp>
// #include <simulation_data.hpp>
#include <simulation_data_export.hpp>
// #include <single_element.hpp>
// #include <stage_element.hpp>
// #include <glm::dvec3.hpp>

#include <cmath>
#include <iostream>
#include <sstream>

#include "common.hpp"
#include "count_absorbed_native.h"

using SolTrace::Runner::RunnerStatus;
using SolTrace::NativeRunner::NativeRunner;


// TODO: Create a master configuration then test individual features through modifications
SimulationData create_tower_demo_simulation_data(bool create_stages)
{
    SimulationData sd;

    // Sun
    auto sun = make_ray_source<Sun>();
    sun->set_position(0.0, 0.0, 100.0);
    sun->set_shape(SunShape::GAUSSIAN, 1.0, 0.0, 0.0);
    sd.add_ray_source(sun);

    // Absorber -- Flat
    auto absorber = new_element<SingleElement>(sd);
    absorber->set_origin(0.0, 0.0, 10.0);
    absorber->set_aim_vector(0.0, 5.0, 0.0);
    absorber->set_zrot(0.0);
    absorber->compute_coordinate_rotations();
    set_element_shape(sd, absorber, make_aperture<Rectangle>(2.0, 2.0), make_surface<Flat>()); // surface(nullptr)

    SolTrace::Data::OpticalPropertySet abs_optics(SolTrace::Data::InteractionType::REFLECTION, "Absorber");
    abs_optics.set_ideal_absorption(SolTrace::Data::OpticalSide::Both);
    auto abs_ref = sd.add_optical_property_set(abs_optics);
    absorber->set_optical_property_set(abs_ref);

    absorber->set_name("Absorber");

    stage_ptr st0, st1;
    if (create_stages) {
        // Make stage 1 -- second stage -- these can be added to SimulationData
        // in any order but should be numbered in the desired order
        st1 = new_stage(sd, 1);
        // Origin is initialized to zero but set it explicitly
        st1->set_origin(0.0, 0.0, 0.0);
        // Set aim vector so stage and global coordinates are identical
        st1->set_aim_vector(0.0, 0.0, 1.0);
        st1->set_zrot(0.0);
        st1->compute_coordinate_rotations();
        attach_child(st1, absorber);
        // Optional -- to help the user identify things
        st1->set_name("Stage 1--Absorber");

        // Make stage 0 -- this will be the first stage if the runner uses stages
        st0 = new_stage(sd, 0);
        st0->set_origin(0.0, 0.0, 0.0);
        st0->set_aim_vector(0.0, 0.0, 1.0);
        st0->set_zrot(0.0);
        st0->compute_coordinate_rotations();
        st0->set_name("Stage 0--Reflectors");
    }
    else {
        // Add absorber directly
        register_element(sd, absorber);
    }

    glm::dvec3 rvec, svec, avec;
    glm::dvec3 aim, pos;

    SolTrace::Data::OpticalPropertySet mirror_optics_set(SolTrace::Data::InteractionType::REFLECTION, "Mirror");
    mirror_optics_set.set_ideal_reflection(SolTrace::Data::OpticalSide::Both);
    auto mirror_optics_ref = sd.add_optical_property_set(mirror_optics_set);

    for (int i = -1; i < 2; ++i)
    {
        auto el = make_single(sd, make_aperture<Rectangle>(1.0, 1.95), make_surface<Flat>(), mirror_optics_ref);

        pos = {5 * sin(i * PI / 2.0), 5 * cos(i * PI / 2.0), 0.0};
        el->set_origin(pos);
        rvec = absorber->get_origin_global() - pos;
        SolTrace::Data::normalize_inplace(rvec);
        svec = sun->get_position();
        SolTrace::Data::normalize_inplace(svec);
        avec = 0.5 * rvec + 0.5 * svec;

        aim = pos + 100.0 * avec;
        el->set_aim_vector(aim);

        // TODO: Set zrot as in python file?
        el->set_zrot(30.0 * i);
        // // Could also be set in radians
        // el->set_zrot_radians(PI / 3.0 * i);
        el->compute_coordinate_rotations();

        std::stringstream name;
        name << "Reflector " << i;
        el->set_name(name.str());

        if (create_stages)
            attach_child(st0, el);
        else
            register_element(sd, el);
    }

    if (create_stages) {
        // Stages must have all elements present before adding to SimulationData!!
        register_stage(sd, st0);
        register_stage(sd, st1);
    }

    // Set parameters
    SimulationParameters& params = sd.get_simulation_parameters();
    params.number_of_rays = 100000;
    // params.number_of_rays = 10; // Above takes too long
    params.max_number_of_rays = params.number_of_rays * 100;
    params.include_optical_errors = false;
    params.include_sun_shape_errors = false;
    params.seed = 12345;

    return sd;
}


TEST(TowerDemo, NativeRunnerWithStages)
{
    SimulationData sd = create_tower_demo_simulation_data(true);

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

    NativeRunner runner;
    RunnerStatus sts = runner.initialize();
    EXPECT_EQ(sts, RunnerStatus::SUCCESS);
    // Setup runs but is not complete
    sts = runner.setup_simulation(&sd);
    EXPECT_EQ(sts, RunnerStatus::SUCCESS);
    // Run simulation runs but returns RunnerStatus::ERROR
    sts = runner.run_simulation();
    EXPECT_EQ(sts, RunnerStatus::SUCCESS);

    // TODO: Do some post processing tests here

    // const TSystem *sys = runner.get_system();
    // // auto ray_data = sys->AllRayData;
    // sys->AllRayData.Print();
}

TEST(TowerDemo, NativeRunnerWithoutStages)
{
    SimulationData sd = create_tower_demo_simulation_data(false);

    NativeRunner runner;
    RunnerStatus sts = runner.initialize();
    EXPECT_EQ(sts, RunnerStatus::SUCCESS);
    // Setup runs but is not complete
    sts = runner.setup_simulation(&sd);
    EXPECT_EQ(sts, RunnerStatus::SUCCESS);
    // Run simulation runs but returns RunnerStatus::ERROR
    sts = runner.run_simulation();
    EXPECT_EQ(sts, RunnerStatus::SUCCESS);

    // TODO: Do some post processing tests here

    // const TSystem *sys = runner.get_system();
    // // auto ray_data = sys->AllRayData;
    // sys->AllRayData.Print();
}

TEST(TowerDemo, NativeRunnerWithErrors)
{
    SimulationData sd = create_tower_demo_simulation_data(true);

    // Set parameters
    SimulationParameters &params = sd.get_simulation_parameters();
    params.include_optical_errors = true;
    params.include_sun_shape_errors = true;

    NativeRunner runner;
    RunnerStatus sts = runner.initialize();
    EXPECT_EQ(sts, RunnerStatus::SUCCESS);
    // Setup runs but is not complete
    sts = runner.setup_simulation(&sd);
    EXPECT_EQ(sts, RunnerStatus::SUCCESS);
    // Run simulation runs but returns RunnerStatus::ERROR
    sts = runner.run_simulation();
    EXPECT_EQ(sts, RunnerStatus::SUCCESS);

    // TODO: Do some post processing tests here

    // const TSystem *sys = runner.get_system();
    // // auto ray_data = sys->AllRayData;
    // sys->AllRayData.Print();
}

TEST(TowerDemo, NativeRunnerWithPillboxSunErrors)
{
    SimulationData sd = create_tower_demo_simulation_data(true);

    SimulationParameters& params = sd.get_simulation_parameters();
    params.include_optical_errors = true;
    params.include_sun_shape_errors = true;

    // modify the sun to have user defined shape
    auto sun = sd.get_ray_source();
    // Limb darkened sun shape
    sun->set_shape(SunShape::PILLBOX, 0.0, 4.65, 0.0);

    NativeRunner runner;
    RunnerStatus sts = runner.initialize();
    EXPECT_EQ(sts, RunnerStatus::SUCCESS);
    // Setup runs but is not complete
    sts = runner.setup_simulation(&sd);
    EXPECT_EQ(sts, RunnerStatus::SUCCESS);
    // Run simulation runs but returns RunnerStatus::ERROR
    sts = runner.run_simulation();
    EXPECT_EQ(sts, RunnerStatus::SUCCESS);

    // TODO: Do some post processing tests here

    // const TSystem *sys = runner.get_system();
    // // auto ray_data = sys->AllRayData;
    // sys->AllRayData.Print();
}

TEST(TowerDemo, NativeRunnerWithUserDefinedSunErrors)
{
    SimulationData sd = create_tower_demo_simulation_data(true);

    SimulationParameters& params = sd.get_simulation_parameters();
    params.include_optical_errors = true;
    params.include_sun_shape_errors = true;

    // modify the sun to have user defined shape
    auto sun = sd.get_ray_source();
	// Limb darkened sun shape
    std::vector<double> angles =      { 0.0,    0.25,      0.5,    1.0,   1.5,   2.0,  3.0,  4.0,   4.5, 4.65 };
	std::vector<double> intensities = { 1.0, 0.99999, 0.999893, 0.9991, 0.993, 0.981, 0.90, 0.71, 0.563, 0.0 };
	sun->set_shape(SunShape::USER_DEFINED, 0.0, 0.0, 0.0, angles, intensities);

    NativeRunner runner;
    RunnerStatus sts = runner.initialize();
    EXPECT_EQ(sts, RunnerStatus::SUCCESS);
    // Setup runs but is not complete
    sts = runner.setup_simulation(&sd);
    EXPECT_EQ(sts, RunnerStatus::SUCCESS);
    // Run simulation runs but returns RunnerStatus::ERROR
    sts = runner.run_simulation();
    EXPECT_EQ(sts, RunnerStatus::SUCCESS);

    // TODO: Do some post processing tests here

    // const TSystem *sys = runner.get_system();
    // // auto ray_data = sys->AllRayData;
    // sys->AllRayData.Print();
}

TEST(TowerDemo, NativeRunnerWithLimbDarkenedSunErrors)
{
    SimulationData sd = create_tower_demo_simulation_data(true);

    SimulationParameters& params = sd.get_simulation_parameters();
    params.include_optical_errors = true;
    params.include_sun_shape_errors = true;

    // modify the sun to have user defined shape
    auto sun = sd.get_ray_source();
    // Limb darkened sun shape
    sun->set_shape(SunShape::LIMBDARKENED, 0.0, 0.0, 0.0);

    NativeRunner runner;
    RunnerStatus sts = runner.initialize();
    EXPECT_EQ(sts, RunnerStatus::SUCCESS);
    // Setup runs but is not complete
    sts = runner.setup_simulation(&sd);
    EXPECT_EQ(sts, RunnerStatus::SUCCESS);
    // Run simulation runs but returns RunnerStatus::ERROR
    sts = runner.run_simulation();
    EXPECT_EQ(sts, RunnerStatus::SUCCESS);

    // TODO: Do some post processing tests here

    // const TSystem *sys = runner.get_system();
    // // auto ray_data = sys->AllRayData;
    // sys->AllRayData.Print();
}

TEST(TowerDemo, NativeRunnerWithBuieCsrSunErrors)
{
    SimulationData sd = create_tower_demo_simulation_data(true);

    SimulationParameters& params = sd.get_simulation_parameters();
    params.include_optical_errors = true;
    params.include_sun_shape_errors = true;

    // modify the sun to have user defined shape
    auto sun = sd.get_ray_source();
    // Buie CSR sun shape
    sun->set_shape(SunShape::BUIE_CSR, 0.0, 0.0, 0.1);

    NativeRunner runner;
    RunnerStatus sts = runner.initialize();
    EXPECT_EQ(sts, RunnerStatus::SUCCESS);
    // Setup runs but is not complete
    sts = runner.setup_simulation(&sd);
    EXPECT_EQ(sts, RunnerStatus::SUCCESS);
    // Run simulation runs but returns RunnerStatus::ERROR
    sts = runner.run_simulation();
    EXPECT_EQ(sts, RunnerStatus::SUCCESS);

    // TODO: Do some post processing tests here

    // const TSystem *sys = runner.get_system();
    // // auto ray_data = sys->AllRayData;
    // sys->AllRayData.Print();
}
