#include <gtest/gtest.h>

#include "across_builds.hpp"

TEST(DataTests, DataSetSimulationParams)
{
    SETUP_TEST_CXT();

    code = call_stapi_v2_set_simulation_parameters(pcxt);
    EXPECT_EQ(code, st_return_code::SUCCESS);

    CLEANUP_TEST_CXT();
}

TEST(DataTests, DataParams)
{
    SETUP_TEST_CXT();

    code = call_stapi_v2_sim_params(pcxt);
    EXPECT_EQ(code, st_return_code::SUCCESS);

    CLEANUP_TEST_CXT();
}

TEST(DataTests, DataRays)
{
    SETUP_TEST_CXT();

    code = call_stapi_v2_sim_rays(pcxt);
    EXPECT_EQ(code, st_return_code::SUCCESS);

    CLEANUP_TEST_CXT();
}

TEST(DataTests, DataPowerTower)
{
    SETUP_TEST_CXT();

    code = call_stapi_v2_sim_power_tower(pcxt);
    EXPECT_EQ(code, st_return_code::SUCCESS);

    CLEANUP_TEST_CXT();
}

TEST(DataTests, DataErrors)
{
    SETUP_TEST_CXT();

    code = call_stapi_v2_sim_errors(pcxt);
    EXPECT_EQ(code, st_return_code::SUCCESS);

    CLEANUP_TEST_CXT();
}

TEST(DataTests, DataLocation)
{
    SETUP_TEST_CXT();

    code = call_stapi_v2_sim_location(pcxt);
    EXPECT_EQ(code, st_return_code::SUCCESS);

    CLEANUP_TEST_CXT();
}

TEST(DataTests, DataTolerance)
{
    SETUP_TEST_CXT();

    code = call_stapi_v2_sim_tolerance(pcxt);
    EXPECT_EQ(code, st_return_code::SUCCESS);

    CLEANUP_TEST_CXT();
}

TEST(DataTests, DataGetSimulationParams)
{
    SETUP_TEST_CXT();

    code = call_stapi_v2_get_simulation_parameters(pcxt);
    EXPECT_EQ(code, st_return_code::SUCCESS);

    CLEANUP_TEST_CXT();
}

TEST(DataTests, DataAddOptics)
{
    SETUP_TEST_CXT();

    code = call_stapi_v2_add_optics(pcxt);
    EXPECT_EQ(code, 2 * st_return_code::INVALID_ARGUMENTS);

    CLEANUP_TEST_CXT();
}

TEST(DataTests, DataGetOptic)
{
    SETUP_TEST_CXT();

    code = call_stapi_v2_get_optic(pcxt);
    EXPECT_EQ(code, st_return_code::DATA_VALUE_NOT_FOUND);

    CLEANUP_TEST_CXT();
}

TEST(DataTests, DataRemoveOptics)
{
    SETUP_TEST_CXT();

    code = call_stapi_v2_remove_optics(pcxt);
    EXPECT_EQ(code, st_return_code::SUCCESS);

    CLEANUP_TEST_CXT();
}

TEST(DataTests, DataAddElements)
{
    SETUP_TEST_CXT();

    code = call_stapi_v2_add_elements(pcxt);
    EXPECT_EQ(code, st_return_code::DATA_VALUE_NOT_FOUND
                    + 4 * st_return_code::INVALID_ARGUMENTS);

    CLEANUP_TEST_CXT();
}

TEST(DataTests, DataGetElement)
{
    SETUP_TEST_CXT();

    code = call_stapi_v2_get_element(pcxt);
    EXPECT_EQ(code, st_return_code::DATA_VALUE_NOT_FOUND);

    CLEANUP_TEST_CXT();
}

TEST(DataTests, DataRemoveElements)
{
    SETUP_TEST_CXT();

    code = call_stapi_v2_remove_elements(pcxt);
    EXPECT_EQ(code, st_return_code::WARNING_NOT_FOUND);

    CLEANUP_TEST_CXT();
}

TEST(DataTests, DataToggleElement)
{
    SETUP_TEST_CXT();

    code = call_stapi_v2_toggle_element(pcxt);
    EXPECT_EQ(code, 2 * st_return_code::WARNING_NOT_FOUND);

    CLEANUP_TEST_CXT();
}

TEST(DataTests, DataElementXYZ)
{
    SETUP_TEST_CXT();

    code = call_stapi_v2_element_xyz(pcxt);
    EXPECT_EQ(code, st_return_code::WARNING_NOT_FOUND);

    CLEANUP_TEST_CXT();
}

TEST(DataTests, DataElementAim)
{
    SETUP_TEST_CXT();

    code = call_stapi_v2_element_aim(pcxt);
    EXPECT_EQ(code, st_return_code::WARNING_NOT_FOUND);

    CLEANUP_TEST_CXT();
}

TEST(DataTests, DataElementZrot)
{
    SETUP_TEST_CXT();

    code = call_stapi_v2_element_zrot(pcxt);
    EXPECT_EQ(code, st_return_code::WARNING_NOT_FOUND);

    CLEANUP_TEST_CXT();
}

TEST(DataTests, DataElementAperture)
{
    SETUP_TEST_CXT();

    code = call_stapi_v2_element_aperture(pcxt);
    EXPECT_EQ(code, 2 * st_return_code::INVALID_ARGUMENTS);

    CLEANUP_TEST_CXT();
}

TEST(DataTests, DataElementSurface)
{
    SETUP_TEST_CXT();

    code = call_stapi_v2_element_surface(pcxt);
    EXPECT_EQ(code, 2 * st_return_code::INVALID_ARGUMENTS);

    CLEANUP_TEST_CXT();
}

TEST(DataTests, DataElementGeometry)
{
    SETUP_TEST_CXT();

    code = call_stapi_v2_element_geometry(pcxt);
    EXPECT_EQ(code, 4 * st_return_code::INVALID_ARGUMENTS);

    CLEANUP_TEST_CXT();
}

TEST(DataTests, DataElementOptic)
{
    SETUP_TEST_CXT();

    code = call_stapi_v2_element_optic(pcxt);
    EXPECT_EQ(code, st_return_code::DATA_VALUE_NOT_FOUND);

    CLEANUP_TEST_CXT();
}

TEST(DataTests, DataElementGroup)
{
    SETUP_TEST_CXT();

    code = call_stapi_v2_element_group(pcxt);
    EXPECT_EQ(code, 2 * st_return_code::WARNING_GROUP_IGNORED);

    CLEANUP_TEST_CXT();
}

TEST(DataTests, DataAddSun)
{
    SETUP_TEST_CXT();

    code = call_stapi_v2_add_sun(pcxt);
    EXPECT_EQ(code, 3 * st_return_code::WARNING_SUN_SHAPE_IGNORED
                    + st_return_code::EXCEPTION);

    CLEANUP_TEST_CXT();
}

TEST(DataTests, DataGetSun)
{
    SETUP_TEST_CXT();

    code = call_stapi_v2_get_sun(pcxt);
    EXPECT_EQ(code, st_return_code::DATA_VALUE_NOT_FOUND +
                    2 * st_return_code::INVALID_ARGUMENTS);

    CLEANUP_TEST_CXT();
}

TEST(DataTests, DataSunShape)
{
    SETUP_TEST_CXT();

    code = call_stapi_v2_sun_shape(pcxt);
    EXPECT_EQ(code, 3 * st_return_code::WARNING_SUN_SHAPE_IGNORED);

    CLEANUP_TEST_CXT();
}


TEST(DataTests, DataSunXYZ)
{
    SETUP_TEST_CXT();

    code = call_stapi_v2_sun_xyz(pcxt);
    EXPECT_EQ(code, st_return_code::SUCCESS);

    CLEANUP_TEST_CXT();
}

TEST(DataTests, DataSunUserdata)
{
    SETUP_TEST_CXT();

    code = call_stapi_v2_sun_userdata(pcxt);
    EXPECT_EQ(code, st_return_code::EXCEPTION);

    CLEANUP_TEST_CXT();
}

TEST(DataTests, DataSunCalculator)
{
    SETUP_TEST_CXT();

    code = call_stapi_v2_solar_calculator(pcxt);
    EXPECT_EQ(code, st_return_code::INVALID_ARGUMENTS);

    CLEANUP_TEST_CXT();
}