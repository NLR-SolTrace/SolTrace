#include "exception_guard.hpp"
#include "utils.hpp"

///////////////////////////////////////
// functions to get results directly //
///////////////////////////////////////

uint_fast64_t number_of_interactions(SimulationResult *result)
{
    uint_fast64_t num = 0;

    for (auto it = result->get_ray_record_iterator(); !result->is_at_end(it); ++it)
        num += (*it)->get_number_of_interactions();
    
    return num;
}

STAPI_V2 st_return_t st_num_intersections(st_context_v2_t pcxt, uint_fast64_t *num_intersections) noexcept
{
return st_guard::guarded(st_utils::get_cb(pcxt), "st_num_intersections",[&]() -> st_return_t {
    CONTEXT(pcxt);
    RESULT(cxt);
    
    if (cxt->report_level == RunnerStatistics::GROUPED_COUNTS)
        return st_return_code::RESULT_NOT_REPORTED;

    *num_intersections = number_of_interactions(result);
    return st_return_code::SUCCESS;
});
}

STAPI_V2 st_return_t st_locations(st_context_v2_t pcxt,
								  double 		  *loc_x,
								  double 		  *loc_y,
								  double 		  *loc_z) noexcept
{
return st_guard::guarded(st_utils::get_cb(pcxt), "st_locations",[&]() -> st_return_t {
    /* *loc_x, *loc_y, *loc_z must be allocated by 
       caller using st_num_intersections */
    CONTEXT(pcxt);
    RESULT(cxt);
    
    if (cxt->report_level == RunnerStatistics::GROUPED_COUNTS)
        return st_return_code::RESULT_NOT_REPORTED;

    uint_fast64_t n = 0;
    glm::dvec3 loc;
    for (auto it = result->get_ray_record_iterator(); !result->is_at_end(it); ++it)
    {
        ray_record_ptr rec = *it;
        for (auto jt = rec->get_interaction_record_iterator(); !rec->is_at_end(jt); ++jt)
        {
            loc = (*jt)->location;
            loc_x[n] = loc[0];
            loc_y[n] = loc[1];
            loc_z[n] = loc[2];
            ++n;
        }
    }

    return st_return_code::SUCCESS;
});
}

STAPI_V2 st_return_t st_cosines(st_context_v2_t pcxt,
							 	double 		    *cos_x,
							 	double 		    *cos_y,
							 	double 		    *cos_z) noexcept
{
return st_guard::guarded(st_utils::get_cb(pcxt), "st_cosines",[&]() -> st_return_t {
    /* *cos_x, *cos_y, *cos_z must be allocated by 
       caller using st_num_intersections */
    CONTEXT(pcxt);
    RESULT(cxt);
    
    if (cxt->report_level == RunnerStatistics::GROUPED_COUNTS)
        return st_return_code::RESULT_NOT_REPORTED;

    uint_fast64_t n = 0;
    glm::dvec3 cosine;
    for (auto it = result->get_ray_record_iterator(); !result->is_at_end(it); ++it)
    {
        ray_record_ptr rec = *it;
        for (auto jt = rec->get_interaction_record_iterator(); !rec->is_at_end(jt); ++jt)
        {
            cosine = (*jt)->direction;
            cos_x[n] = cosine[0];
            cos_y[n] = cosine[1];
            cos_z[n] = cosine[2];
            ++n;
        }
    }

    return st_return_code::SUCCESS;
});
}

STAPI_V2 st_return_t st_elementmap(st_context_v2_t pcxt, uint_fast64_t *element_map) noexcept
{
return st_guard::guarded(st_utils::get_cb(pcxt), "st_elementmap",[&]() -> st_return_t {
    /* *element_map must be allocated by caller using st_num_intersections */
    CONTEXT(pcxt);
    RESULT(cxt);
    
    if (cxt->report_level == RunnerStatistics::GROUPED_COUNTS)
        return st_return_code::RESULT_NOT_REPORTED;

    uint_fast64_t n = 0;
    for (auto it = result->get_ray_record_iterator(); !result->is_at_end(it); ++it)
    {
        ray_record_ptr rec = *it;
        for (auto jt = rec->get_interaction_record_iterator(); !rec->is_at_end(jt); ++jt)
        {
            element_map[n] = (*jt)->element;
            ++n;
        }
    }

    return st_return_code::SUCCESS;
});
}

STAPI_V2 st_return_t st_stagemap(st_context_v2_t pcxt, uint_fast64_t *stage_map) noexcept
{
return st_guard::guarded(st_utils::get_cb(pcxt), "st_stagemap",[&]() -> st_return_t {
    /* *stage_map must be allocated by caller using st_num_intersections */
    CONTEXT(pcxt);
    RESULT(cxt);
    
    if (cxt->report_level == RunnerStatistics::GROUPED_COUNTS)
        return st_return_code::RESULT_NOT_REPORTED;

    uint_fast64_t n = number_of_interactions(result);
    // deprecating stages return array of 0s
    for (uint_fast64_t i = 0; i < n; ++i)
        stage_map[i] = 0;

    return st_return_code::SUCCESS;
});
}

STAPI_V2 st_return_t st_raynumbers(st_context_v2_t pcxt, uint_fast64_t *ray_numbers) noexcept
{
return st_guard::guarded(st_utils::get_cb(pcxt), "st_raynumbers",[&]() -> st_return_t {
    /* *ray_numbers must be allocated by caller using st_num_intersections */
    CONTEXT(pcxt);
    RESULT(cxt);
    
    if (cxt->report_level == RunnerStatistics::GROUPED_COUNTS)
        return st_return_code::RESULT_NOT_REPORTED;

    uint_fast64_t n = 0;
    for (auto it = result->get_ray_record_iterator(); !result->is_at_end(it); ++it)
    {
        ray_record_ptr rec = *it;
        for (auto jt = rec->get_interaction_record_iterator(); !rec->is_at_end(jt); ++jt)
        {
            ray_numbers[n] = rec->id;
            ++n;
        }
    }

    return st_return_code::SUCCESS;
});
}

STAPI_V2 st_return_t st_sun_stats(st_context_v2_t pcxt,
                                  double 		  *width,
								  double 		  *height,
								  double 		  *area,
								  uint_fast64_t	  *nsunrays) noexcept
{
return st_guard::guarded(st_utils::get_cb(pcxt), "st_sun_stats",[&]() -> st_return_t {
    // TODO: compute power per ray
    CONTEXT(pcxt);
    RESULT(cxt);

    *nsunrays = result->get_sun_ray_count();

    if (cxt->runner_type == st_runner_type_t::OPTIX)
    {
        // optix only sets sun area
        *area = result->get_sun_A_box();
    }
    else
    {
        result->get_sun_dimensions(*width, *height);
        *area = (*width) * (*height);
    }

    return st_return_code::SUCCESS;
});
}

STAPI_V2 st_return_t st_get_results_data(st_context_v2_t pcxt, args_results_data *data) noexcept
{
return st_guard::guarded(st_utils::get_cb(pcxt), "st_get_results_data",[&]() -> st_return_t {
    /* all arrays in data must be allocated by caller using st_num_intersections */
    CONTEXT(pcxt);
    RESULT(cxt);
    
    if (cxt->report_level == RunnerStatistics::GROUPED_COUNTS)
        return st_return_code::RESULT_NOT_REPORTED;

    uint_fast64_t n = 0;
    glm::dvec3 loc;
    glm::dvec3 cosine;
    for (auto it = result->get_ray_record_iterator(); !result->is_at_end(it); ++it)
    {
        ray_record_ptr rec = *it;
        for (auto jt = rec->get_interaction_record_iterator(); !rec->is_at_end(jt); ++jt)
        {
            loc = (*jt)->location;
            cosine = (*jt)->direction;
            
            data->loc_x[n]       = loc[0];
            data->loc_y[n]       = loc[1];
            data->loc_z[n]       = loc[2];
            data->cos_x[n]       = cosine[0];
            data->cos_y[n]       = cosine[1];
            data->cos_z[n]       = cosine[2];
            data->element_map[n] = (*jt)->element;
            data->stage_map[n]   = 0;
            data->ray_numbers[n] = rec->id;
            ++n;
        }
    }

    return st_return_code::SUCCESS;
});
}

// functions to get grouped results directly

STAPI_V2 st_return_t st_num_groups(st_context_v2_t pcxt, size_t *num_groups) noexcept
{
return st_guard::guarded(st_utils::get_cb(pcxt), "st_num_groups",[&]() -> st_return_t {
    CONTEXT(pcxt);
    RESULT(cxt);
    
    if (cxt->report_level == RunnerStatistics::RAY_RECORDS)
        return st_return_code::RESULT_NOT_REPORTED;

    *num_groups = result->get_number_of_groups();
    return st_return_code::SUCCESS;
});
}

STAPI_V2 st_return_t st_get_grouped_results_data(st_context_v2_t pcxt, args_group_result *data) noexcept
{
return st_guard::guarded(st_utils::get_cb(pcxt), "st_get_grouped_results_data",[&]() -> st_return_t {
    // data must be preallocated by caller
    CONTEXT(pcxt);
    RESULT(cxt);
    
    if (cxt->report_level == RunnerStatistics::RAY_RECORDS)
        return st_return_code::RESULT_NOT_REPORTED;

    std::vector<GroupResult> groups = result->get_grouped_results();
    for (size_t i = 0; i < groups.size(); ++i)
    {
        data[i].group_id              = groups[i].group_id;
        data[i].absorb_count          = groups[i].absorb_count;
        data[i].reflect_count         = groups[i].reflect_count;
        data[i].transmit_count        = groups[i].transmit_count;
        data[i].virtual_count         = groups[i].virtual_count;
        data[i].absorb_sun_previous   = groups[i].absorb_sun_previous;
        data[i].reflect_sun_previous  = groups[i].reflect_sun_previous;
        data[i].transmit_sun_previous = groups[i].transmit_sun_previous;
        data[i].virtual_sun_previous  = groups[i].virtual_sun_previous;

        // same number of elements in _previous_group vectors as number of groups
        for (size_t j = 0; j < groups.size(); ++j)
        {
            data[i].absorb_previous_group[j]   = groups[i].absorb_previous_group[j];
            data[i].reflect_previous_group[j]  = groups[i].reflect_previous_group[j];
            data[i].transmit_previous_group[j] = groups[i].transmit_previous_group[j];
            data[i].virtual_previous_group[j]  = groups[i].virtual_previous_group[j];
        }
    }

    return st_return_code::SUCCESS;
});
}