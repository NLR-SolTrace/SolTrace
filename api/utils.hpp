#pragma once

#include "stapi_v2.h"

#include "../coretrace/simulation_data/simulation_data_export.hpp"
#include "../coretrace/simulation_results/simulation_result_export.hpp"
#include "../coretrace/simulation_runner/native_runner/native_runner.hpp"
#include "../coretrace/simulation_runner/simulation_runner.hpp"

#ifdef STAPI_V2_EMBREE_SUPPORT
#include "../coretrace/simulation_runner/embree_runner/embree_runner.hpp"
#endif

#ifdef STAPI_V2_OPTIX_SUPPORT
#include "../coretrace/simulation_runner/optix_runner/optix_runner.hpp"
#endif

using SolTrace::Runner::SimulationRunner;
using SolTrace::Runner::RunnerStatus;
using SolTrace::Runner::RunnerStatistics;
using SolTrace::NativeRunner::NativeRunner;
using SolTrace::Data::Aperture;
using SolTrace::Data::single_element_ptr;
using spcm = SolTrace::Data::SolarPositionCalculationMethod;

typedef struct st_context {
	SimulationData*   p_data	   = nullptr;
	st_runner_type_t  runner_type  = st_runner_type_t::RUNNER_COUNT;
	SimulationRunner* p_runner	   = nullptr;
	RunnerStatistics  report_level = RunnerStatistics::STATISTICS_COUNT;
	SimulationResult* p_results	   = nullptr;
	p_callback		  p_cb	   	   = nullptr;
	fs::path		  location;
} st_context;

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