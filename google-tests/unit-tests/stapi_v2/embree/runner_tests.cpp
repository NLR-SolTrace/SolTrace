#include <gtest/gtest.h>

#include "across_builds.hpp"

TEST(runner_tests, runners_installed)
{
    SETUP_TEST_CXT();

    uint8_t installed;
    code = st_get_installed_runners(&installed);
    EXPECT_EQ(code, st_return_code::SUCCESS);
    EXPECT_EQ(installed, (1 << st_runner_type_t::EMBREE) + (1 << st_runner_type_t::NATIVE));

    CLEANUP_TEST_CXT();
}

TEST(runner_tests, runners_is_installed)
{
    SETUP_TEST_CXT();

    bool installed;
    code = st_is_runner_installed(st_runner_type_t::NATIVE, &installed);
    EXPECT_EQ(code, st_return_code::SUCCESS);
    EXPECT_EQ(installed, true);
    
    code = st_is_runner_installed(st_runner_type_t::EMBREE, &installed);
    EXPECT_EQ(code, st_return_code::SUCCESS);
    EXPECT_EQ(installed, true);
    
    code = st_is_runner_installed(st_runner_type_t::OPTIX, &installed);
    EXPECT_EQ(code, st_return_code::SUCCESS);
    EXPECT_EQ(installed, false);

    CLEANUP_TEST_CXT();
}

TEST(runner_tests, runner_setup_native)
{
    SETUP_TEST_CXT();

    LOAD_TEST_JSON();
    
    EXPECT_EQ(
        st_sim_setup(pcxt, st_runner_type_t::NATIVE),
        st_return_code::SUCCESS);

    CLEANUP_TEST_CXT();
}

TEST(runner_tests, runner_setup_embree)
{
    SETUP_TEST_CXT();

    LOAD_TEST_JSON();
    
    EXPECT_EQ(
        st_sim_setup(pcxt, st_runner_type_t::EMBREE),
        st_return_code::SUCCESS);

    CLEANUP_TEST_CXT();
}

TEST(runner_tests, runner_setup_optix)
{
    SETUP_TEST_CXT();

    LOAD_TEST_JSON();
    
    EXPECT_EQ(
        st_sim_setup(pcxt, st_runner_type_t::OPTIX),
        st_return_code::WARNING_FELLBACK_FROM_OPTIX);

    CLEANUP_TEST_CXT();
}

TEST(runner_tests, runner_setup_mismatch)
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

TEST(runner_tests, runner_run_native)
{
    SETUP_TEST_CXT();

    LOAD_TEST_JSON();

    call_stapi_v2_sim_run_v2(pcxt, st_runner_type_t::NATIVE);
    
    CLEANUP_TEST_CXT();
}

TEST(runner_tests, runner_run_embree)
{
    SETUP_TEST_CXT();

    LOAD_TEST_JSON();

    call_stapi_v2_sim_run_v2(pcxt, st_runner_type_t::EMBREE);
    
    CLEANUP_TEST_CXT();
}

TEST(runner_tests, runner_not_found)
{
    SETUP_TEST_CXT();

    EXPECT_EQ(st_sim_run_v2(pcxt),
              st_return_code::RUNNER_NOT_FOUND);

    CLEANUP_TEST_CXT();
}

TEST(runner_tests, runner_report)
{
    SETUP_TEST_CXT();

    LOAD_TEST_JSON();

    call_stapi_v2_sim_report(pcxt, st_runner_type_t::NATIVE);
    call_stapi_v2_sim_report(pcxt, st_runner_type_t::EMBREE);

    CLEANUP_TEST_CXT();
}