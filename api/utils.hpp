#pragma once

#include "stapi_v2.h"

namespace st_utils {

inline p_callback get_cb(st_context_v2_t pcxt)
{
    st_context *cxt = reinterpret_cast<st_context*>(pcxt);
    return cxt->p_cb;
}

inline const OpticalPropertySetReference get_optics_ref_by_id(SimulationData *data, int_fast64_t opt_id)
{
    optical_set_ptr existing_set;
    for (auto it = data->get_optics_iterator(); !data->is_optics_at_end(it); ++it)
    {
        if (it->first == opt_id)
            existing_set = it->second;
    }

    if (!existing_set) {
        OpticalPropertySetReference ref;
        ref.id = SolTrace::Data::OPTICS_ID_TYPES::OPTICS_ID_UNASSIGNED;
        return ref;
    }

    return data->find_or_add_optical_property_set(*existing_set);
}

} // namespace st_utils