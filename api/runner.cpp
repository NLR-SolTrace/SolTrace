#include "exception_guard.hpp"
#include "utils.hpp"

//////////////////////////////////
// Simlulation Runner Functions //
//////////////////////////////////

// functions for SolTrace runner management
STAPI_V2 st_return_t st_get_installed_runners(uint8_t *installed) noexcept
{
return st_guard::guarded(nullptr, "st_get_installed_runners", [&]() -> st_return_t {
    *installed = 1 << st_runner_type_t::NATIVE;
#ifdef STAPI_V2_EMBREE_SUPPORT
    *installed += 1 << st_runner_type_t::EMBREE;
#endif
#ifdef STAPI_V2_OPTIX_SUPPORT
    *installed += 1 << st_runner_type_t::OPTIX;
#endif
    return st_return_code::SUCCESS;
});
}

STAPI_V2 st_return_t st_is_runner_installed(st_runner_type_t type,
                                            bool             *installed) noexcept
{
return st_guard::guarded(nullptr, "st_is_runner_installed", [&]() -> st_return_t {
    uint8_t runners;
    st_get_installed_runners(&runners);
    *installed = runners & (1 << type);
    return st_return_code::SUCCESS;
});
}

STAPI_V2 st_return_t st_sim_setup(st_context_v2_t  pcxt, 
								  st_runner_type_t runner_type, 
								  uint_fast64_t    num_threads,
								  st_uint_t 	   *seeds,
								  size_t		   num_seeds) noexcept
{
return st_guard::guarded(st_utils::get_cb(pcxt), "st_sim_setup", [&]() -> st_return_t {
    CONTEXT(pcxt);
    DATA(cxt);

    delete cxt->p_runner;
    delete cxt->p_results;
    cxt->p_runner  = nullptr;
    cxt->p_results = nullptr;

    bool use_embree = runner_type == st_runner_type_t::EMBREE;
    bool use_optix = runner_type == st_runner_type_t::OPTIX;

    RunnerStatus sts;
    st_return_t  rt;
    rt = st_return_code::SUCCESS;
    SimulationRunner *runner;

#ifdef STAPI_V2_EMBREE_SUPPORT
    if (use_embree)
    {
        runner = new SolTrace::EmbreeRunner::EmbreeRunner();
    }
    else
#endif
#ifdef STAPI_V2_OPTIX_SUPPORT
        if (use_optix)
    {
        runner = new OptixRunner();
    }
    else
#endif
    {
#ifndef STAPI_V2_EMBREE_SUPPORT
        if (use_embree) 
        {
            rt = st_return_code::WARNING_FELLBACK_FROM_EMBREE;
        }
#endif
#ifndef STAPI_V2_OPTIX_SUPPORT
        if (use_optix) 
        {
            rt = st_return_code::WARNING_FELLBACK_FROM_OPTIX;
        }
#endif
        runner_type = st_runner_type_t::NATIVE;
        runner = new NativeRunner();
    }

    sts = runner->initialize();
    if (sts != RunnerStatus::SUCCESS)
    {
        delete runner;
        return st_return_code::RUNNER_INILIALIZE_FAILURE;
    }

    /* optix doesn't use threads the way native/embree does, 
       and check if user requested optix but didn't build it,
       in either case, set the number of threads for the runner */
    if (!use_optix || rt == st_return_code::WARNING_FELLBACK_FROM_OPTIX)
    {
        NativeRunner *temp_native = dynamic_cast<NativeRunner*>(runner);
        if (seeds != nullptr)
        {
            // ensures runtime_error won't be raised
            if (num_threads != num_seeds)
            {
                delete runner;
                return st_return_code::RUNNER_NUMBER_THREADS_SEEDS_MISMATCH_FAILURE;
            }

            const std::vector<st_uint_t> temp_seeds(seeds, seeds + num_seeds);
            temp_native->set_number_of_threads(num_threads, temp_seeds);
        }
        else
        {
            temp_native->set_number_of_threads(num_threads);
        }
        // by default disable stages
        temp_native->disable_stages();
    }
    // otherwise using optix runner
    else {
        // if context is located, add for ptx discovery, include ifdef guard b/c need to access OptixRunner
#ifdef STAPI_V2_OPTIX_SUPPORT
        if (!cxt->location.empty())
        {
            OptixRunner *temp_optix = dynamic_cast<OptixRunner*>(runner);
            temp_optix->set_additional_ptx_directory(cxt->location);
        }
#endif
        // if using optix runner and requested threads, emit warning that it was ignored 
        if (num_threads != DEFAULT_NUM_THREADS) rt = st_return_code::WARNING_ARGUMENT_IGNORED_BY_RUNNER;
    }

    // auto t_setup_start = std::chrono::steady_clock::now();
    
    try { sts = runner->setup_simulation(data); }
    catch (std::runtime_error &e)
    {
        if (cxt->p_cb) cxt->p_cb("set up runner", e.what());
        delete runner;
        return st_return_code::RUNNER_SETUP_ERROR;
    }
    
    // auto t_setup_end = std::chrono::steady_clock::now();
    if (sts != RunnerStatus::SUCCESS)
    {
        delete runner;
        return st_return_code::RUNNER_SETUP_FAILURE;
    }
    
    cxt->runner_type = runner_type;
    cxt->p_runner = runner;

    return rt;
});
}

STAPI_V2 st_return_t st_sim_run_v2(st_context_v2_t pcxt) noexcept
{
return st_guard::guarded(st_utils::get_cb(pcxt), "st_sim_run_v2", [&]() -> st_return_t {
    CONTEXT(pcxt);
    RUNNER(cxt);

    if (!runner->is_ready_to_run()) return st_return_code::RUNNER_NOT_READY_TO_RUN;

    RunnerStatus sts = runner->run_simulation();
    
    if (sts == RunnerStatus::CANCEL) return st_return_code::CANCEL;
    else if (sts == RunnerStatus::MAXRAYS) return st_return_code::WARNING_MAX_RAYS;
    else if (sts != RunnerStatus::SUCCESS) return st_return_code::FAILURE;
    return st_return_code::SUCCESS;
});
}

STAPI_V2 st_return_t st_sim_report(st_context_v2_t pcxt, int level) noexcept
{
return st_guard::guarded(st_utils::get_cb(pcxt), "st_sim_report", [&]() -> st_return_t {
    if (level < RunnerStatistics::RAY_RECORDS ||
        level >= RunnerStatistics::STATISTICS_COUNT)
        return st_return_code::INVALID_ARGUMENTS;
        
    CONTEXT(pcxt);
    RUNNER(cxt);

    if (!runner->is_ready_to_report()) return st_return_code::RUNNER_NOT_READY_TO_REPORT;

    cxt->report_level = (RunnerStatistics)level;
    delete cxt->p_results;
    cxt->p_results = new SimulationResult();
    RunnerStatus sts = runner->report_simulation(cxt->p_results, level);

    if (sts != RunnerStatus::SUCCESS) return st_return_code::FAILURE;
    return st_return_code::SUCCESS;
});
}
