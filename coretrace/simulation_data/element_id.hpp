/**
 * @file element_id.hpp
 * @brief Element identifier type and the special values it can take
 */

#ifndef SOLTRACE_ELEMENT_ID_H
#define SOLTRACE_ELEMENT_ID_H

#include <cstdint>

namespace SolTrace::Data {

using element_id = std::int_fast64_t;
const element_id ELEMENT_ERROR = -1;
const element_id ELEMENT_ID_UNASSIGNED = -2;
const element_id ELEMENT_ALREADY_REGISTERED = -3;
const element_id ELEMENT_INVALID_SETUP = -4;
const element_id ELEMENT_NULL = -5;
// A child that belongs to another model, or to a model that no longer exists
const element_id ELEMENT_WRONG_MODEL = -6;

} // namespace SolTrace::Data

#endif
