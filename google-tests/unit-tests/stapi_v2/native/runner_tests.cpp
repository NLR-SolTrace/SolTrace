#include <gtest/gtest.h>

#include "across_builds.hpp"

TEST(RunnerTests, RunnersInstalled)
{
    SETUP_TEST_CXT();

    uint8_t installed;
    code = st_get_installed_runners(&installed);
    EXPECT_EQ(code, st_return_code::SUCCESS);
    EXPECT_EQ(installed, 1 << st_runner_type_t::NATIVE);

    CLEANUP_TEST_CXT();
}

TEST(RunnerTests, RunnersIsInstalled)
{
    SETUP_TEST_CXT();

    bool installed;
    code = st_is_runner_installed(st_runner_type_t::NATIVE, &installed);
    EXPECT_EQ(code, st_return_code::SUCCESS);
    EXPECT_EQ(installed, true);
    
    code = st_is_runner_installed(st_runner_type_t::EMBREE, &installed);
    EXPECT_EQ(code, st_return_code::SUCCESS);
    EXPECT_EQ(installed, false);
    
    code = st_is_runner_installed(st_runner_type_t::OPTIX, &installed);
    EXPECT_EQ(code, st_return_code::SUCCESS);
    EXPECT_EQ(installed, false);

    CLEANUP_TEST_CXT();
}

TEST(RunnerTests, RunnersSetupNative)
{
    SETUP_TEST_CXT();

    LOAD_TEST_JSON();
    
    EXPECT_EQ(
        st_sim_setup(pcxt, st_runner_type_t::NATIVE),
        st_return_code::SUCCESS);

    CLEANUP_TEST_CXT();
}

TEST(RunnerTests, RunnersSetupEmbree)
{
    SETUP_TEST_CXT();

    LOAD_TEST_JSON();
    
    EXPECT_EQ(
        st_sim_setup(pcxt, st_runner_type_t::EMBREE),
        st_return_code::WARNING_FELLBACK_FROM_EMBREE);

    CLEANUP_TEST_CXT();
}

TEST(RunnerTests, RunnersSetupOptix)
{
    SETUP_TEST_CXT();

    LOAD_TEST_JSON();
    
    EXPECT_EQ(
        st_sim_setup(pcxt, st_runner_type_t::OPTIX),
        st_return_code::WARNING_FELLBACK_FROM_OPTIX);

    CLEANUP_TEST_CXT();
}

TEST(RunnerTests, RunnersSetupMismatch)
{
    SETUP_TEST_CXT();

    LOAD_TEST_JSON();

    unsigned int *seeds_test = new unsigned int[2] { 608, 303, };

    EXPECT_EQ(
        st_sim_setup(pcxt, st_runner_type_t::NATIVE, 3, seeds_test, 2),
        st_return_code::RUNNER_NUMBER_THREADS_SEEDS_MISMATCH_FAILURE);

    delete[] seeds_test;
    
    CLEANUP_TEST_CXT();
}

TEST(RunnerTests, RunnersRunNative)
{
    SETUP_TEST_CXT();

    LOAD_TEST_JSON();

    call_stapi_v2_sim_run_v2(pcxt, st_runner_type_t::NATIVE);

    CLEANUP_TEST_CXT();
}

TEST(RunnerTests, RunnersNotFound)
{
    SETUP_TEST_CXT();

    EXPECT_EQ(st_sim_run_v2(pcxt),
              st_return_code::RUNNER_NOT_FOUND);

    CLEANUP_TEST_CXT();
}

TEST(RunnerTests, RunnersReport)
{
    SETUP_TEST_CXT();

    LOAD_TEST_JSON();

    call_stapi_v2_sim_report(pcxt, st_runner_type_t::NATIVE);
    
    CLEANUP_TEST_CXT();
}

TEST(RunnerTests, RunMoveRunNative)
{
    SETUP_TEST_CXT();

    test_move_element(pcxt, st_runner_type_t::NATIVE);
    
    CLEANUP_TEST_CXT();
}
