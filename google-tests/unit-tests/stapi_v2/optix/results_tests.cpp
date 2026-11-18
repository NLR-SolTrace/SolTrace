#include <gtest/gtest.h>

#include "across_builds.hpp"


TEST(ResultsTests, ResultsWriteCSV)
{
    SETUP_TEST_CXT();

    LOAD_TEST_JSON();

    call_stapi_v2_write_results_csv(pcxt, st_runner_type_t::NATIVE, "temp_char.csv");
    call_stapi_v2_write_results_csv(pcxt, st_runner_type_t::OPTIX, "temp_char.csv");
    
    CLEANUP_TEST_CXT();
}

TEST(ResultsTests, ResultsWriteGroupJSON)
{
    SETUP_TEST_CXT();

    LOAD_TEST_JSON();

    call_stapi_v2_write_group_results_json(pcxt, st_runner_type_t::NATIVE, "temp_char.csv");
    call_stapi_v2_write_group_results_json(pcxt, st_runner_type_t::OPTIX, "temp_char.csv");
    
    CLEANUP_TEST_CXT();
}

TEST(ResultsTests, ResultsLocations)
{
    SETUP_TEST_CXT();

    LOAD_TEST_JSON();

    call_stapi_v2_locations(pcxt, st_runner_type_t::NATIVE);
    call_stapi_v2_locations(pcxt, st_runner_type_t::OPTIX);
    
    CLEANUP_TEST_CXT();
}

TEST(ResultsTests, ResultsCosines)
{
    SETUP_TEST_CXT();

    LOAD_TEST_JSON();

    call_stapi_v2_cosines(pcxt, st_runner_type_t::NATIVE);
    call_stapi_v2_cosines(pcxt, st_runner_type_t::OPTIX);
    
    CLEANUP_TEST_CXT();
}

TEST(ResultsTests, ResultsElementMap)
{
    SETUP_TEST_CXT();

    LOAD_TEST_JSON();

    call_stapi_v2_elementmap(pcxt, st_runner_type_t::NATIVE);
    call_stapi_v2_elementmap(pcxt, st_runner_type_t::OPTIX);
    
    CLEANUP_TEST_CXT();
}

TEST(ResultsTests, ResultsStageMap)
{
    SETUP_TEST_CXT();

    LOAD_TEST_JSON();

    call_stapi_v2_stagemap(pcxt, st_runner_type_t::NATIVE);
    call_stapi_v2_stagemap(pcxt, st_runner_type_t::OPTIX);
    
    CLEANUP_TEST_CXT();
}

TEST(ResultsTests, ResultsRayNumbers)
{
    SETUP_TEST_CXT();

    LOAD_TEST_JSON();

    call_stapi_v2_raynumbers(pcxt, st_runner_type_t::NATIVE);
    call_stapi_v2_raynumbers(pcxt, st_runner_type_t::OPTIX);
    
    CLEANUP_TEST_CXT();
}

TEST(ResultsTests, ResultsSunStats)
{
    SETUP_TEST_CXT();

    LOAD_TEST_JSON();

    call_stapi_v2_sun_stats(pcxt, st_runner_type_t::NATIVE);
    call_stapi_v2_sun_stats(pcxt, st_runner_type_t::OPTIX);
    
    CLEANUP_TEST_CXT();
}

TEST(ResultsTests, ResultsGetResultsData)
{
    SETUP_TEST_CXT();

    LOAD_TEST_JSON();

    call_stapi_v2_get_results_data(pcxt, st_runner_type_t::NATIVE);
    call_stapi_v2_get_results_data(pcxt, st_runner_type_t::OPTIX);
    
    CLEANUP_TEST_CXT();
}

TEST(ResultsTests, ResultsGetGroupedResultsData)
{
    SETUP_TEST_CXT();

    LOAD_TEST_JSON();

    call_stapi_v2_get_grouped_results_data(pcxt, st_runner_type_t::NATIVE);
    call_stapi_v2_get_grouped_results_data(pcxt, st_runner_type_t::OPTIX);
    
    CLEANUP_TEST_CXT();
}