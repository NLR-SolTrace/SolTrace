#include "exception_guard.hpp"
#include "utils.hpp"

////////////////////////////////////////////////////////////////
// functions for simulation data management thru json strings //
////////////////////////////////////////////////////////////////

STAPI_V2 st_return_t st_read_input_json(st_context_v2_t pcxt, const char *json) noexcept
{
return st_guard::guarded(st_utils::get_cb(pcxt), "st_read_input_json", [&]() -> st_return_t {
    CONTEXT(pcxt);
    DATA(cxt);

    data->import_json_string(json);
    return st_return_code::SUCCESS;
});
}

STAPI_V2 st_return_t st_read_input_json_file(st_context_v2_t pcxt, const char *filename) noexcept
{
return st_guard::guarded(st_utils::get_cb(pcxt), "st_read_input_json_file", [&]() -> st_return_t {
	CONTEXT(pcxt);
    DATA(cxt);
    
    data->import_json_file(std::string(filename));
    return st_return_code::SUCCESS;
});
}

///////////////////////////////////////
// functions for writing input files //
///////////////////////////////////////

STAPI_V2 st_return_t st_export_json_file(st_context_v2_t pcxt, const char *filename) noexcept
{
return st_guard::guarded(st_utils::get_cb(pcxt), "st_export_json_file", [&]() -> st_return_t {
    CONTEXT(pcxt);
    DATA(cxt);

    data->export_json_file(std::string(filename));
    return st_return_code::SUCCESS;
});
}

//////////////////////////////////////////////
// functions for writing simulation results //
//////////////////////////////////////////////

STAPI_V2 st_return_t st_write_results_csv(st_context_v2_t pcxt, 
										  const char 	  *filename, 
										  int 			  precision) noexcept
{
return st_guard::guarded(st_utils::get_cb(pcxt), "st_write_results_csv", [&]() -> st_return_t {
    CONTEXT(pcxt);
    RESULT(cxt);

    if (cxt->report_level == RunnerStatistics::GROUPED_COUNTS)
        return st_return_code::RESULT_NOT_REPORTED;

    result->write_csv_file(filename, precision);
    return st_return_code::SUCCESS;
});
}

STAPI_V2 st_return_t st_write_group_results_json(st_context_v2_t pcxt, 
										         const char 	 *filename, 
										         int 			 precision,
                                                 int             indent) noexcept
{
return st_guard::guarded(st_utils::get_cb(pcxt), "st_write_group_results_json", [&]() -> st_return_t {
    CONTEXT(pcxt);
    RESULT(cxt);

    if (cxt->report_level == RunnerStatistics::RAY_RECORDS)
        return st_return_code::RESULT_NOT_REPORTED;

    result->write_group_json_file(filename, precision, indent);
    return st_return_code::SUCCESS;
});
}