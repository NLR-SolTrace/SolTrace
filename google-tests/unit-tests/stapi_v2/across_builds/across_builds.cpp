/*
functions that are across different api builds (only native, with only embree, with only optix, with both embree and optix).
i.e. shouldn't need google-test.h in here.
intended to be called from tests inside different api build tests, those manage the context that these functions use.
*/

#include <gtest/gtest.h>
#include <fstream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include "across_builds.hpp"

///////////////////////
// Utility Functions //
///////////////////////

std::string load_json()
{
    namespace fs = std::filesystem;

    // Build paths
    const fs::path project_root(PROJECT_DIR);
    const fs::path sample_path = project_root / "field_test.json";

    std::ifstream ifs(sample_path);
    if (!ifs.is_open()) throw std::runtime_error("Failure opening json");

    // Load json from file stream
    std::ostringstream buffer;
    buffer << ifs.rdbuf();
    std::string content = buffer.str();
    return content;
}

void ASSERT_OPTICAL_SIDE_EQ(args_optical_properties_face expected, args_optical_properties_face actual)
{
    ASSERT_EQ(expected.error_distribution_type, actual.error_distribution_type);
    ASSERT_EQ(expected.transmissivity,          actual.transmissivity);
    ASSERT_EQ(expected.reflectivity,            actual.reflectivity);
    ASSERT_EQ(expected.slope_error,             actual.slope_error);
    ASSERT_EQ(expected.specularity_error,       actual.specularity_error);
}

////////////////////////////////
// Simlulation Data Functions //
////////////////////////////////

// functions for simulation data management thru json strings
void call_stapi_v2_read_input_json(st_context_v2_t pcxt)
{
    std::string root = load_json();
    // add calls to other overloads
    ASSERT_EQ(
        st_read_input_json(pcxt, root.c_str()), 
        st_return_code::SUCCESS);
}

void call_stapi_v2_read_input_json_file(st_context_v2_t pcxt)
{
    namespace fs = std::filesystem;

    // Build paths
    const fs::path project_root(PROJECT_DIR);
    const fs::path sample_path = project_root / "sample_ver_20251112.json";
    ASSERT_EQ(
        st_read_input_json_file(pcxt, sample_path.string().c_str()),
        st_return_code::SUCCESS);
}

// functions for simulation data management directly
void call_stapi_v2_set_simulation_parameters(st_context_v2_t pcxt)
{
    args_simulation_parameters _params = { 1, 100, 608, .1, 35.962278, -106.5122622, true, true, false };

    ASSERT_EQ(
        st_set_simulation_parameters(pcxt, &_params),
        st_return_code::SUCCESS);

    args_simulation_parameters rt_params;
    ASSERT_EQ(
        st_get_simulation_parameters(pcxt, &rt_params),
        st_return_code::SUCCESS);

    ASSERT_EQ(rt_params.number_of_rays,           _params.number_of_rays);
    ASSERT_EQ(rt_params.max_number_of_rays,       _params.max_number_of_rays);
    ASSERT_EQ(rt_params.seed,                     _params.seed);
    ASSERT_EQ(rt_params.tolerance,                _params.tolerance);
    ASSERT_EQ(rt_params.latitude,                 _params.latitude);
    ASSERT_EQ(rt_params.longitude,                _params.longitude);
    ASSERT_EQ(rt_params.include_sun_shape_errors, _params.include_sun_shape_errors);
    ASSERT_EQ(rt_params.include_optical_errors,   _params.include_optical_errors);
    ASSERT_EQ(rt_params.as_power_tower,           _params.as_power_tower);
}

void call_stapi_v2_sim_params(st_context_v2_t pcxt)
{
    ASSERT_EQ(
        st_sim_params(pcxt, 1, 100, true), 
        st_return_code::SUCCESS);

    args_simulation_parameters rt_params;
    ASSERT_EQ(
        st_get_simulation_parameters(pcxt, &rt_params),
        st_return_code::SUCCESS);

    ASSERT_EQ(rt_params.number_of_rays, 1);
    ASSERT_EQ(rt_params.max_number_of_rays, 100);
    ASSERT_EQ(rt_params.as_power_tower, true);
}

void call_stapi_v2_sim_rays(st_context_v2_t pcxt)
{
    ASSERT_EQ(
        st_sim_rays(pcxt, 1, 100),
        st_return_code::SUCCESS);

    args_simulation_parameters rt_params;
    ASSERT_EQ(
        st_get_simulation_parameters(pcxt, &rt_params),
        st_return_code::SUCCESS);


    ASSERT_EQ(rt_params.number_of_rays, 1);
    ASSERT_EQ(rt_params.max_number_of_rays, 100);
}

void call_stapi_v2_sim_power_tower(st_context_v2_t pcxt)
{
    ASSERT_EQ(
        st_sim_power_tower(pcxt, true),
        st_return_code::SUCCESS);

    args_simulation_parameters rt_params;
    ASSERT_EQ(
        st_get_simulation_parameters(pcxt, &rt_params),
        st_return_code::SUCCESS);

    ASSERT_EQ(rt_params.as_power_tower, true);
}

void call_stapi_v2_sim_errors(st_context_v2_t pcxt)
{
    ASSERT_EQ(
        st_sim_errors(pcxt, true, true),
        st_return_code::SUCCESS);

    args_simulation_parameters rt_params;
    ASSERT_EQ(
        st_get_simulation_parameters(pcxt, &rt_params),
        st_return_code::SUCCESS);

    ASSERT_EQ(rt_params.include_sun_shape_errors, true);
    ASSERT_EQ(rt_params.include_optical_errors, true);
}

void call_stapi_v2_sim_location(st_context_v2_t pcxt)
{
    ASSERT_EQ(
        st_sim_location(pcxt, 35.962278, -106.5122622),
        st_return_code::SUCCESS);

    args_simulation_parameters rt_params;
    ASSERT_EQ(
        st_get_simulation_parameters(pcxt, &rt_params),
        st_return_code::SUCCESS);
    
    ASSERT_EQ(rt_params.latitude, 35.962278);
    ASSERT_EQ(rt_params.longitude, -106.5122622);
}

void call_stapi_v2_sim_tolerance(st_context_v2_t pcxt)
{
    ASSERT_EQ(
        st_sim_tolerance(pcxt, .1),
        st_return_code::SUCCESS);
    
    args_simulation_parameters rt_params;
    ASSERT_EQ(
        st_get_simulation_parameters(pcxt, &rt_params),
        st_return_code::SUCCESS);
    
    ASSERT_EQ(rt_params.tolerance, .1);
}

void call_stapi_v2_sim_seed(st_context_v2_t pcxt)
{
    ASSERT_EQ(
        st_sim_seed(pcxt, 12345),
        st_return_code::SUCCESS);
    
    args_simulation_parameters rt_params;
    ASSERT_EQ(
        st_get_simulation_parameters(pcxt, &rt_params),
        st_return_code::SUCCESS);
    
    ASSERT_EQ(rt_params.seed, 12345);
}

void call_stapi_v2_get_simulation_parameters(st_context_v2_t pcxt)
{
    args_simulation_parameters _params = { 1, 100, 608, .1, 35.962278, -106.5122622, true, true, false };

    ASSERT_EQ(
        st_set_simulation_parameters(pcxt, &_params),
        st_return_code::SUCCESS);
    
    args_simulation_parameters rt_params;
    ASSERT_EQ(
        st_get_simulation_parameters(pcxt, &rt_params),
        st_return_code::SUCCESS);
    
    ASSERT_EQ(rt_params.number_of_rays,           _params.number_of_rays);
    ASSERT_EQ(rt_params.max_number_of_rays,       _params.max_number_of_rays);
    ASSERT_EQ(rt_params.seed,                     _params.seed);
    ASSERT_EQ(rt_params.tolerance,                _params.tolerance);
    ASSERT_EQ(rt_params.latitude,                 _params.latitude);
    ASSERT_EQ(rt_params.longitude,                _params.longitude);
    ASSERT_EQ(rt_params.include_sun_shape_errors, _params.include_sun_shape_errors);
    ASSERT_EQ(rt_params.include_optical_errors,   _params.include_optical_errors);
    ASSERT_EQ(rt_params.as_power_tower,           _params.as_power_tower);
}

// functions to add/remove/set optical properties
void call_stapi_v2_add_optics(st_context_v2_t pcxt)
{
    args_optical_properties_set set = {"test1", 1.1, 1.1, 1};
    args_optical_properties_face f = {.5, .5, .5, .5, 'g'};
    args_optical_properties_face b = {.25, .25, .25, .25, 'g'};

    // check no optics set
    uint_fast64_t num = -1;
    uint_fast64_t id = -1;
    ASSERT_EQ(
        st_num_optics(pcxt, &num),
        st_return_code::SUCCESS);
    ASSERT_EQ(num, 0);

    // test add refractive set, expect st_return_code::SUCCESS
    ASSERT_EQ(
        st_add_optical_properties_set(pcxt, &set, &f, &b, &id),
        st_return_code::SUCCESS);
    ASSERT_EQ(id, 0);
    ASSERT_EQ(
        st_num_optics(pcxt, &num),
        st_return_code::SUCCESS);
    ASSERT_EQ(num, 1);

    // check values were set
    args_optical_properties_set rt_set;
    args_optical_properties_face rt_f, rt_b;

    ASSERT_EQ(
        st_get_optical_properties_set(pcxt, id, &rt_set, &rt_f, &rt_b),
        st_return_code::SUCCESS);

    ASSERT_EQ(std::string(rt_set.name), std::string(set.name));
    ASSERT_EQ(rt_set.type, set.type);
    ASSERT_OPTICAL_SIDE_EQ(f, rt_f);
    ASSERT_OPTICAL_SIDE_EQ(b, rt_b);

    // test add reflective set
    set.type = 2;
    set.name = "test2";
    // test add reflective set, expect st_return_code::SUCCESS
    ASSERT_EQ(
        st_add_optical_properties_set(pcxt, &set, &f, &b, &id),
        st_return_code::SUCCESS);
    ASSERT_EQ(id, 1);
    ASSERT_EQ(
        st_num_optics(pcxt, &num),
        st_return_code::SUCCESS);
    ASSERT_EQ(num, 2);

    // check values were set
    ASSERT_EQ(
        st_get_optical_properties_set(pcxt, id, &rt_set, &rt_f, &rt_b),
        st_return_code::SUCCESS);

    ASSERT_EQ(std::string(rt_set.name), std::string(set.name));
    ASSERT_EQ(rt_set.type, set.type);
    ASSERT_OPTICAL_SIDE_EQ(f, rt_f);
    ASSERT_OPTICAL_SIDE_EQ(b, rt_b);

    // set bad distribution characters for front
    f.error_distribution_type = 'z';
    // expect st_return_code::INVALID_ARGUMENTS
    ASSERT_EQ(
        st_add_optical_properties_set(pcxt, &set, &f, &b, &id),
        st_return_code::INVALID_ARGUMENTS);
    ASSERT_EQ(id, 1);
    ASSERT_EQ(
        st_num_optics(pcxt, &num),
        st_return_code::SUCCESS);
    ASSERT_EQ(num, 2);

    // set bad distribution characters for back
    f.error_distribution_type = 'g';
    b.error_distribution_type = 'z';
    // expect st_return_code::INVALID_ARGUMENTS
    ASSERT_EQ(
        st_add_optical_properties_set(pcxt, &set, &f, &b, &id),
        st_return_code::INVALID_ARGUMENTS);
    ASSERT_EQ(id, 1);
    ASSERT_EQ(
        st_num_optics(pcxt, &num),
        st_return_code::SUCCESS);
    ASSERT_EQ(num, 2);
}

void call_stapi_v2_get_optic(st_context_v2_t pcxt)
{
    args_optical_properties_set set = {"test1", 1.1, 1.1, 2};
    args_optical_properties_face f = {.5, .5, .5, .5, 'g'};
    args_optical_properties_face b = {.25, .25, .25, .25, 'g'};
    uint_fast64_t id = -1;

    ASSERT_EQ(
        st_add_optical_properties_set(pcxt, &set, &f, &b, &id),
        st_return_code::SUCCESS);

    args_optical_properties_set rt_set;
    args_optical_properties_face rt_f;
    args_optical_properties_face rt_b;

    ASSERT_EQ(
        st_get_optical_properties_set(pcxt, id, &rt_set, &rt_f, &rt_b),
        st_return_code::SUCCESS);

    // check set struct
    ASSERT_EQ(std::string(rt_set.name), std::string(set.name));
    ASSERT_EQ(rt_set.refraction_index_front, set.refraction_index_front);
    ASSERT_EQ(rt_set.refraction_index_back, set.refraction_index_back);
    ASSERT_EQ(rt_set.type, set.type);

    // check front struct
    ASSERT_EQ(rt_f.error_distribution_type, f.error_distribution_type);
    ASSERT_EQ(rt_f.reflectivity, f.reflectivity);
    ASSERT_EQ(rt_f.transmissivity, f.transmissivity);
    ASSERT_EQ(rt_f.slope_error, f.slope_error);
    ASSERT_EQ(rt_f.specularity_error, f.specularity_error);
    
    // check back struct
    ASSERT_EQ(rt_b.error_distribution_type, b.error_distribution_type);
    ASSERT_EQ(rt_b.reflectivity, b.reflectivity);
    ASSERT_EQ(rt_b.transmissivity, b.transmissivity);
    ASSERT_EQ(rt_b.slope_error, b.slope_error);
    ASSERT_EQ(rt_b.specularity_error, b.specularity_error);

    // expect st_return_code::DATA_VALUE_NOT_FOUND
    ASSERT_EQ(
        st_get_optical_properties_set(pcxt, id + 1, &rt_set, &rt_f, &rt_b),
        st_return_code::DATA_VALUE_NOT_FOUND);
}

void call_stapi_v2_remove_optics(st_context_v2_t pcxt)
{
    args_optical_properties_set set = {"test1", 1.1, 1.1, 0};
    args_optical_properties_face f = {.5, .5, .5, .5, 'g'};
    args_optical_properties_face b = {.25, .25, .25, .25, 'g'};

    // check no optics set
    uint_fast64_t num = -1;
    uint_fast64_t id = -1;
    ASSERT_EQ(
        st_num_optics(pcxt, &num),
        st_return_code::SUCCESS);
    ASSERT_EQ(num, 0);

    ASSERT_EQ(
        st_add_optical_properties_set(pcxt, &set, &f, &b, &id),
        st_return_code::SUCCESS);
    ASSERT_EQ(id, 0);
    ASSERT_EQ(
        st_num_optics(pcxt, &num),
        st_return_code::SUCCESS);
    ASSERT_EQ(num, 1);

    // expect st_return_code::SUCCESS
    ASSERT_EQ(
        st_clear_optics(pcxt),
        st_return_code::SUCCESS);
    ASSERT_EQ(
        st_num_optics(pcxt, &num),
        st_return_code::SUCCESS);
    ASSERT_EQ(num, 0);

    // expect st_return_code::SUCCESS
    ASSERT_EQ(
        st_add_optical_properties_set(pcxt, &set, &f, &b, &id),
        st_return_code::SUCCESS);
    ASSERT_EQ(id, 1);
    ASSERT_EQ(
        st_num_optics(pcxt, &num),
        st_return_code::SUCCESS);
    ASSERT_EQ(num, 1);

    // expect st_return_code::SUCCESS
    ASSERT_EQ(
        st_delete_optic(pcxt, 1),
        st_return_code::SUCCESS);
    ASSERT_EQ(
        st_num_optics(pcxt, &num),
        st_return_code::SUCCESS);
    ASSERT_EQ(num, 0);
}

// functions to add/remove/modify elements
void call_stapi_v2_add_elements(st_context_v2_t pcxt)
{
    // set up dummy optical properties
    args_optical_properties_set set = {"dummy", 1.1, 1.1, 2};
    args_optical_properties_face f = {.5, .5, .5, .5, 'g'};
    args_optical_properties_face b = {.25, .25, .25, .25, 'g'};
    uint_fast64_t opt_id = -1;
    ASSERT_EQ(
        st_add_optical_properties_set(pcxt, &set, &f, &b, &opt_id),
        st_return_code::SUCCESS);
    ASSERT_EQ(opt_id, 0);

    // check no elements set
    uint_fast64_t num = -1;
    uint_fast64_t id = -1;
    ASSERT_EQ(
        st_num_elements(pcxt, &num),
        st_return_code::SUCCESS);
    ASSERT_EQ(num, 0);

    // set element to non default values
    args_element el_args = { 2, 2, 2, 2, 2, 2, 2, false, true };
    args_geometry geom_args = { { 2 }, { 2, 2 }, 'c', 'p' };

    // test good element, expect st_return_code::SUCCESS
    ASSERT_EQ(
        st_add_element(pcxt, &el_args, opt_id, &geom_args, &id),
        st_return_code::SUCCESS);
    ASSERT_EQ(id, 1);
    ASSERT_EQ(
        st_num_elements(pcxt, &num),
        st_return_code::SUCCESS);
    ASSERT_EQ(num, 1);

    // test values set
    args_element rt_el;
    args_geometry rt_geom;
    int_fast64_t rt_opt_id = 0;
    ASSERT_EQ(
        st_get_element(pcxt, id, &rt_el, &rt_opt_id, &rt_geom),
        st_return_code::SUCCESS);
    
    ASSERT_EQ(el_args.enabled_flag, rt_el.enabled_flag);
    ASSERT_EQ(el_args.virtual_flag, rt_el.virtual_flag);
    ASSERT_EQ(el_args.x,            rt_el.x);
    ASSERT_EQ(el_args.y,            rt_el.y);
    ASSERT_EQ(el_args.z,            rt_el.z);
    ASSERT_EQ(el_args.ax,           rt_el.ax);
    ASSERT_EQ(el_args.ay,           rt_el.ay);
    ASSERT_EQ(el_args.az,           rt_el.az);
    ASSERT_EQ(el_args.zrot,         rt_el.zrot);

    ASSERT_EQ(rt_opt_id, opt_id);
    
    ASSERT_EQ(geom_args.ap, rt_geom.ap);
    ASSERT_EQ(geom_args.a_params[0], rt_geom.a_params[0]);

    ASSERT_EQ(geom_args.surf, rt_geom.surf);
    ASSERT_EQ(rt_geom.s_params[0], 1. / 4.);
    ASSERT_EQ(rt_geom.s_params[1], 1. / 4.);

    // try bad optical id, expect st_return_code::DATA_VALUE_NOT_FOUND
    ASSERT_EQ(
        st_add_element(pcxt, &el_args, rt_opt_id + 1, &geom_args, &id),
        st_return_code::DATA_VALUE_NOT_FOUND);
    ASSERT_EQ(id, 1);
    ASSERT_EQ(
        st_num_elements(pcxt, &num),
        st_return_code::SUCCESS);
    ASSERT_EQ(num, 1);

    // try bad aperture type, expect st_return_code::INVALID_ARGUMENTS
    args_geometry geom_bad_ap = { { 2 }, { 2, 2 }, 'z', 'p' };
    ASSERT_EQ(
        st_add_element(pcxt, &el_args, rt_opt_id, &geom_bad_ap, &id),
        st_return_code::INVALID_ARGUMENTS);
    ASSERT_EQ(id, 1);
    ASSERT_EQ(
        st_num_elements(pcxt, &num),
        st_return_code::SUCCESS);
    ASSERT_EQ(num, 1);

    // try bad surface type, expect st_return_code::INVALID_ARGUMENTS
    args_geometry geom_bad_surf = { { 2 }, { 2, 2 }, 'c', 'z' };
    ASSERT_EQ(
        st_add_element(pcxt, &el_args, rt_opt_id, &geom_bad_surf, &id),
        st_return_code::INVALID_ARGUMENTS);
    ASSERT_EQ(id, 1);
    ASSERT_EQ(
        st_num_elements(pcxt, &num),
        st_return_code::SUCCESS);
    ASSERT_EQ(num, 1);
    
    // try bad aperture params, expect st_return_code::INVALID_ARGUMENTS
    args_geometry geom_bad_ap_params = { { -2 }, { 2, 2 }, 'c', 'p' };
    ASSERT_EQ(
        st_add_element(pcxt, &el_args, rt_opt_id, &geom_bad_ap_params, &id),
        st_return_code::INVALID_ARGUMENTS);
    ASSERT_EQ(id, 1);
    ASSERT_EQ(
        st_num_elements(pcxt, &num),
        st_return_code::SUCCESS);
    ASSERT_EQ(num, 1);

    // try bad surface params, expect st_return_code::INVALID_ARGUMENTS
    args_geometry geom_bad_surf_params = { { 2 }, { std::numeric_limits<double>::quiet_NaN(), 2 }, 'c', 'p'};
    ASSERT_EQ(
        st_add_element(pcxt, &el_args, rt_opt_id, &geom_bad_surf_params, &id),
        st_return_code::INVALID_ARGUMENTS);
    ASSERT_EQ(id, 1);
    ASSERT_EQ(
        st_num_elements(pcxt, &num),
        st_return_code::SUCCESS);
    ASSERT_EQ(num, 1);

    // test another good element, expect st_return_code::SUCCESS
    ASSERT_EQ(
        st_add_element(pcxt, &el_args, rt_opt_id, &geom_args, &id),
        st_return_code::SUCCESS);
    ASSERT_EQ(id, 2);
    ASSERT_EQ(
        st_num_elements(pcxt, &num),
        st_return_code::SUCCESS);
    ASSERT_EQ(num, 2);
}

void add_dummy_element(st_context_v2_t pcxt,
                       args_element    &el_args,
                       args_geometry   &geom_args,
                       uint_fast64_t   &id,
                       uint_fast64_t   &opt_id,
                       int32_t         group = -1)
{
    // set up dummy optical properties
    args_optical_properties_set set = {"dummy", 1.1, 1.1, 2};
    args_optical_properties_face f = {.5, .5, .5, .5, 'g'};
    args_optical_properties_face b = {.25, .25, .25, .25, 'g'};
    
    ASSERT_EQ(
        st_add_optical_properties_set(pcxt, &set, &f, &b, &opt_id),
        st_return_code::SUCCESS);
    
    // set element to non default values
    el_args = { 2, 2, 2, 2, 2, 2, 2, false, true };
    el_args.group = group;
    geom_args = { { 2 }, { 2, 2 }, 'c', 'p' };
    
    if (group < -1)
    {
        ASSERT_EQ(
            st_add_element(pcxt, &el_args, opt_id, &geom_args, &id),
            st_return_code::WARNING_GROUP_IGNORED);
    }
    else
    {
        ASSERT_EQ(
            st_add_element(pcxt, &el_args, opt_id, &geom_args, &id),
            st_return_code::SUCCESS);
    }
}

void call_stapi_v2_get_element(st_context_v2_t pcxt)
{
    args_element  el_args;
    args_geometry geom_args;
    uint_fast64_t id = -1;
    uint_fast64_t opt_id = -1;
    add_dummy_element(pcxt, el_args, geom_args, id, opt_id);
    
    args_element  rt_el_args;
    int_fast64_t  rt_opt_id;
    args_geometry rt_geom_args;
    
    ASSERT_EQ(
        st_get_element(pcxt, id, &rt_el_args, &rt_opt_id, &rt_geom_args),
        st_return_code::SUCCESS);

    // test values set
    ASSERT_EQ(el_args.enabled_flag, rt_el_args.enabled_flag);
    ASSERT_EQ(el_args.virtual_flag, rt_el_args.virtual_flag);
    ASSERT_EQ(el_args.x,            rt_el_args.x);
    ASSERT_EQ(el_args.y,            rt_el_args.y);
    ASSERT_EQ(el_args.z,            rt_el_args.z);
    ASSERT_EQ(el_args.ax,           rt_el_args.ax);
    ASSERT_EQ(el_args.ay,           rt_el_args.ay);
    ASSERT_EQ(el_args.az,           rt_el_args.az);
    ASSERT_EQ(el_args.zrot,         rt_el_args.zrot);

    ASSERT_EQ(opt_id, rt_opt_id);
    
    ASSERT_EQ(geom_args.ap, rt_geom_args.ap);
    ASSERT_EQ(geom_args.a_params[0], rt_geom_args.a_params[0]);

    ASSERT_EQ(geom_args.surf, rt_geom_args.surf);
    ASSERT_EQ(rt_geom_args.s_params[0], 1. / 4.);
    ASSERT_EQ(rt_geom_args.s_params[1], 1. / 4.);

    // expect st_return_code::DATA_VALUE_NOT_FOUND
    ASSERT_EQ(
        st_get_element(pcxt, id + 1, &rt_el_args, &rt_opt_id, &rt_geom_args),
        st_return_code::DATA_VALUE_NOT_FOUND);
}

void call_stapi_v2_remove_elements(st_context_v2_t pcxt)
{
    args_element  el_args;
    args_geometry geom_args;
    uint_fast64_t id = -1;
    uint_fast64_t opt_id = -1;
    add_dummy_element(pcxt, el_args, geom_args, id, opt_id);
    
    uint_fast64_t num = -1;
    ASSERT_EQ(
        st_num_elements(pcxt, &num),
        st_return_code::SUCCESS);
    ASSERT_EQ(num, 1);
    
    ASSERT_EQ(
        st_clear_elements(pcxt),
        st_return_code::SUCCESS);
    ASSERT_EQ(
        st_num_elements(pcxt, &num),
        st_return_code::SUCCESS);
    ASSERT_EQ(num, 0);
    
    // expect st_return_code::SUCCESS
    add_dummy_element(pcxt, el_args, geom_args, id, opt_id);
    ASSERT_EQ(id, 2);
    ASSERT_EQ(
        st_num_elements(pcxt, &num),
        st_return_code::SUCCESS);
    ASSERT_EQ(num, 1);

    // expect st_return_code::WARNING_NOT_FOUND
    ASSERT_EQ(
        st_delete_element(pcxt, id - 1),
        st_return_code::WARNING_NOT_FOUND);
    ASSERT_EQ(
        st_num_elements(pcxt, &num),
        st_return_code::SUCCESS);
    ASSERT_EQ(num, 1);
    
    // expect st_return_code::SUCCESS
    ASSERT_EQ(
        st_delete_element(pcxt, id),
        st_return_code::SUCCESS);
    ASSERT_EQ(
        st_num_elements(pcxt, &num),
        st_return_code::SUCCESS);
    ASSERT_EQ(num, 0);
}

void call_stapi_v2_toggle_element(st_context_v2_t pcxt)
{
    args_element  el_args;
    args_geometry geom_args;
    uint_fast64_t id = -1;
    uint_fast64_t opt_id = -1;
    add_dummy_element(pcxt, el_args, geom_args, id, opt_id);
    
    // expect st_return_code::SUCCESS
    ASSERT_EQ(
        st_element_enabled(pcxt, id, true),
        st_return_code::SUCCESS);
    
    // expect st_return_code::SUCCESS
    ASSERT_EQ(
        st_element_virtual(pcxt, id, false),
        st_return_code::SUCCESS);
    
    // expect st_return_code::WARNING_NOT_FOUND
    ASSERT_EQ(
        st_element_enabled(pcxt, id + 1, false),
        st_return_code::WARNING_NOT_FOUND);
    
    // expect st_return_code::WARNING_NOT_FOUND
    ASSERT_EQ(
        st_element_virtual(pcxt, id + 1, true),
        st_return_code::WARNING_NOT_FOUND);

    // test values set
    args_element  rt_el_args;
    int_fast64_t  rt_opt_id;
    args_geometry rt_geom_args;
    
    ASSERT_EQ(
        st_get_element(pcxt, id, &rt_el_args, &rt_opt_id, &rt_geom_args),
        st_return_code::SUCCESS);

    ASSERT_EQ(rt_el_args.enabled_flag, true);
    ASSERT_EQ(rt_el_args.virtual_flag, false);
}

void call_stapi_v2_element_xyz(st_context_v2_t pcxt)
{
    args_element  el_args;
    args_geometry geom_args;
    uint_fast64_t id = -1;
    uint_fast64_t opt_id = -1;
    add_dummy_element(pcxt, el_args, geom_args, id, opt_id);
    
    // expect st_return_code::SUCCESS
    ASSERT_EQ(
        st_element_xyz(pcxt, id, 1, 1, 1),
        st_return_code::SUCCESS);
    
    // expect st_return_code::WARNING_NOT_FOUND
    ASSERT_EQ(
        st_element_xyz(pcxt, id + 1, 2, 2, 2),
        st_return_code::WARNING_NOT_FOUND);

    // test values set
    args_element  rt_el_args;
    int_fast64_t  rt_opt_id;
    args_geometry rt_geom_args;
    
    ASSERT_EQ(
        st_get_element(pcxt, id, &rt_el_args, &rt_opt_id, &rt_geom_args),
        st_return_code::SUCCESS);

    ASSERT_EQ(rt_el_args.x, 1);
    ASSERT_EQ(rt_el_args.y, 1);
    ASSERT_EQ(rt_el_args.z, 1);
}

void call_stapi_v2_element_aim(st_context_v2_t pcxt)
{
    args_element  el_args;
    args_geometry geom_args;
    uint_fast64_t id = -1;
    uint_fast64_t opt_id = -1;
    add_dummy_element(pcxt, el_args, geom_args, id, opt_id);
    
    // expect st_return_code::SUCCESS
    ASSERT_EQ(
        st_element_aim(pcxt, id, 1, 1, 1),
        st_return_code::SUCCESS);

    // expect st_return_code::WARNING_NOT_FOUND
    ASSERT_EQ(
        st_element_aim(pcxt, id + 1, 2, 2, 2),
        st_return_code::WARNING_NOT_FOUND);

    // test values set
    args_element  rt_el_args;
    int_fast64_t  rt_opt_id;
    args_geometry rt_geom_args;
    
    ASSERT_EQ(
        st_get_element(pcxt, id, &rt_el_args, &rt_opt_id, &rt_geom_args),
        st_return_code::SUCCESS);

    ASSERT_EQ(rt_el_args.ax, 1);
    ASSERT_EQ(rt_el_args.ay, 1);
    ASSERT_EQ(rt_el_args.az, 1);
}

void call_stapi_v2_element_zrot(st_context_v2_t pcxt)
{
    args_element  el_args;
    args_geometry geom_args;
    uint_fast64_t id = -1;
    uint_fast64_t opt_id = -1;
    add_dummy_element(pcxt, el_args, geom_args, id, opt_id);
    
    // expect st_return_code::SUCCESS
    ASSERT_EQ(
        st_element_zrot(pcxt, id, 1),
        st_return_code::SUCCESS);

    // expect st_return_code::WARNING_NOT_FOUND
    ASSERT_EQ(
        st_element_zrot(pcxt, id + 1, 2),
        st_return_code::WARNING_NOT_FOUND);

    // test values set
    args_element  rt_el_args;
    int_fast64_t  rt_opt_id;
    args_geometry rt_geom_args;
    
    ASSERT_EQ(
        st_get_element(pcxt, id, &rt_el_args, &rt_opt_id, &rt_geom_args),
        st_return_code::SUCCESS);

    ASSERT_EQ(rt_el_args.zrot, 1);
}

void call_stapi_v2_element_aperture(st_context_v2_t pcxt)
{
    args_element  el_args;
    args_geometry geom_args;
    uint_fast64_t id = -1;
    uint_fast64_t opt_id = -1;
    add_dummy_element(pcxt, el_args, geom_args, id, opt_id);

    double params[2] = { 4, 4 };

    ASSERT_EQ(
        st_element_aperture(pcxt, id, 'r', params),
        st_return_code::SUCCESS);

    args_element  rt_el_args;
    int_fast64_t  rt_opt_id;
    args_geometry rt_geom_args;
    
    ASSERT_EQ(
        st_get_element(pcxt, id, &rt_el_args, &rt_opt_id, &rt_geom_args),
        st_return_code::SUCCESS);
    
    ASSERT_EQ(rt_geom_args.ap, 'r');
    ASSERT_EQ(rt_geom_args.a_params[0], 4);
    ASSERT_EQ(rt_geom_args.a_params[1], 4);

    // expect st_return_code::INVALID_ARGUMENTS
    ASSERT_EQ(
        st_element_aperture(pcxt, id, 'z', params),
        st_return_code::INVALID_ARGUMENTS);

    double bad_params[2] = { -4, 4 };
    // expect st_return_code::INVALID_ARGUMENTS
    ASSERT_EQ(
        st_element_aperture(pcxt, id, 'r', bad_params),
        st_return_code::INVALID_ARGUMENTS);
}

void call_stapi_v2_element_surface(st_context_v2_t pcxt)
{
    args_element  el_args;
    args_geometry geom_args;
    uint_fast64_t id = -1;
    uint_fast64_t opt_id = -1;
    add_dummy_element(pcxt, el_args, geom_args, id, opt_id);
    
    double params[1] = { 4 };
    
    ASSERT_EQ(
        st_element_surface(pcxt, id, 's', params),
        st_return_code::SUCCESS);

    args_element  rt_el_args;
    int_fast64_t  rt_opt_id;
    args_geometry rt_geom_args;
    
    ASSERT_EQ(
        st_get_element(pcxt, id, &rt_el_args, &rt_opt_id, &rt_geom_args),
        st_return_code::SUCCESS);
    
    ASSERT_EQ(rt_geom_args.surf, 's');
    ASSERT_EQ(rt_geom_args.s_params[0], 4);
    
    // expect st_return_code::INVALID_ARGUMENTS
    ASSERT_EQ(
        st_element_surface(pcxt, id, 'z', params),
        st_return_code::INVALID_ARGUMENTS);

    double bad_params[1] = { -4 };
    // expect st_return_code::INVALID_ARGUMENTS
    ASSERT_EQ(
        st_element_surface(pcxt, id, 's', bad_params),
        st_return_code::INVALID_ARGUMENTS);
}

void call_stapi_v2_element_geometry(st_context_v2_t pcxt)
{
    args_element  el_args;
    args_geometry geom_args;
    uint_fast64_t id = -1;
    uint_fast64_t opt_id = -1;
    add_dummy_element(pcxt, el_args, geom_args, id, opt_id);
    
    // test other good geometry
    args_geometry geom_other = { { 4, 4 }, { 4 }, 'r', 's'};
    ASSERT_EQ(
        st_element_geometry(pcxt, id, &geom_other),
        st_return_code::SUCCESS);

    // try bad aperture type, expect st_return_code::INVALID_ARGUMENTS
    args_geometry geom_bad_ap = { { 2 }, { 2, 2 }, 'z', 'p' };
    ASSERT_EQ(
        st_element_geometry(pcxt, id, &geom_bad_ap),
        st_return_code::INVALID_ARGUMENTS);

    // try bad surface type, expect st_return_code::INVALID_ARGUMENTS
    args_geometry geom_bad_surf = { { 2 }, { 2, 2 }, 'c', 'z' };
    ASSERT_EQ(
        st_element_geometry(pcxt, id, &geom_bad_surf),
        st_return_code::INVALID_ARGUMENTS);

    // try bad aperture params, expect st_return_code::INVALID_ARGUMENTS
    args_geometry geom_bad_ap_params = { { -2 }, { 2, 2 }, 'c', 'p' };
    ASSERT_EQ(
        st_element_geometry(pcxt, id, &geom_bad_ap_params),
        st_return_code::INVALID_ARGUMENTS);

    // try bad surface params, expect st_return_code::INVALID_ARGUMENTS
    args_geometry geom_bad_surf_params = { { 2 }, { std::numeric_limits<double>::quiet_NaN(), 2 }, 'c', 'p'};
    ASSERT_EQ(
        st_element_geometry(pcxt, id, &geom_bad_surf_params),
        st_return_code::INVALID_ARGUMENTS);
}

void call_stapi_v2_element_optic(st_context_v2_t pcxt)
{
    args_element  el_args;
    args_geometry geom_args;
    uint_fast64_t id = -1;
    uint_fast64_t opt_id = -1;
    add_dummy_element(pcxt, el_args, geom_args, id, opt_id);
    
    // expect st_return_code::DATA_VALUE_NOT_FOUND
    ASSERT_EQ(
        st_element_optic(pcxt, id, opt_id + 1),
        st_return_code::DATA_VALUE_NOT_FOUND);

    // set up other optical properties
    args_optical_properties_set other_set = {"other", 1.1, 1.1, 2};
    args_optical_properties_face other_f = {.05, .05, .05, .05, 'g'};
    args_optical_properties_face other_b = {.025, .025, .025, .025, 'g'};
    
    ASSERT_EQ(
        st_add_optical_properties_set(pcxt, &other_set, &other_f, &other_b, &opt_id),
        st_return_code::SUCCESS);
    
    ASSERT_EQ(
        st_element_optic(pcxt, id, opt_id),
        st_return_code::SUCCESS);
    
    // check values were set
    args_optical_properties_set rt_set;
    args_optical_properties_face rt_f, rt_b;

    ASSERT_EQ(
        st_get_optical_properties_set(pcxt, id, &rt_set, &rt_f, &rt_b),
        st_return_code::SUCCESS);

    ASSERT_EQ(std::string(rt_set.name), std::string(other_set.name));
    ASSERT_EQ(rt_set.type, other_set.type);
    ASSERT_OPTICAL_SIDE_EQ(other_f, rt_f);
    ASSERT_OPTICAL_SIDE_EQ(other_b, rt_b);
}

void call_stapi_v2_element_group(st_context_v2_t pcxt)
{
    args_element  el_args;
    args_geometry geom_args;
    uint_fast64_t id = -1;
    uint_fast64_t opt_id = -1;
    // expect st_return_code::WARNING_GROUP_IGNORED
    add_dummy_element(pcxt, el_args, geom_args, id, opt_id, -2);

    args_element rt_el_args;
    int_fast64_t rt_opt_id = -1;
    args_geometry rt_geom_args;
    
    ASSERT_EQ(
        st_get_element(pcxt, id, &rt_el_args, &rt_opt_id, &rt_geom_args),
        st_return_code::SUCCESS);
    ASSERT_EQ(rt_el_args.group, -1);
    
    // expect st_return_code::WARNING_GROUP_IGNORED
    ASSERT_EQ(
        st_element_group(pcxt, id, -2),
        st_return_code::WARNING_GROUP_IGNORED);
    ASSERT_EQ(
        st_get_element(pcxt, id, &rt_el_args, &rt_opt_id, &rt_geom_args),
        st_return_code::SUCCESS);
    ASSERT_EQ(rt_el_args.group, -1);
    
    // expect st_return_code::SUCCESS
    ASSERT_EQ(
        st_element_group(pcxt, id, 1),
        st_return_code::SUCCESS);
    ASSERT_EQ(
        st_get_element(pcxt, id, &rt_el_args, &rt_opt_id, &rt_geom_args),
        st_return_code::SUCCESS);
    ASSERT_EQ(rt_el_args.group, 1);
}

// sun functions
void call_stapi_v2_add_sun(st_context_v2_t pcxt)
{
    double good_angles[3]      = {0, 1, 2};
    double good_intensities[3] = {0, 1, 2};
    double bad_intensities[2]  = {0, -1};

    args_sun args = {3, 608, 303, 1000, 5, ' '};

    // test bad intensitites, expect st_return_code::EXCEPTION
    ASSERT_EQ(
        st_add_sun(pcxt, &args, good_angles, bad_intensities),
        st_return_code::EXCEPTION);

    // test good intensities, expect st_return_code::SUCCESS
    ASSERT_EQ(
        st_add_sun(pcxt, &args, good_angles, good_intensities),
        st_return_code::SUCCESS);

    args_sun rt_args;
    double *rt_angle     = new double[3];
    double *rt_intensity = new double[3];

    ASSERT_EQ(
        st_get_sun(pcxt, &rt_args, &rt_angle, &rt_intensity, 3),
        st_return_code::SUCCESS);
    ASSERT_EQ(rt_angle[0],     good_angles[0]);
    ASSERT_EQ(rt_angle[1],     good_angles[1]);
    ASSERT_EQ(rt_angle[2],     good_angles[2]);
    ASSERT_EQ(rt_intensity[0], good_intensities[0]);
    ASSERT_EQ(rt_intensity[1], good_intensities[1]);
    ASSERT_EQ(rt_intensity[2], good_intensities[2]);

    // test built in shapes
    args.npoints = 0;
    
    // expect st_return_code::WARNING_SUN_SHAPE_IGNORED
    ASSERT_EQ(
        st_add_sun(pcxt, &args, good_angles, good_intensities),
        st_return_code::WARNING_SUN_SHAPE_IGNORED);
    
    ASSERT_EQ(
        st_get_sun(pcxt, &rt_args, &rt_angle, &rt_intensity),
        st_return_code::SUCCESS);
    ASSERT_EQ(rt_args.shape,               'g');
    ASSERT_EQ(rt_args.sigma_halfwidth_csr, 4.65);

    // expect st_return_code::SUCCESS
    args.shape = 'g';
    ASSERT_EQ(
        st_add_sun(pcxt, &args, good_angles, good_intensities),
        st_return_code::SUCCESS);
    
    ASSERT_EQ(
        st_get_sun(pcxt, &rt_args, &rt_angle, &rt_intensity),
        st_return_code::SUCCESS);
    ASSERT_EQ(rt_args.shape,               'g');
    ASSERT_EQ(rt_args.sigma_halfwidth_csr, 5);
    
    // expect st_return_code::SUCCESS
    args.shape = 'p';
    ASSERT_EQ(
        st_add_sun(pcxt, &args, good_angles, good_intensities),
        st_return_code::SUCCESS);
    
    ASSERT_EQ(
        st_get_sun(pcxt, &rt_args, &rt_angle, &rt_intensity),
        st_return_code::SUCCESS);
    ASSERT_EQ(rt_args.shape,               'p');
    ASSERT_EQ(rt_args.sigma_halfwidth_csr, 5);

    // expect st_return_code::WARNING_SUN_SHAPE_IGNORED
    args.shape = 'l';
    ASSERT_EQ(
        st_add_sun(pcxt, &args, good_angles, good_intensities),
        st_return_code::WARNING_SUN_SHAPE_IGNORED);
    
    ASSERT_EQ(
        st_get_sun(pcxt, &rt_args, &rt_angle, &rt_intensity),
        st_return_code::SUCCESS);
    ASSERT_EQ(rt_args.shape,               'g');
    ASSERT_EQ(rt_args.sigma_halfwidth_csr, 4.65);

    // expect st_return_code::SUCCESS
    args.shape = 'b';
    args.sigma_halfwidth_csr = .5;
    ASSERT_EQ(
        st_add_sun(pcxt, &args, good_angles, good_intensities),
        st_return_code::SUCCESS);
    
    ASSERT_EQ(
        st_get_sun(pcxt, &rt_args, &rt_angle, &rt_intensity),
        st_return_code::SUCCESS);
    ASSERT_EQ(rt_args.shape,               'b');
    ASSERT_EQ(rt_args.sigma_halfwidth_csr, .5);
    
    delete[] rt_angle;
    delete[] rt_intensity;
}

void call_stapi_v2_get_sun(st_context_v2_t pcxt)
{
    args_sun rt_args;
    double *rt_angle     = new double[3];
    double *rt_intensity = new double[3];

    // expect st_return_code::DATA_VALUE_NOT_FOUND
    ASSERT_EQ(
        st_get_sun(pcxt, &rt_args, &rt_angle, &rt_intensity),
        st_return_code::DATA_VALUE_NOT_FOUND);

    args_sun args = {0, 608, 303, 1000, 5, 'g'};
    
    ASSERT_EQ(
        st_add_sun(pcxt, &args, {}, {}),
        st_return_code::SUCCESS);
    
    ASSERT_EQ(
        st_get_sun(pcxt, &rt_args, &rt_angle, &rt_intensity),
        st_return_code::SUCCESS);

    ASSERT_EQ(rt_args.npoints,             args.npoints);
    ASSERT_EQ(rt_args.shape,               args.shape);
    ASSERT_EQ(rt_args.sigma_halfwidth_csr, args.sigma_halfwidth_csr);
    ASSERT_EQ(rt_args.x,                   args.x);
    ASSERT_EQ(rt_args.y,                   args.y);
    ASSERT_EQ(rt_args.z,                   args.z);

    double angles[3]      = {0, 1, 2};
    double intensities[3] = {0, 1, 2};
    args = {3, 608, 303, 1000, 5, 'd'};
    ASSERT_EQ(
        st_add_sun(pcxt, &args, angles, intensities),
        st_return_code::SUCCESS);
        
    ASSERT_EQ(
        st_get_sun(pcxt, &rt_args, &rt_angle, &rt_intensity, 3),
        st_return_code::SUCCESS);
        
    ASSERT_EQ(rt_angle[0],     angles[0]);
    ASSERT_EQ(rt_angle[1],     angles[1]);
    ASSERT_EQ(rt_angle[2],     angles[2]);
    ASSERT_EQ(rt_intensity[0], intensities[0]);
    ASSERT_EQ(rt_intensity[1], intensities[1]);
    ASSERT_EQ(rt_intensity[2], intensities[2]);
    
    ASSERT_EQ(
        st_get_sun(pcxt, &rt_args, &rt_angle, &rt_intensity),
        st_return_code::INVALID_ARGUMENTS);
    ASSERT_EQ(
        st_get_sun(pcxt, &rt_args, &rt_angle, &rt_intensity, 4),
        st_return_code::INVALID_ARGUMENTS);

    delete[] rt_angle;
    delete[] rt_intensity;
}

void call_stapi_v2_sun_shape(st_context_v2_t pcxt)
{
    double good_angles[3]      = {0, 1, 2};
    double good_intensities[3] = {0, 1, 2};

    args_sun args = {3, 608, 303, 1000, 5, 'g'};

    ASSERT_EQ(
        st_add_sun(pcxt, &args, good_angles, good_intensities),
        st_return_code::SUCCESS);
    
    // bad sun shape
    ASSERT_EQ(
        st_sun_shape(pcxt, ' ', 0),
        st_return_code::WARNING_SUN_SHAPE_IGNORED);

    args_sun rt_args;
    double *rt_angle     = new double[3];
    double *rt_intensity = new double[3];
    
    ASSERT_EQ(
        st_get_sun(pcxt, &rt_args, &rt_angle, &rt_intensity),
        st_return_code::SUCCESS);
    ASSERT_EQ(rt_args.shape,               'g');
    ASSERT_EQ(rt_args.sigma_halfwidth_csr, 4.65);

    // tests below follow switch statement order in st_sun_shape

    // gaussian
    ASSERT_EQ(
        st_sun_shape(pcxt, 'g', 5),
        st_return_code::SUCCESS);
    
    ASSERT_EQ(
        st_get_sun(pcxt, &rt_args, &rt_angle, &rt_intensity),
        st_return_code::SUCCESS);
    ASSERT_EQ(rt_args.shape,               'g');
    ASSERT_EQ(rt_args.sigma_halfwidth_csr, 5);
    
    // pillbox
    ASSERT_EQ(
        st_sun_shape(pcxt, 'p', 5),
        st_return_code::SUCCESS);
    
    ASSERT_EQ(
        st_get_sun(pcxt, &rt_args, &rt_angle, &rt_intensity),
        st_return_code::SUCCESS);
    ASSERT_EQ(rt_args.shape,               'p');
    ASSERT_EQ(rt_args.sigma_halfwidth_csr, 5);
    
    // expect st_return_code::WARNING_SUN_SHAPE_IGNORED
    ASSERT_EQ(
        st_sun_shape(pcxt, 'l', 5),
        st_return_code::WARNING_SUN_SHAPE_IGNORED);
    
    ASSERT_EQ(
        st_get_sun(pcxt, &rt_args, &rt_angle, &rt_intensity),
        st_return_code::SUCCESS);
    ASSERT_EQ(rt_args.shape,               'g');
    ASSERT_EQ(rt_args.sigma_halfwidth_csr, 4.65);
    
    // buie
    ASSERT_EQ(
        st_sun_shape(pcxt, 'b', .5),
        st_return_code::SUCCESS);
    
    ASSERT_EQ(
        st_get_sun(pcxt, &rt_args, &rt_angle, &rt_intensity),
        st_return_code::SUCCESS);
    ASSERT_EQ(rt_args.shape,               'b');
    ASSERT_EQ(rt_args.sigma_halfwidth_csr, .5);

    delete[] rt_angle;
    delete[] rt_intensity;
}

void call_stapi_v2_sun_xyz(st_context_v2_t pcxt)
{
    args_sun args = {0, 608, 303, 1000, 5, 'g'};

    ASSERT_EQ(
        st_add_sun(pcxt, &args, {}, {}),
        st_return_code::SUCCESS);

    ASSERT_EQ(
        st_sun_xyz(pcxt, 0, 0, 0),
        st_return_code::SUCCESS);

    args_sun rt_args;
    
    ASSERT_EQ(
        st_get_sun(pcxt, &rt_args, nullptr, nullptr),
        st_return_code::SUCCESS);

    ASSERT_EQ(rt_args.x, 0);
    ASSERT_EQ(rt_args.y, 0);
    ASSERT_EQ(rt_args.z, 0);
}

void call_stapi_v2_sun_userdata(st_context_v2_t pcxt)
{
    double good_angles[3]      = {0, 1, 2};
    double good_intensities[3] = {0, 1, 2};

    args_sun args = {3, 608, 303, 1000, 5, 'g'};

    ASSERT_EQ(
        st_add_sun(pcxt, &args, good_angles, good_intensities),
        st_return_code::SUCCESS);

    double new_angles[3]      = {0, .1, .2};
    double new_intensities[3] = {0, .1, .2};
    
    ASSERT_EQ(
        st_sun_userdata(pcxt, 3, new_angles, new_intensities),
        st_return_code::SUCCESS);

    args_sun rt_args;
    double *rt_angles      = new double[3];
    double *rt_intensities = new double[3];
    
    ASSERT_EQ(
        st_get_sun(pcxt, &rt_args, &rt_angles, &rt_intensities, 3),
        st_return_code::SUCCESS);

    ASSERT_EQ(rt_angles[0],      new_angles[0]);
    ASSERT_EQ(rt_angles[1],      new_angles[1]);
    ASSERT_EQ(rt_angles[2],      new_angles[2]);
    ASSERT_EQ(rt_intensities[0], new_intensities[0]);
    ASSERT_EQ(rt_intensities[1], new_intensities[1]);
    ASSERT_EQ(rt_intensities[2], new_intensities[2]);
    
    double bad_intensities[2] = {0, -1};
    
    ASSERT_EQ(
        st_sun_userdata(pcxt, 3, good_angles, bad_intensities),
        st_return_code::EXCEPTION);

    delete[] rt_angles;
    delete[] rt_intensities;
}

typedef struct expected_solar_values {
    double azimuth;
    double elevation;
    double sun_x;
    double sun_y;
    double sun_z;
} expected_solar_values;

void test_calculator_method(st_context_v2_t pcxt,
                            st_spcm calc,
                            expected_solar_values *expected)
{
    args_sun_location loc = { 40.0, -105.0, -7.0 };
    args_sun_datetime dt  = { 2025, 6, 20, 12 };

    double azimuth, elevation, zenith, sun_x, sun_y, sun_z;
    
    // azimuth/elevation
    ASSERT_EQ(
        st_get_sun_az_el(pcxt, calc, &loc, &dt, &azimuth, &elevation),
        st_return_code::SUCCESS);
    
    ASSERT_NEAR(azimuth, expected->azimuth, 1e-3);
    ASSERT_NEAR(elevation, expected->elevation, 1e-3);
    
    // azimuth/elevation
    ASSERT_EQ(
        st_get_sun_az_zen(pcxt, calc, &loc, &dt, &azimuth, &zenith),
        st_return_code::SUCCESS);
    
    double expected_zenith = 90.0 - expected->elevation;
    ASSERT_NEAR(azimuth, expected->azimuth, 1e-3);
    ASSERT_NEAR(zenith, expected_zenith, 1e-3);

    // sun vector
    ASSERT_EQ(
        st_get_sun_vector(pcxt, calc, &loc, &dt, &sun_x, &sun_y, &sun_z),
        st_return_code::SUCCESS);

    // on macOS sun_x diff is 0.000003 -> changed bound from 1e-6 to 5e-6
    ASSERT_NEAR(sun_x, expected->sun_x, 5e-6);
    ASSERT_NEAR(sun_y, expected->sun_y, 5e-6);
    ASSERT_NEAR(sun_z, expected->sun_z, 5e-6);
}

void call_stapi_v2_solar_calculator(st_context_v2_t pcxt)
{
    // method: SPA
    // Expected values - confirmed with https://gml.noaa.gov/grad/solcalc/azel.html
    expected_solar_values expected_SPA = {178.61128380, 73.439035265, 0.006908, -0.2849516, 0.9585169};
    test_calculator_method(pcxt,
                           st_spcm::SPA,
                           &expected_SPA);
    
    // method: SPA_ORIGINAL
    expected_solar_values expected_SPA_ORIGINAL = {178.614529, 73.443097, 0.006890135, -0.284884148, 0.95853719};
    test_calculator_method(pcxt,
                           st_spcm::SPA_ORIGINAL,
                           &expected_SPA_ORIGINAL);
   
    // method: SOLPOS
    expected_solar_values expected_SOLPOS = {178.6119537, 73.430931, 0.006907870, -0.28508729, 0.95847666};
    test_calculator_method(pcxt,
                           st_spcm::SOLPOS,
                           &expected_SOLPOS);
   
    // method: LEGACY
    expected_solar_values expected_LEGACY = {179.9991897, 73.435378, 4.031894e-06, -0.28509658, 0.958498794};
    test_calculator_method(pcxt,
                           st_spcm::LEGACY,
                           &expected_LEGACY);
   
    // method: DUFFIE
    expected_solar_values expected_DUFFIE = {179.110386, 73.43996784, 0.00442455, -0.284985454, 0.958521629};
    test_calculator_method(pcxt,
                           st_spcm::DUFFIE,
                           &expected_DUFFIE);
   
    // test bad calc input, expect st_return_code::INVALID_ARGUMENTS
    args_sun_location loc = { 40.0, -105.0, -7.0 };
    args_sun_datetime dt  = { 2025, 6, 20, 12 };

    double azimuth, elevation, zenith, sun_x, sun_y, sun_z;

    ASSERT_EQ(
        st_get_sun_az_el(pcxt,
                         st_spcm::CALCULATOR_COUNT,
                         &loc, &dt, &azimuth, &elevation),
        st_return_code::INVALID_ARGUMENTS);
    ASSERT_EQ(
        st_get_sun_az_zen(pcxt,
                          st_spcm::CALCULATOR_COUNT,
                          &loc, &dt, &azimuth, &zenith),
        st_return_code::INVALID_ARGUMENTS);
    ASSERT_EQ(
        st_get_sun_vector(pcxt,
                          st_spcm::CALCULATOR_COUNT,
                          &loc, &dt, &sun_x, &sun_y, &sun_z),
        st_return_code::INVALID_ARGUMENTS);
}

//////////////////////////////////
// Simlulation Runner Functions //
//////////////////////////////////

// functions for SolTrace runner management
void call_stapi_v2_sim_run_v2(st_context_v2_t pcxt, st_runner_type_t runner_type)
{
    if (runner_type != st_runner_type_t::OPTIX)
        st_sim_rays(pcxt, 1000, 10000);

    ASSERT_EQ(st_sim_setup(pcxt, runner_type),
              st_return_code::SUCCESS);
    ASSERT_EQ(st_sim_run_v2(pcxt),
              st_return_code::SUCCESS);
}

void call_stapi_v2_sim_report(st_context_v2_t pcxt, st_runner_type_t runner_type)
{
    if (runner_type != st_runner_type_t::OPTIX)
        st_sim_rays(pcxt, 1000, 10000);

    ASSERT_EQ(st_sim_setup(pcxt, runner_type),
              st_return_code::SUCCESS);
    // hasn't run yet, expect st_return_code::RUNNER_NOT_READY_TO_REPORT
    ASSERT_EQ(st_sim_report(pcxt, st_runner_statistics::RAY_RECORDS),
              st_return_code::RUNNER_NOT_READY_TO_REPORT);

    ASSERT_EQ(st_sim_run_v2(pcxt),
              st_return_code::SUCCESS);

    // bad arguments, expect st_return_code::INVALID_ARGUMENTS
    ASSERT_EQ(st_sim_report(pcxt, st_runner_statistics::RAY_RECORDS - 1),
              st_return_code::INVALID_ARGUMENTS);
    ASSERT_EQ(st_sim_report(pcxt, st_runner_statistics::STATISTICS_COUNT),
              st_return_code::INVALID_ARGUMENTS);

    ASSERT_EQ(st_sim_report(pcxt, st_runner_statistics::GROUPED_COUNTS),
              st_return_code::SUCCESS);
}

void test_move_element(st_context_v2_t pcxt, st_runner_type_t runner_type)
{
    // set up parameters
    st_sim_rays(pcxt, 1000, 10000);
    st_sim_seed(pcxt, 608);
    st_sim_errors(pcxt, false, false);
    
    // set up sun
    args_sun sun_args = { 0, 0, 0, 1000, 4.65, 'p' };
    st_add_sun(pcxt, &sun_args, nullptr, nullptr);
    
    // set up absorber
    uint_fast64_t abs_opt_id;
    args_optical_properties_set abs_set = { "absorber", 1.1, 1.1, 2 };
    args_optical_properties_face abs_f = { 0, 0, 0, 0, 'g' };
    args_optical_properties_face abs_b = { 0, 0, 0, 0, 'g' };

    st_add_optical_properties_set(pcxt, &abs_set, &abs_f, &abs_b, &abs_opt_id);

    uint_fast64_t abs_el_id;
    args_element absorber_el = { 0, 0, 0, 0, 0, -100, 0, true, false };
    args_geometry absorber_geom = { { 2, 2 }, { 1 }, 'r', 't' };

    st_add_element(pcxt, &absorber_el, abs_opt_id, &absorber_geom, &abs_el_id);

    // set up reflector
    uint_fast64_t refl_opt_id;
    args_optical_properties_set refl_set = { "reflector", 1.1, 1.1, 2 };
    args_optical_properties_face refl_f = { 0, 1, 0, 0, 'g' };
    args_optical_properties_face refl_b = { 0, 1, 0, 0, 'g' };

    st_add_optical_properties_set(pcxt, &refl_set, &refl_f, &refl_b, &refl_opt_id);

    uint_fast64_t refl_el_id;
    args_element reflector_el = { 10, 0, 0, 0, 0, 10, 0, true, false };
    args_geometry reflector_geom = { { 2, 2 }, {}, 'r', 'f' };

    st_add_element(pcxt, &reflector_el, refl_opt_id, &reflector_geom, &refl_el_id);

    st_sim_setup(pcxt, runner_type);
    st_sim_run_v2(pcxt);
    st_sim_report(pcxt, st_runner_statistics::RAY_RECORDS);

    uint_fast64_t num_int;
    st_num_intersections(pcxt, &num_int);

    ASSERT_GT(num_int, 0);

    double *loc_x      = new double[num_int];
    double *loc_y      = new double[num_int];
    double *loc_z      = new double[num_int];
    uint_fast64_t *els = new uint_fast64_t[num_int];

    st_locations(pcxt, loc_x, loc_y, loc_z);
    st_elementmap(pcxt, els);

    uint_fast32_t hit_abs = 0;
    uint_fast32_t hit_refl = 0;

    for (int i = 1; i < num_int; ++i)
    {
        if (els[i] == refl_el_id)
            ++hit_refl;
        if ((els[i-1] == refl_el_id) && (els[i] == abs_el_id))
        {
            ++hit_abs;
            ASSERT_GE(loc_x[i], 0);
        }
    }
    ASSERT_GT(hit_refl, 0);
    ASSERT_GT(hit_abs, 0);

    delete[] loc_x;
    delete[] loc_y;
    delete[] loc_z;
    delete[] els;

    // move reflector and rerun
    st_element_xyz(pcxt, refl_el_id, -10, 0, 0);

    st_sim_setup(pcxt, runner_type);
    st_sim_run_v2(pcxt);
    st_sim_report(pcxt, st_runner_statistics::RAY_RECORDS);

    st_num_intersections(pcxt, &num_int);

    ASSERT_GT(num_int, 0);

    loc_x = new double[num_int];
    loc_y = new double[num_int];
    loc_z = new double[num_int];
    els   = new uint_fast64_t[num_int];

    st_locations(pcxt, loc_x, loc_y, loc_z);
    st_elementmap(pcxt, els);

    hit_abs = 0;
    hit_refl = 0;

    for (int i = 1; i < num_int; ++i)
    {
        if (els[i] == refl_el_id)
            ++hit_refl;
        if ((els[i-1] == refl_el_id) && (els[i] == abs_el_id))
        {
            ++hit_abs;
            ASSERT_LE(loc_x[i], 0);
        }
    }
    ASSERT_GT(hit_refl, 0);
    ASSERT_GT(hit_abs, 0);

    delete[] loc_x;
    delete[] loc_y;
    delete[] loc_z;
    delete[] els;
}

///////////////////////////////////
// Simlulation Results Functions //
///////////////////////////////////

void call_stapi_v2_write_results_csv(st_context_v2_t  pcxt, 
                                     st_runner_type_t runner_type, 
                                     const char       *filename)
{
    if (runner_type != st_runner_type_t::OPTIX)
        st_sim_rays(pcxt, 1000, 10000);

    ASSERT_EQ(st_sim_setup(pcxt, runner_type),
              st_return_code::SUCCESS);
    ASSERT_EQ(st_sim_run_v2(pcxt),
              st_return_code::SUCCESS);
    ASSERT_EQ(st_sim_report(pcxt, 0),
              st_return_code::SUCCESS);
    ASSERT_EQ(st_write_results_csv(pcxt, filename),
              st_return_code::SUCCESS);

    std::error_code ec;
    std::filesystem::remove(filename, ec);
}

void call_stapi_v2_write_group_results_json(st_context_v2_t  pcxt, 
                                            st_runner_type_t runner_type, 
                                            const char       *filename)
{
    if (runner_type != st_runner_type_t::OPTIX)
        st_sim_rays(pcxt, 1000, 10000);

    ASSERT_EQ(st_sim_setup(pcxt, runner_type),
              st_return_code::SUCCESS);
    ASSERT_EQ(st_sim_run_v2(pcxt),
              st_return_code::SUCCESS);
    ASSERT_EQ(st_sim_report(pcxt, 1),
              st_return_code::SUCCESS);
    ASSERT_EQ(st_write_group_results_json(pcxt, filename),
              st_return_code::SUCCESS);

    std::error_code ec;
    std::filesystem::remove(filename, ec);
}

// functions to get results directly

void call_stapi_v2_locations(st_context_v2_t  pcxt,
                             st_runner_type_t runner_type)
{
    st_sim_rays(pcxt, 1000, 10000);

    ASSERT_EQ(st_sim_setup(pcxt, runner_type), 
              st_return_code::SUCCESS);
    ASSERT_EQ(st_sim_run_v2(pcxt), 
              st_return_code::SUCCESS);
    ASSERT_EQ(st_sim_report(pcxt, 0), 
              st_return_code::SUCCESS);

    uint_fast64_t num;
    
    ASSERT_EQ(st_num_intersections(pcxt, &num),
              st_return_code::SUCCESS);
    
    ASSERT_GT(num, 0);
    
    double *loc_x = new double[num];
    double *loc_y = new double[num];
    double *loc_z = new double[num];

    ASSERT_EQ(st_locations(pcxt, loc_x, loc_y, loc_z),
              st_return_code::SUCCESS);

    // how should the values in loc be tested?

    delete[] loc_x;
    delete[] loc_y;
    delete[] loc_z;
}

void call_stapi_v2_cosines(st_context_v2_t  pcxt,
                           st_runner_type_t runner_type)
{
    st_sim_rays(pcxt, 1000, 10000);

    ASSERT_EQ(st_sim_setup(pcxt, runner_type),
              st_return_code::SUCCESS);
    ASSERT_EQ(st_sim_run_v2(pcxt),
              st_return_code::SUCCESS);
    ASSERT_EQ(st_sim_report(pcxt, 0),
              st_return_code::SUCCESS);

    uint_fast64_t num;
    
    ASSERT_EQ(st_num_intersections(pcxt, &num),
              st_return_code::SUCCESS);
    
    ASSERT_GT(num, 0);
    
    double *cos_x = new double[num];
    double *cos_y = new double[num];
    double *cos_z = new double[num];

    ASSERT_EQ(st_cosines(pcxt, cos_x, cos_y, cos_z),
              st_return_code::SUCCESS);

    // how should the values in cos be tested?

    delete[] cos_x;
    delete[] cos_y;
    delete[] cos_z;
}

void call_stapi_v2_elementmap(st_context_v2_t  pcxt,
                              st_runner_type_t runner_type)
{
    st_sim_rays(pcxt, 1000, 10000);

    ASSERT_EQ(st_sim_setup(pcxt, runner_type),
              st_return_code::SUCCESS);
    ASSERT_EQ(st_sim_run_v2(pcxt),
              st_return_code::SUCCESS);
    ASSERT_EQ(st_sim_report(pcxt, 0),
              st_return_code::SUCCESS);

    uint_fast64_t num;
    
    ASSERT_EQ(st_num_intersections(pcxt, &num),
              st_return_code::SUCCESS);
    
    ASSERT_GT(num, 0);
    
    uint_fast64_t *els = new uint_fast64_t[num];

    ASSERT_EQ(st_elementmap(pcxt, els),
              st_return_code::SUCCESS);

    for (uint_fast64_t n = 0; n < num; ++n)
        // assert valid element id if not the sun id
        if (els[n] != -1) ASSERT_GT(els[n], 0);

    delete[] els;
}

void call_stapi_v2_stagemap(st_context_v2_t  pcxt,
                            st_runner_type_t runner_type)
{
    st_sim_rays(pcxt, 1000, 10000);

    ASSERT_EQ(st_sim_setup(pcxt, runner_type),
              st_return_code::SUCCESS);
    ASSERT_EQ(st_sim_run_v2(pcxt),
              st_return_code::SUCCESS);
    ASSERT_EQ(st_sim_report(pcxt, 0),
              st_return_code::SUCCESS);

    uint_fast64_t num;
    
    ASSERT_EQ(st_num_intersections(pcxt, &num),
              st_return_code::SUCCESS);
    
    ASSERT_GT(num, 0);
    
    uint_fast64_t *stages = new uint_fast64_t[num];

    ASSERT_EQ(st_stagemap(pcxt, stages),
              st_return_code::SUCCESS);

    for (uint_fast64_t n = 0; n < num; ++n)
        ASSERT_EQ(stages[n], 0);

    delete[] stages;
}

void call_stapi_v2_raynumbers(st_context_v2_t  pcxt,
                              st_runner_type_t runner_type)
{
    st_sim_rays(pcxt, 1000, 10000);

    ASSERT_EQ(st_sim_setup(pcxt, runner_type),
              st_return_code::SUCCESS);
    ASSERT_EQ(st_sim_run_v2(pcxt),
              st_return_code::SUCCESS);
    ASSERT_EQ(st_sim_report(pcxt, 0),
              st_return_code::SUCCESS);

    uint_fast64_t num;
    
    ASSERT_EQ(st_num_intersections(pcxt, &num),
              st_return_code::SUCCESS);
    
    ASSERT_GT(num, 0);
    
    uint_fast64_t *raynumbers = new uint_fast64_t[num];

    ASSERT_EQ(st_raynumbers(pcxt, raynumbers),
              st_return_code::SUCCESS);

    for (uint_fast64_t n = 0; n < num; ++n)
        ASSERT_GE(raynumbers[n], 0);

    delete[] raynumbers;
}

void call_stapi_v2_sun_stats(st_context_v2_t  pcxt,
                             st_runner_type_t runner_type)
{
    st_sim_rays(pcxt, 1000, 10000);

    ASSERT_EQ(st_sim_setup(pcxt, runner_type),
              st_return_code::SUCCESS);
    ASSERT_EQ(st_sim_run_v2(pcxt),
              st_return_code::SUCCESS);
    ASSERT_EQ(st_sim_report(pcxt, 0),
              st_return_code::SUCCESS);

    double        height;
    double        width;
    double        area;
    uint_fast64_t nrunrays;

    ASSERT_EQ(st_sun_stats(pcxt,
                           &width,
                           &height,
                           &area,
                           &nrunrays),
              st_return_code::SUCCESS);

    if (runner_type == st_runner_type_t::OPTIX)
    {
        ASSERT_GT(area, 0);
    }
    else
    {
        ASSERT_GT(width, 0);
        ASSERT_GT(height, 0);
    }
    
    ASSERT_GT(nrunrays, 0);
}

void call_stapi_v2_get_results_data(st_context_v2_t  pcxt,
                                    st_runner_type_t runner_type)
{
    st_sim_rays(pcxt, 1000, 10000);

    ASSERT_EQ(st_sim_setup(pcxt, runner_type),
              st_return_code::SUCCESS);
    ASSERT_EQ(st_sim_run_v2(pcxt),
              st_return_code::SUCCESS);
    ASSERT_EQ(st_sim_report(pcxt, 0),
              st_return_code::SUCCESS);

    uint_fast64_t num;
    
    ASSERT_EQ(st_num_intersections(pcxt, &num),
              st_return_code::SUCCESS);
    
    ASSERT_GT(num, 0);
    
    args_results_data data;
    data.loc_x = new double[num];
    data.loc_y = new double[num];
    data.loc_z = new double[num];
    data.cos_x = new double[num];
    data.cos_y = new double[num];
    data.cos_z = new double[num];
    data.element_map = new uint_fast64_t[num];
    data.stage_map   = new uint_fast64_t[num];
    data.ray_numbers = new uint_fast64_t[num];

    ASSERT_EQ(st_get_results_data(pcxt, &data),
              st_return_code::SUCCESS);

    for (uint_fast64_t n = 0; n < num; ++n)
    {
        // assert valid element id if not the sun id
        if (data.element_map[n] != -1)
            ASSERT_GT(data.element_map[n], 0);
        ASSERT_EQ(data.stage_map[n], 0);
        ASSERT_GE(data.ray_numbers[n], 0);
    }

    delete[] data.loc_x;
    delete[] data.loc_y;
    delete[] data.loc_z;
    delete[] data.cos_x;
    delete[] data.cos_y;
    delete[] data.cos_z;
    delete[] data.element_map;
    delete[] data.stage_map;
    delete[] data.ray_numbers;
}

// functions to get grouped results directly

void call_stapi_v2_get_grouped_results_data(st_context_v2_t  pcxt,
                                            st_runner_type_t runner_type)
{
    if (runner_type != st_runner_type_t::OPTIX)
        st_sim_rays(pcxt, 1000, 10000);

    ASSERT_EQ(st_sim_setup(pcxt, runner_type),
              st_return_code::SUCCESS);
    ASSERT_EQ(st_sim_run_v2(pcxt),
              st_return_code::SUCCESS);
    ASSERT_EQ(st_sim_report(pcxt, 1),
              st_return_code::SUCCESS);

    size_t num_groups;
    ASSERT_EQ(st_num_groups(pcxt, &num_groups),
              st_return_code::SUCCESS);
    args_group_result *dummy_data = new args_group_result[num_groups];

    for (size_t i = 0; i < num_groups; ++i)
    {
        dummy_data[i] = args_group_result{};
        dummy_data[i].absorb_previous_group   = new uint_fast64_t[num_groups];
        dummy_data[i].reflect_previous_group  = new uint_fast64_t[num_groups];
        dummy_data[i].transmit_previous_group = new uint_fast64_t[num_groups];
        dummy_data[i].virtual_previous_group  = new uint_fast64_t[num_groups];
    }
    
    ASSERT_EQ(st_get_grouped_results_data(pcxt, dummy_data),
              st_return_code::SUCCESS);

    for (size_t i = 0; i < num_groups; ++i)
    {
        ASSERT_GT(dummy_data[i].absorb_sun_previous, 0);
    }

    // clean up
    for (size_t i = 0; i < num_groups; ++i)
    {
        delete[] dummy_data[i].absorb_previous_group;
        delete[] dummy_data[i].reflect_previous_group;
        delete[] dummy_data[i].transmit_previous_group;
        delete[] dummy_data[i].virtual_previous_group;
    }
    delete[] dummy_data;
}