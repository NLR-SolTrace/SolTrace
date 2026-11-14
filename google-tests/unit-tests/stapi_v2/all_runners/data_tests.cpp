#include <gtest/gtest.h>

#include "across_builds.hpp"

TEST(DataTests, DataSetSimulationParams)
{
    SETUP_TEST_CXT();

    call_stapi_v2_set_simulation_parameters(pcxt);
    
    CLEANUP_TEST_CXT();
}

TEST(DataTests, DataParams)
{
    SETUP_TEST_CXT();

    call_stapi_v2_sim_params(pcxt);
    
    CLEANUP_TEST_CXT();
}

TEST(DataTests, DataRays)
{
    SETUP_TEST_CXT();

    call_stapi_v2_sim_rays(pcxt);
    
    CLEANUP_TEST_CXT();
}

TEST(DataTests, DataPowerTower)
{
    SETUP_TEST_CXT();

    call_stapi_v2_sim_power_tower(pcxt);
    
    CLEANUP_TEST_CXT();
}

TEST(DataTests, DataErrors)
{
    SETUP_TEST_CXT();

    call_stapi_v2_sim_errors(pcxt);
    
    CLEANUP_TEST_CXT();
}

TEST(DataTests, DataLocation)
{
    SETUP_TEST_CXT();

    call_stapi_v2_sim_location(pcxt);
    
    CLEANUP_TEST_CXT();
}

TEST(DataTests, DataTolerance)
{
    SETUP_TEST_CXT();

    call_stapi_v2_sim_tolerance(pcxt);
    
    CLEANUP_TEST_CXT();
}

TEST(DataTests, DataGetSimulationParams)
{
    SETUP_TEST_CXT();

    call_stapi_v2_get_simulation_parameters(pcxt);
    
    CLEANUP_TEST_CXT();
}

TEST(DataTests, DataAddOptics)
{
    SETUP_TEST_CXT();

    call_stapi_v2_add_optics(pcxt);
    
    CLEANUP_TEST_CXT();
}

TEST(DataTests, DataGetOptic)
{
    SETUP_TEST_CXT();

    call_stapi_v2_get_optic(pcxt);
    
    CLEANUP_TEST_CXT();
}

TEST(DataTests, DataRemoveOptics)
{
    SETUP_TEST_CXT();

    call_stapi_v2_remove_optics(pcxt);
    
    CLEANUP_TEST_CXT();
}

TEST(DataTests, DataAddElements)
{
    SETUP_TEST_CXT();

    call_stapi_v2_add_elements(pcxt);

    CLEANUP_TEST_CXT();
}

TEST(DataTests, DataGetElement)
{
    SETUP_TEST_CXT();

    call_stapi_v2_get_element(pcxt);
    
    CLEANUP_TEST_CXT();
}

TEST(DataTests, DataRemoveElements)
{
    SETUP_TEST_CXT();

    call_stapi_v2_remove_elements(pcxt);
    
    CLEANUP_TEST_CXT();
}

TEST(DataTests, DataToggleElement)
{
    SETUP_TEST_CXT();

    call_stapi_v2_toggle_element(pcxt);
    
    CLEANUP_TEST_CXT();
}

TEST(DataTests, DataElementXYZ)
{
    SETUP_TEST_CXT();

    call_stapi_v2_element_xyz(pcxt);
    
    CLEANUP_TEST_CXT();
}

TEST(DataTests, DataElementAim)
{
    SETUP_TEST_CXT();

    call_stapi_v2_element_aim(pcxt);
    
    CLEANUP_TEST_CXT();
}

TEST(DataTests, DataElementZrot)
{
    SETUP_TEST_CXT();

    call_stapi_v2_element_zrot(pcxt);
    
    CLEANUP_TEST_CXT();
}

TEST(DataTests, DataElementAperture)
{
    SETUP_TEST_CXT();

    call_stapi_v2_element_aperture(pcxt);
    
    CLEANUP_TEST_CXT();
}

TEST(DataTests, DataElementSurface)
{
    SETUP_TEST_CXT();

    call_stapi_v2_element_surface(pcxt);
    
    CLEANUP_TEST_CXT();
}

TEST(DataTests, DataElementGeometry)
{
    SETUP_TEST_CXT();

    call_stapi_v2_element_geometry(pcxt);
    
    CLEANUP_TEST_CXT();
}

TEST(DataTests, DataElementOptic)
{
    SETUP_TEST_CXT();

    call_stapi_v2_element_optic(pcxt);
    
    CLEANUP_TEST_CXT();
}

TEST(DataTests, DataElementGroup)
{
    SETUP_TEST_CXT();

    call_stapi_v2_element_group(pcxt);
    
    CLEANUP_TEST_CXT();
}

TEST(DataTests, DataAddSun)
{
    SETUP_TEST_CXT();

    call_stapi_v2_add_sun(pcxt);

    CLEANUP_TEST_CXT();
}

TEST(DataTests, DataGetSun)
{
    SETUP_TEST_CXT();

    call_stapi_v2_get_sun(pcxt);

    CLEANUP_TEST_CXT();
}

TEST(DataTests, DataSunShape)
{
    SETUP_TEST_CXT();

    call_stapi_v2_sun_shape(pcxt);
    
    CLEANUP_TEST_CXT();
}

TEST(DataTests, DataSunXYZ)
{
    SETUP_TEST_CXT();

    call_stapi_v2_sun_xyz(pcxt);
    
    CLEANUP_TEST_CXT();
}

TEST(DataTests, DataSunUserdata)
{
    SETUP_TEST_CXT();

    call_stapi_v2_sun_userdata(pcxt);
    
    CLEANUP_TEST_CXT();
}

TEST(DataTests, DataSunCalculator)
{
    SETUP_TEST_CXT();

    call_stapi_v2_solar_calculator(pcxt);
    
    CLEANUP_TEST_CXT();
}