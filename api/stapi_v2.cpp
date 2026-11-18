#include "exception_guard.hpp"
#include "stapi_v2.h"
#include "utils.hpp"

////////////////////////////////
// SolTrace Context Functions //
////////////////////////////////

/* functions for SolTrace context management */
STAPI_V2 st_return_t st_create_context(st_context_v2_t* pcxt, p_callback cb) noexcept
{
return st_guard::guarded(nullptr, "st_create_context", [&]() -> st_return_t {
    st_context* cxt = new st_context();
    cxt->p_data     = new SimulationData();
    cxt->p_cb       = cb;
    cxt->location   = fs::path();

    *pcxt = reinterpret_cast<st_context_v2_t>(cxt);
    return st_return_code::SUCCESS;
});
}

STAPI_V2 st_return_t st_reset_context(st_context_v2_t pcxt) noexcept
{
return st_guard::guarded(st_utils::get_cb(pcxt), "st_reset_context", [&]() -> st_return_t {
	CONTEXT(pcxt);

    delete cxt->p_data;
    delete cxt->p_runner;
    delete cxt->p_results;

    cxt->p_data       = new SimulationData();
    cxt->runner_type  = st_runner_type_t::RUNNER_COUNT;
    cxt->p_runner     = nullptr;
    cxt->report_level = RunnerStatistics::STATISTICS_COUNT;
    cxt->p_results    = nullptr;
    cxt->location     = fs::path();

	return st_return_code::SUCCESS;
});
}

STAPI_V2 st_return_t st_free_context(st_context_v2_t pcxt) noexcept
{
return st_guard::guarded(st_utils::get_cb(pcxt), "st_free_context", [&]() -> st_return_t {
	CONTEXT(pcxt);

    delete cxt->p_data;
    cxt->p_data    = nullptr;
    delete cxt->p_runner;
    cxt->p_runner  = nullptr;
    delete cxt->p_results;
    cxt->p_results = nullptr;

    delete cxt;
	return st_return_code::SUCCESS;
});
}

STAPI_V2 st_return_t st_locate_context(st_context_v2_t pcxt, const char *path) noexcept
{
return st_guard::guarded(st_utils::get_cb(pcxt), "st_locate_context", [&]() -> st_return_t {
	CONTEXT(pcxt);

    fs::path p(path);
    std::error_code ec;

    if (fs::exists(p, ec))
    {
        cxt->location = p;
        return st_return_code::SUCCESS;
    }
    else if (ec && cxt->p_cb)
    {
        cxt->p_cb("locate context", ec.message().c_str());
    }
    return st_return_code::INVALID_ARGUMENTS;
});
}

////////////////////////////////
// Simlulation Data Functions //
////////////////////////////////

// functions for SolTrace data management
// // functions for simulation data management thru json strings
// STAPI_V2 st_return_t st_read_input_json(st_context_v2_t pcxt, const char *json) noexcept
// {
// return st_guard::guarded(st_utils::get_cb(pcxt), [&]() -> st_return_t {
//     CONTEXT(pcxt);
//     DATA(cxt);

//     data->import_json_string(json);
//     return st_return_code::SUCCESS;
// });
// }

// STAPI_V2 st_return_t st_read_input_json_file(st_context_v2_t pcxt, const char *filename) noexcept
// {
// return st_guard::guarded(st_utils::get_cb(pcxt), [&]() -> st_return_t {
// 	CONTEXT(pcxt);
//     DATA(cxt);
    
//     data->import_json_file(std::string(filename));
//     return st_return_code::SUCCESS;
// });
// }

// // functions for writing input files for SolTrace
// STAPI_V2 st_return_t st_export_json_file(st_context_v2_t pcxt, const char *filename)
// {
//     CONTEXT(pcxt);
//     DATA(cxt);

//     ST_WRAP_CB_TRY_CATCH(data->export_json_file(std::string(filename)), cxt->p_cb);
//     return st_return_code::SUCCESS;
// }

///////////////////////////////////
// Simlulation Results Functions //
///////////////////////////////////

// STAPI_V2 st_return_t st_write_results_csv(st_context_v2_t pcxt, 
// 										  const char 	  *filename, 
// 										  int 			  precision)
// {
//     CONTEXT(pcxt);
//     RESULT(cxt);

//     if (cxt->report_level == RunnerStatistics::GROUPED_COUNTS)
//         return st_return_code::RESULT_NOT_REPORTED;

//     ST_WRAP_CB_TRY_CATCH(result->write_csv_file(filename, precision), cxt->p_cb);
//     return st_return_code::SUCCESS;
// }

// STAPI_V2 st_return_t st_write_group_results_json(st_context_v2_t pcxt, 
// 										         const char 	 *filename, 
// 										         int 			 precision,
//                                                  int             indent)
// {
//     CONTEXT(pcxt);
//     RESULT(cxt);

//     if (cxt->report_level == RunnerStatistics::RAY_RECORDS)
//         return st_return_code::RESULT_NOT_REPORTED;

//     ST_WRAP_CB_TRY_CATCH(result->write_group_json_file(filename, precision, indent), cxt->p_cb);
//     return st_return_code::SUCCESS;
// }

// TODO: make a way to get group results (number too)
//       make checks positive

/////////////////////////
// Batch api call work //
/////////////////////////

// could upgrade by moving non macro stuff to internal _st_* functions and fetch 
// full context at the beginning of the for loop

bool is_error(st_return_t code)
{
    return code > st_return_code::SUCCESS && code < st_return_code::WARNING_FELLBACK_FROM_EMBREE;
}

STAPI_V2 st_return_t st_batch(st_context_v2_t  pcxt,
							//   st_api_call_args **arguments, 
							  void             **arguments, 
							  st_uint_t        num_calls,
                              st_uint_t        *fail_iteration,
                              bool             verbose)
{
    CONTEXT(pcxt);

    st_return_t code = st_return_code::SUCCESS;
    
    // wrapping in try catch for runtime errors
    try {
        // for each pair of function and arguments, determine which function to call
        for (int i = 0; i < num_calls; ++i) {
            // cast the generic void* argument pointer back to its real type
            // so we can inspect the tag and reach the right payload
            st_api_call_args *call_args = (st_api_call_args*)arguments[i];
            
            if (verbose && cxt->p_cb) 
            {
                std::string msg = "batch call " + std::to_string(i);
                // TODO: magic_enum
                cxt->p_cb(msg.data(), std::to_string(call_args->type).c_str());
            }
            
            // based on the type of call, cast the generic function pointer
            // to the specific function pointer and signature.
            switch (call_args->type) {
        		// Simlulation Data Functions
                // functions for simulation data management thru json strings
                case st_api_call::CALL_ST_READ_INPUT_JSON:
                {
                    code = st_read_input_json(pcxt, call_args->payload.read_input_json_args.json);
                    break;
                }
                case st_api_call::CALL_ST_READ_INPUT_JSON_FILE:
                {
                    code = st_read_input_json_file(pcxt, call_args->payload.read_input_json_file_args.filename);
                    break;
                }
                // functions for simulation data management directly
                case st_api_call::CALL_ST_SET_SIMULATION_PARAMETERS:
                {
                    code = st_set_simulation_parameters(pcxt,
                                                        call_args->payload.set_simulation_parameters_args.params);
                    break;
                }
	            case st_api_call::CALL_ST_SIM_PARAMS:
                {
                    code = st_sim_params(pcxt,
                                         call_args->payload.sim_params_args.raycount,
                                         call_args->payload.sim_params_args.maxcount,
                                         call_args->payload.sim_params_args.include_dynamic_group);
                    break;
                }
	            case st_api_call::CALL_ST_SIM_RAYS:
                {
                    code = st_sim_rays(pcxt,
                                       call_args->payload.sim_rays_args.raycount,
                                       call_args->payload.sim_rays_args.maxcount);
                    break;
                }
	            case st_api_call::CALL_ST_SIM_POWER_TOWER:
                {
                    code = st_sim_power_tower(pcxt,
                                              call_args->payload.sim_power_tower_args.is_power_tower);
                    break;
                }
	            case st_api_call::CALL_ST_SIM_ERRORS:
                {
                    code = st_sim_errors(pcxt,
                                         call_args->payload.sim_errors_args.include_sun_shape,
                                         call_args->payload.sim_errors_args.include_optics);
                    break;
                }
                case st_api_call::CALL_ST_SIM_LOCATION:
                {
                    code = st_sim_location(pcxt,
                                         call_args->payload.sim_location_args.latitude,
                                         call_args->payload.sim_location_args.longitude);
                    break;
                }
                case st_api_call::CALL_ST_SIM_TOLERANCE:
                {
                    code = st_sim_tolerance(pcxt,
                                            call_args->payload.sim_tolerance_args.tolerance);
                    break;
                }
                // functions to add/remove optical properties
                case st_api_call::CALL_ST_NUM_OPTICS:
                {
                    code = st_num_optics(pcxt, call_args->payload.num_optics_args.num_optics);
                    break;
                }
                case st_api_call::CALL_ST_ADD_OPTICAL_PROPERIES_SET:
                {
                    code = st_add_optical_properties_set(pcxt,
                                                         call_args->payload.add_optical_properties_set_args.opt_set,
                                                         call_args->payload.add_optical_properties_set_args.front,
                                                         call_args->payload.add_optical_properties_set_args.back,
                                                         call_args->payload.add_optical_properties_set_args.optic_id);
                    break;
                }
                case st_api_call::CALL_ST_DELETE_OPTIC:
                {
                    code = st_delete_optic(pcxt, call_args->payload.delete_optic_args.idx);
                    break;
                }
                case st_api_call::CALL_ST_CLEAR_OPTICS:
                {
                    code = st_clear_optics(pcxt);
                    break;
                }
                // functions to add/remove elements
                case st_api_call::CALL_ST_NUM_ELEMENTS:
                {
                    code = st_num_elements(pcxt, call_args->payload.num_elements_args.num_elements);
                    break;
                }
	            case st_api_call::CALL_ST_ADD_ELEMENT:
                {
                    code = st_add_element(pcxt,
                                          call_args->payload.add_element_args.args,
                                          call_args->payload.add_element_args.opt_id,
                                          call_args->payload.add_element_args.geom,
                                          call_args->payload.add_element_args.element_id);
                    break;
                }
	            case st_api_call::CALL_ST_DELETE_ELEMENT:
                {
                    code = st_delete_element(pcxt, call_args->payload.delete_element_args.idx);
                    break;
                }
	            case st_api_call::CALL_ST_CLEAR_ELEMENTS:
                {
                    code = st_clear_elements(pcxt);
                    break;
                }
	            // functions to modify elements
	            case st_api_call::CALL_ST_ELEMENT_ENABLED:
                {
                    code = st_element_enabled(pcxt,
                                              call_args->payload.element_enabled_args.idx,
                                              call_args->payload.element_enabled_args.enabled_flag);
                    break;
                }
	            case st_api_call::CALL_ST_ELEMENT_VIRTUAL:
                {
                    code = st_element_virtual(pcxt,
                                              call_args->payload.element_virtual_args.idx,
                                              call_args->payload.element_virtual_args.virtual_flag);
                    break;
                }
	            case st_api_call::CALL_ST_ELEMENT_XYZ:
                {
                    code = st_element_xyz(pcxt,
                                          call_args->payload.element_xyz_args.idx,
                                          call_args->payload.element_xyz_args.x,
                                          call_args->payload.element_xyz_args.y,
                                          call_args->payload.element_xyz_args.z);
                    break;
                }
	            case st_api_call::CALL_ST_ELEMENT_AIM:
                {
                    code = st_element_aim(pcxt,
                                          call_args->payload.element_aim_args.idx,
                                          call_args->payload.element_aim_args.ax,
                                          call_args->payload.element_aim_args.ay,
                                          call_args->payload.element_aim_args.az);
                    break;
                }
	            case st_api_call::CALL_ST_ELEMENT_ZROT:
                {
                    code = st_element_zrot(pcxt,
                                           call_args->payload.element_zrot_args.idx,
                                           call_args->payload.element_zrot_args.zrot);
                    break;
                }
	            case st_api_call::CALL_ST_ELEMENT_APERTURE:
                {
                    code = st_element_aperture(pcxt,
                                               call_args->payload.element_aperture_args.idx,
                                               call_args->payload.element_aperture_args.ap,
                                               call_args->payload.element_aperture_args.params);
                    break;
                }
	            case st_api_call::CALL_ST_ELEMENT_SURFACE:
                {
                    code = st_element_surface(pcxt,
                                              call_args->payload.element_surface_args.idx,
                                              call_args->payload.element_surface_args.surf,
                                              call_args->payload.element_surface_args.params);
                    break;
                }
	            case st_api_call::CALL_ST_ELEMENT_GEOMETRY:
                {
                    code = st_element_geometry(pcxt,
                                               call_args->payload.element_geometry_args.idx,
                                               call_args->payload.element_geometry_args.geom);
                    break;
                }
	            case st_api_call::CALL_ST_ELEMENT_OPTIC:
                {
                    code = st_element_optic(pcxt,
                                            call_args->payload.element_optic_args.idx,
                                            call_args->payload.element_optic_args.opt_id);
                    break;
                }
	            case st_api_call::CALL_ST_ELEMENT_GROUP:
                {
                    code = st_element_group(pcxt,
                                            call_args->payload.element_group_args.idx,
                                            call_args->payload.element_group_args.group);
                    break;
                }
                // sun functions
                case st_api_call::CALL_ST_ADD_SUN:
                {
                    code = st_add_sun(pcxt,
                                      call_args->payload.add_sun_args.args,
                                      call_args->payload.add_sun_args.angle,
                                      call_args->payload.add_sun_args.intensity);
                    break;
                }
                case st_api_call::CALL_ST_SUN_SHAPE:
                {
                    code = st_sun_shape(pcxt,
                                        call_args->payload.sun_shape_args.shape,
                                        call_args->payload.sun_shape_args.sigma_halfwidth_csr);
                    break;
                }
                case st_api_call::CALL_ST_SUN_XYZ:
                {
                    code = st_sun_xyz(pcxt,
                                      call_args->payload.sun_xyz_args.x,
                                      call_args->payload.sun_xyz_args.y,
                                      call_args->payload.sun_xyz_args.z);
                    break;
                }
                case st_api_call::CALL_ST_SUN_POSITION:
                {
                    code = st_sun_position(pcxt,
                                           call_args->payload.sun_position_args.lat,
                                           call_args->payload.sun_position_args.day,
                                           call_args->payload.sun_position_args.hour,
                                           call_args->payload.sun_position_args.x,
                                           call_args->payload.sun_position_args.y,
                                           call_args->payload.sun_position_args.z);
                    break;
                }
                case st_api_call::CALL_ST_SUN_USERDATA:
                {
                    code = st_sun_userdata(pcxt,
                                           call_args->payload.sun_userdata_args.npoints,
                                           call_args->payload.sun_userdata_args.angle,
                                           call_args->payload.sun_userdata_args.intensity);
                    break;
                }
                case st_api_call::CALL_ST_GET_SUN_AZ_ZEN:
                {
                    code = st_get_sun_az_zen(pcxt,
                                             call_args->payload.get_sun_az_zen_args.calc,
                                             call_args->payload.get_sun_az_zen_args.loc,
                                             call_args->payload.get_sun_az_zen_args.dt,
                                             call_args->payload.get_sun_az_zen_args.azimuth,
                                             call_args->payload.get_sun_az_zen_args.zenith);
                    break;
                }
                case st_api_call::CALL_ST_GET_SUN_AZ_EL:
                { 
                    code = st_get_sun_az_el(pcxt,
                                            call_args->payload.get_sun_az_el_args.calc,
                                            call_args->payload.get_sun_az_el_args.loc,
                                            call_args->payload.get_sun_az_el_args.dt,
                                            call_args->payload.get_sun_az_el_args.azimuth,
                                            call_args->payload.get_sun_az_el_args.elevation);
                    break;
                }
                case st_api_call::CALL_ST_GET_SUN_VECTOR:
                {
                    code = st_get_sun_vector(pcxt,
                                             call_args->payload.get_sun_vector_args.calc,
                                             call_args->payload.get_sun_vector_args.loc,
                                             call_args->payload.get_sun_vector_args.dt,
                                             call_args->payload.get_sun_vector_args.sun_x,
                                             call_args->payload.get_sun_vector_args.sun_y,
                                             call_args->payload.get_sun_vector_args.sun_z);
                    break;
                }
                // functions for writing input files for SolTrace
                case st_api_call::CALL_ST_EXPORT_JSON_FILE:
                {
                    code = st_export_json_file(pcxt, call_args->payload.export_json_file_args.filename);
                    break;
                }
                // Simlulation Runner Functions
                case st_api_call::CALL_ST_SIM_SETUP:
                {
                    code = st_sim_setup(pcxt, 
                                        call_args->payload.sim_setup_args.runner_type,
                                        call_args->payload.sim_setup_args.num_threads,
                                        call_args->payload.sim_setup_args.seeds,
                                        call_args->payload.sim_setup_args.num_seeds);
                    break;
                }
                case st_api_call::CALL_ST_SIM_RUN_V2:
                {
                    code = st_sim_run_v2(pcxt);
                    break;
                }
                case st_api_call::CALL_ST_SIM_REPORT:
                {
                    code = st_sim_report(pcxt, call_args->payload.sim_report_args.level);
                    break;
                }
                // Simlulation Results Functions
                case st_api_call::CALL_ST_WRITE_RESULTS_CSV:
                {
                    code = st_write_results_csv(pcxt,
                                                call_args->payload.write_results_csv_args.filename,
                                                call_args->payload.write_results_csv_args.precision);
                    break;
                }
                // functions to get results directly
                case st_api_call::CALL_ST_NUM_INTERSECTIONS:
                {
                    code = st_num_intersections(pcxt,
                                                call_args->payload.num_intersections_args.num_intersections);
                    break;
                }
                case st_api_call::CALL_ST_LOCATIONS:
                {
                    code = st_locations(pcxt,
                                        call_args->payload.locations_args.loc_x,
                                        call_args->payload.locations_args.loc_y,
                                        call_args->payload.locations_args.loc_z);
                    break;
                }
                case st_api_call::CALL_ST_COSINES:
                {
                    code = st_cosines(pcxt,
                                      call_args->payload.cosines_args.cos_x,
                                      call_args->payload.cosines_args.cos_y,
                                      call_args->payload.cosines_args.cos_z);
                    break;
                }
                case st_api_call::CALL_ST_ELEMENTMAP:
                {
                    code = st_elementmap(pcxt,
                                         call_args->payload.elementmap_args.element_map);
                    break;
                }
                case st_api_call::CALL_ST_STAGEMAP:
                {
                    code = st_stagemap(pcxt,
                                       call_args->payload.stagemap_args.stage_map);
                    break;
                }
                case st_api_call::CALL_ST_RAYNUMBERS:
                {
                    code = st_raynumbers(pcxt,
                                         call_args->payload.raynumbers_args.ray_numbers);
                    break;
                }
                case st_api_call::CALL_ST_SUN_STATS:
                {
                    code = st_sun_stats(pcxt,
                                        call_args->payload.sun_stats_args.width,
                                        call_args->payload.sun_stats_args.height,
                                        call_args->payload.sun_stats_args.area,
                                        call_args->payload.sun_stats_args.nsunrays);
                    break;
                }
                case st_api_call::CALL_ST_GET_RESULTS_DATA:
                {
                    code = st_get_results_data(pcxt,
                                               call_args->payload.get_results_data_args.data);
                    break;
                }
                default:
                    code = st_return_code::UKNOWN_BATCH_API_CALL_FAILURE;
                    break;
            }

            if (is_error(code))
            {
                *fail_iteration = i;
                break;
            }
        }
    } catch (const std::exception& e) {
        if (cxt->p_cb) cxt->p_cb("st_batch", e.what());
    }
    return code;
}
