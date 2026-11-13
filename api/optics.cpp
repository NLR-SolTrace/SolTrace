#include "exception_guard.hpp"
#include "utils.hpp"

////////////////////////////////////////////////
// functions to add/remove optical properties //
////////////////////////////////////////////////

STAPI_V2 st_return_t st_num_optics(st_context_v2_t pcxt, uint_fast64_t *num_optics) noexcept
{
return st_guard::guarded(st_utils::get_cb(pcxt), "st_num_optics", [&]() -> st_return_t {
    CONTEXT(pcxt);
    DATA(cxt);

    *num_optics = data->get_number_of_optocal_property_sets();
    return st_return_code::SUCCESS;
});
}

STAPI_V2 st_return_t st_add_optical_properties_set(st_context_v2_t 				pcxt, 
												   args_optical_properties_set 	*opt_set,
												   args_optical_properties_face *front, 
												   args_optical_properties_face *back, 
												   uint_fast64_t 				*optic_id) noexcept
{
return st_guard::guarded(st_utils::get_cb(pcxt), "st_add_optical_properties_set", [&]() -> st_return_t {
    auto bad_type_char = [](char type) 
    {
        return type != 'g' && type != 'p' && type != 'f' && type != 'd';
    };

    if (bad_type_char(front->error_distribution_type) 
        || bad_type_char(back->error_distribution_type)) 
        return st_return_code::INVALID_ARGUMENTS;

    InteractionType inter_type = int_to_interaction(opt_set->type);
    OpticalPropertySet opt(inter_type,
                           opt_set->refraction_index_front,
                           opt_set->refraction_index_back,
                           std::string(opt_set->name));
    
    opt.set_properties(OpticalSide::Front,
                       char_to_distribution(front->error_distribution_type),
                       front->transmissivity,
                       front->reflectivity,
                       front->slope_error,
                       front->specularity_error);

    opt.set_properties(OpticalSide::Back,
                       char_to_distribution(back->error_distribution_type),
                       back->transmissivity,
                       back->reflectivity,
                       back->slope_error,
                       back->specularity_error);
    
    CONTEXT(pcxt);
    DATA(cxt);
    OpticalPropertySetReference res = data->add_optical_property_set(opt);

    if (res.id < 0) return st_return_code::DATA_INSERTION_FAILURE;

    *optic_id = res.id;
    return st_return_code::SUCCESS;
});
}

STAPI_V2 st_return_t st_get_optical_properties_set(st_context_v2_t 				pcxt, 
												   uint_fast64_t 				optic_id,
												   args_optical_properties_set 	*opt_set,
												   args_optical_properties_face *front, 
												   args_optical_properties_face *back) noexcept
{
return st_guard::guarded(st_utils::get_cb(pcxt), "st_get_optical_properties_set", [&]() -> st_return_t {
    CONTEXT(pcxt);
    DATA(cxt);

    // check that inputs are good
    const OpticalPropertySetReference found_set = st_utils::get_optics_ref_by_id(data, optic_id);
    if (found_set.id == SolTrace::Data::OPTICS_ID_TYPES::OPTICS_ID_UNASSIGNED)
        return st_return_code::DATA_VALUE_NOT_FOUND;

    auto set = found_set.optical_property_set.lock();
    if (!set) return st_return_code::DATA_VALUE_NOT_FOUND;

    opt_set->name = set->get_name().c_str();
    set->get_refraction_indices(opt_set->refraction_index_front, 
                                opt_set->refraction_index_back);
    opt_set->type = interaction_to_int(set->get_interaction_type());

    front->error_distribution_type = distribution_to_char(
        set->get_error_distribution(OpticalSide::Front));
    front->reflectivity = set->get_reflectivity(OpticalSide::Front);
    front->transmissivity = set->get_transmissivity(OpticalSide::Front);
    front->slope_error = set->get_slope_error(OpticalSide::Front);
    front->specularity_error = set->get_specularity_error(OpticalSide::Front);
    
    back->error_distribution_type = distribution_to_char(
        set->get_error_distribution(OpticalSide::Back));
    back->reflectivity = set->get_reflectivity(OpticalSide::Back);
    back->transmissivity = set->get_transmissivity(OpticalSide::Back);
    back->slope_error = set->get_slope_error(OpticalSide::Back);
    back->specularity_error = set->get_specularity_error(OpticalSide::Back);

    return st_return_code::SUCCESS;
});
}

STAPI_V2 st_return_t st_delete_optic(st_context_v2_t pcxt, st_uint_t idx) noexcept
{
return st_guard::guarded(st_utils::get_cb(pcxt), "st_delete_optic", [&]() -> st_return_t {
    CONTEXT(pcxt);
    DATA(cxt);

    int removed = data->remove_optical_property_set(idx);

    // only warn if requested to remove optical property set that doesn't exist
    return removed > 0 ? st_return_code::SUCCESS : st_return_code::WARNING_NOT_FOUND;
});
}

STAPI_V2 st_return_t st_clear_optics(st_context_v2_t pcxt) noexcept
{
return st_guard::guarded(st_utils::get_cb(pcxt), "st_clear_optics", [&]() -> st_return_t {
    CONTEXT(pcxt);
    DATA(cxt);

    data->clear_optical_property_sets();
    return st_return_code::SUCCESS;
});
}