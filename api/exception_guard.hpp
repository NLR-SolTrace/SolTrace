// stapi_guard.hpp -- PRIVATE C++ header. Guarantee: no C++ exception crosses the C boundary.
// Review (API-1): "catch std::exception and ... in every exported function, ideally through
// one wrapper or macro so the guarantee is structural, report through the callback, and return
// RUNNER_SETUP_ERROR or EXCEPTION."
#pragma once
#include <exception>
#include "stapi_v2.h"

namespace st_guard {

// Reporting must itself never throw: a throwing callback would defeat the guard.
inline void report(p_callback cb, const char* where, const char* what) noexcept
{
    if (!cb) return;
    try { cb(where, what); } catch (...) {}
}

template <class F>
st_return_t guarded(p_callback cb, const char* where, F&& body) noexcept
{
    try {
        return body();
    } catch (const std::bad_alloc&) {
        report(cb, where, "out of memory");
        return st_return_code::BAD_ALLOCATION;
    } catch (const std::exception& e) {
        report(cb, where, e.what());
        return st_return_code::EXCEPTION;
    } catch (...) {
        report(cb, where, "unknown C++ exception");
        return st_return_code::UKNOWN_CPP_EXCEPTION;
    }
}

} // namespace st_guard

// Usage:
//   extern "C" st_return_t st_sim_setup(st_context_v2_t cxt, args_setup* a) noexcept {
//       return st_guard::guarded(get_cb(cxt), "st_sim_setup", [&]() -> st_return_t {
//           ... existing body; free to throw ...
//       });
//   }
// `noexcept` on the exported function is deliberate: if a path ever bypasses the guard,
// the failure is a deterministic terminate in a test, not undefined behaviour in a host.
