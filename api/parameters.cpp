#include "exception_guard.hpp"
#include "utils.hpp"

///////////////////////////////////////////////////
// functions for simulation parameter management //
///////////////////////////////////////////////////

STAPI_V2 st_return_t st_set_simulation_parameters(st_context_v2_t pcxt, args_simulation_parameters *params) noexcept
{
return st_guard::guarded(st_utils::get_cb(pcxt), "st_set_simulation_parameters", [&]() -> st_return_t {
    CONTEXT(pcxt);
    DATA(cxt);
    
    data->set_number_of_rays(params->number_of_rays);
    data->set_max_rays_traced(params->max_number_of_rays);

    data->set_tolerance(params->tolerance);
    
    data->set_latitude(params->latitude);
    data->set_longitude(params->longitude);

    data->set_include_sun_shape_errors(params->include_sun_shape_errors);
    data->set_include_optical_errors(params->include_optical_errors);
    data->set_as_power_tower(params->as_power_tower);

    return st_return_code::SUCCESS;
});
}

STAPI_V2 st_return_t st_sim_params(st_context_v2_t pcxt,
								   uint_fast64_t   raycount,
								   uint_fast64_t   maxcount,
								   bool 		   include_dynamic_group) noexcept
{
return st_guard::guarded(st_utils::get_cb(pcxt), "st_sim_params", [&]() -> st_return_t {
	CONTEXT(pcxt);
    DATA(cxt);
    
    data->set_number_of_rays(raycount);
    data->set_max_rays_traced(maxcount);
    data->set_as_power_tower(include_dynamic_group);
    return st_return_code::SUCCESS;
});
}

STAPI_V2 st_return_t st_sim_rays(st_context_v2_t pcxt,
								 uint_fast64_t   raycount,
								 uint_fast64_t   maxcount) noexcept
{
return st_guard::guarded(st_utils::get_cb(pcxt), "st_sim_rays", [&]() -> st_return_t {
	CONTEXT(pcxt);
    DATA(cxt);
    
    data->set_number_of_rays(raycount);
    data->set_max_rays_traced(maxcount);
    return st_return_code::SUCCESS;
});
}

STAPI_V2 st_return_t st_sim_power_tower(st_context_v2_t pcxt,
								        bool 		    is_power_tower) noexcept
{
return st_guard::guarded(st_utils::get_cb(pcxt), "st_sim_power_tower", [&]() -> st_return_t {
	CONTEXT(pcxt);
    DATA(cxt);

    data->set_as_power_tower(is_power_tower);
    return st_return_code::SUCCESS;
});
}

STAPI_V2 st_return_t st_sim_errors(st_context_v2_t pcxt,
								   bool			   include_sun_shape,
								   bool			   include_optics) noexcept
{
return st_guard::guarded(st_utils::get_cb(pcxt), "st_sim_errors", [&]() -> st_return_t {
	CONTEXT(pcxt);
    DATA(cxt);
    
    data->set_include_sun_shape_errors((bool)include_sun_shape);
    data->set_include_optical_errors((bool)include_optics);
    return st_return_code::SUCCESS;
});
}

STAPI_V2 st_return_t st_sim_location(st_context_v2_t pcxt,
								     double		     latitude,
								     double		     longitude) noexcept
{
return st_guard::guarded(st_utils::get_cb(pcxt), "st_sim_location", [&]() -> st_return_t {
    CONTEXT(pcxt);
    DATA(cxt);

    data->set_latitude(latitude);
    data->set_longitude(longitude);

    return st_return_code::SUCCESS;
});
}

STAPI_V2 st_return_t st_sim_tolerance(st_context_v2_t pcxt, double tolerance) noexcept
{
return st_guard::guarded(st_utils::get_cb(pcxt), "st_sim_tolerance", [&]() -> st_return_t {
    CONTEXT(pcxt);
    DATA(cxt);

    data->set_tolerance(tolerance);

    return st_return_code::SUCCESS;
});
}

STAPI_V2 st_return_t st_get_simulation_parameters(st_context_v2_t pcxt, args_simulation_parameters *params) noexcept
{
return st_guard::guarded(st_utils::get_cb(pcxt), "st_get_simulation_parameters", [&]() -> st_return_t {
    CONTEXT(pcxt);
    DATA(cxt);

    params->number_of_rays = data->get_number_of_rays();
    params->max_number_of_rays = data->get_max_number_rays_traced();

    params->tolerance = data->get_tolerance();
    
    params->latitude = data->get_latitude();
    params->longitude = data->get_longitude();

    params->include_sun_shape_errors = data->get_include_sun_shape_errors();
    params->include_optical_errors = data->get_include_optical_errors();
    params->as_power_tower = data->get_as_power_tower();

    return st_return_code::SUCCESS;
});
}