#include <gtest/gtest.h>

#include "across_builds.hpp"

TEST(json_io, read_input_json)
{
    SETUP_TEST_CXT();

    call_stapi_v2_read_input_json(pcxt);
    
    CLEANUP_TEST_CXT();
}

TEST(json_io, read_input_json_file)
{
    SETUP_TEST_CXT();

    call_stapi_v2_read_input_json_file(pcxt);
    
    CLEANUP_TEST_CXT();
}