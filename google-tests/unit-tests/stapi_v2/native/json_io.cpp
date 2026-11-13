#include <gtest/gtest.h>

#include "across_builds.hpp"

TEST(JSONIO, ReadInputJSON)
{
    SETUP_TEST_CXT();

    call_stapi_v2_read_input_json(pcxt);

    CLEANUP_TEST_CXT();
}

TEST(JSONIO, ReadInputJSONFile)
{
    SETUP_TEST_CXT();

    call_stapi_v2_read_input_json_file(pcxt);

    CLEANUP_TEST_CXT();
}