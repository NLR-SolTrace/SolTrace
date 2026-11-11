#include <gtest/gtest.h>

#include <chrono>
#include <future>
#include <set>
#include <thread>

#include <embree_runner.hpp>
#include <simulation_data_export.hpp>
#include <simulation_runner.hpp>
#include <simulation_result_export.hpp>

using SolTrace::Runner::RunnerStatus;
using SolTrace::EmbreeRunner::EmbreeRunner;
using SolTrace::Result::RayEvent;

TEST(EmbreeRunner, StatusAndCancelMultiThread)
{
    std::string sample_path = std::string(PROJECT_DIR) +
                              std::string("/Power-tower-surround_singlefacet.stinput");

    SimulationData sd;
    EXPECT_TRUE(sd.import_from_file(sample_path));
    sd.set_number_of_rays(1000000);
    sd.set_max_rays_traced(1000000 * 1000);

    EmbreeRunner runner;
    runner.set_number_of_threads(2);
    RunnerStatus sts;
    sts = runner.setup_simulation(&sd);
    EXPECT_EQ(sts, RunnerStatus::SUCCESS);

    auto t0 = std::chrono::high_resolution_clock::now();

    auto fsts = std::async(&EmbreeRunner::run_simulation, &runner);

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    sts = runner.status_simulation();
    EXPECT_EQ(sts, RunnerStatus::RUNNING);

    double prog = -1.0;
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    sts = runner.status_simulation(&prog);
    EXPECT_EQ(sts, RunnerStatus::RUNNING);
    EXPECT_LT(prog, 1.0);
    EXPECT_GT(prog, 0.0);

    runner.cancel_simulation();
    fsts.wait();

    auto t1 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> dur = t1 - t0;

    EXPECT_EQ(fsts.get(), RunnerStatus::CANCEL);
    EXPECT_LT(dur.count(), 10000.0);

    std::cout << "Time for run: " << dur.count() << std::endl;
    std::cout << "Progress before cancel: " << prog << std::endl;
}

TEST(EmbreeRunner, CancelMultithread)
{
    std::string sample_path = std::string(PROJECT_DIR) +
                              std::string("/Power-tower-surround_singlefacet.stinput");

    SimulationData sd;
    EXPECT_TRUE(sd.import_from_file(sample_path));
    sd.set_number_of_rays(1000000);
    sd.set_max_rays_traced(1000000 * 1000);

    EmbreeRunner runner;
    runner.set_number_of_threads(4);
    RunnerStatus sts;
    sts = runner.setup_simulation(&sd);
    ASSERT_EQ(sts, RunnerStatus::SUCCESS);

    auto fsts = std::async(&EmbreeRunner::run_simulation, &runner);

    // Give time to start processing
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    sts = runner.status_simulation();
    EXPECT_EQ(sts, RunnerStatus::RUNNING);

    // Shut everything down to make sure it doesn't hang
    auto t0 = std::chrono::high_resolution_clock::now();
    runner.cancel_simulation();
    ASSERT_EQ(fsts.wait_for(std::chrono::seconds(10)), std::future_status::ready);
    auto t1 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> dur = t1 - t0;

    EXPECT_EQ(fsts.get(), RunnerStatus::CANCEL);

    std::cout << "Time to cancel: " << dur.count() << std::endl;
}

TEST(EmbreeRunner, RayIdAssignmentMultiThread)
{
    const uint_fast64_t NRAYS = 50000;
    const unsigned NTHREADS = 12;

    std::string project_path = std::string(PROJECT_DIR);
    std::string sample_path = project_path +
                              std::string("/simple_test_case.stinput");

    // Load simulation data from file
    SimulationData sd;
    ASSERT_TRUE(sd.import_from_file(sample_path));

    sd.set_number_of_rays(NRAYS);
    sd.set_max_rays_traced(NRAYS * 10);

    // Create and run the native runner
    EmbreeRunner runner;
    runner.set_number_of_threads(NTHREADS);
    RunnerStatus sts = runner.initialize();
    ASSERT_EQ(sts, RunnerStatus::SUCCESS);
    sts = runner.setup_simulation(&sd);
    ASSERT_EQ(sts, RunnerStatus::SUCCESS);
    sts = runner.run_simulation();
    ASSERT_EQ(sts, RunnerStatus::SUCCESS);

    double prog = -1.0;
    sts = runner.status_simulation(&prog);
    EXPECT_LE(prog, 1.0);
    EXPECT_GT(prog, 0.0);
    std::cout << "Progress: " << prog << std::endl;

    SimulationResult result;
    sts = runner.report_simulation(&result, 0);
    ASSERT_EQ(sts, RunnerStatus::SUCCESS);

    // std::cout << result << std::endl;

    uint_fast64_t nrec = result.get_number_of_records();

    std::cout << "Number of rays: " << NRAYS
              << "\nNumber of records: " << nrec
              << std::endl;

    if (nrec != NRAYS)
    {
        runner.print_log(std::cout);
    }

    ASSERT_EQ(nrec, NRAYS);

    for (uint_fast64_t k = 0; k < nrec; ++k)
    {
        auto ray_rec = result[k];
        ASSERT_EQ(ray_rec->id, k + 1);
    }
}

TEST(EmbreeRunner, MaxRaysReturnsPartialResultsMultiThread)
{
    // Max generated rays equals the requested hit count and some rays miss,
    // so every thread hits its cap before reaching its share of hits.
    const uint_fast64_t NRAYS = 10000;
    const uint_fast64_t MAX_RAYS = NRAYS;
    const unsigned NTHREADS = 4;

    SimulationData sd;
    SimulationParameters &params = sd.get_simulation_parameters();
    params.include_optical_errors = false;
    params.include_sun_shape_errors = false;
    params.number_of_rays = NRAYS;
    params.max_number_of_rays = MAX_RAYS;
    params.seed = 123;

    auto sun = SolTrace::Data::make_ray_source<Sun>();
    sun->set_position(0.0, 0.0, 100.0);
    sun->set_shape(SolTrace::Data::SunShape::GAUSSIAN, 1.0, -5.0, 0.0);
    sd.add_ray_source(sun);

    // Four circular mirrors leave gaps in the sun plane, so some rays miss
    auto my_st = SolTrace::Data::make_stage(0);
    const int NUM_ELEMENTS = 4;
    double x[NUM_ELEMENTS] = {1.0, 0.0, -1.0, 0.0};
    double y[NUM_ELEMENTS] = {0.0, 1.0, 0.0, -1.0};
    SolTrace::Data::OpticalPropertySet mirror_optics_set(SolTrace::Data::InteractionType::REFLECTION, "MaxRaysOptics");
    mirror_optics_set.set_ideal_reflection(SolTrace::Data::OpticalSide::Both);
    mirror_optics_set.set_errors(SolTrace::Data::OpticalSide::Both, SolTrace::Data::DistributionType::GAUSSIAN, 0.0, 0.0);
    auto optics_ref = sd.add_optical_property_set(mirror_optics_set);
    for (int k = 0; k < NUM_ELEMENTS; ++k)
    {
        element_ptr el = SolTrace::Data::make_element<SingleElement>();
        el->set_aperture(SolTrace::Data::make_aperture<Circle>(2.0));
        el->set_surface(SolTrace::Data::make_surface<Flat>());
        el->set_reference_frame_geometry(glm::dvec3(x[k], y[k], 0.0),
                                         glm::dvec3(-x[k], -y[k], 1.0),
                                         0.0);
        el->set_optical_property_set(optics_ref);
        my_st->add_element(el);
    }
    sd.add_stage(my_st);

    EmbreeRunner runner;
    runner.set_number_of_threads(NTHREADS);
    ASSERT_EQ(runner.initialize(), RunnerStatus::SUCCESS);
    ASSERT_EQ(runner.setup_simulation(&sd), RunnerStatus::SUCCESS);

    ASSERT_EQ(runner.run_simulation(), RunnerStatus::MAXRAYS);

    // Each thread gets a budget of MAX_RAYS / NTHREADS + 1
    const uint_fast64_t per_thread_cap = MAX_RAYS / NTHREADS + 1;
    EXPECT_LE(runner.get_number_rays_launched(), NTHREADS * per_thread_cap);
    EXPECT_GE(runner.get_number_rays_launched(), MAX_RAYS);

    SimulationResult result;
    ASSERT_EQ(runner.report_simulation(&result, 0), RunnerStatus::SUCCESS);
    const uint_fast64_t nrec = result.get_number_of_records();
    EXPECT_GT(nrec, 0u);
    EXPECT_LT(nrec, NRAYS);
    EXPECT_EQ(runner.get_number_rays_traced(), nrec);

    // Partial results from all threads are well-formed with unique ids
    std::set<uint_fast64_t> ids;
    for (uint_fast64_t k = 0; k < nrec; ++k)
    {
        auto rr = result[k];
        ASSERT_GT(rr->get_number_of_interactions(), 0);
        EXPECT_EQ(rr->get_event(0), RayEvent::CREATE);
        ids.insert(rr->id);
    }
    EXPECT_EQ(ids.size(), nrec);
}
