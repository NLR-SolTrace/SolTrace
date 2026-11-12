#ifndef SOLTRACE_SIMULATION_DATA_API_H
#define SOLTRACE_SIMULATION_DATA_API_H

// Model, identity and creation handles
#include "element_id.hpp"
#include "model_context.hpp"
#include "model_identity.hpp"
#include "simulation_data.hpp"
#include "simulation_parameters.hpp"

// Elements
#include "composite_element.hpp"
#include "single_element.hpp"
#include "stage_element.hpp"
#include "virtual_element.hpp"

// Geometry and optics
#include "aperture.hpp"
#include "error_distributions.hpp"
#include "optical_properties.hpp"
#include "surface.hpp"

// Ray sources
#include "datetime.hpp"
#include "ray_source.hpp"
#include "sun.hpp"

// File input and output
#include "json_schema.hpp"
#include "simdata_io.hpp"

// Math and utilities
#include "constants.hpp"
#include "matvec.hpp"
#include "utilities.hpp"

#include "cst_templates/heliostat.hpp"
#include "cst_templates/linear_fresnel.hpp"
#include "cst_templates/parabolic_dish.hpp"
#include "cst_templates/parabolic_trough.hpp"

#include "solar_position_calculators/solar_position_calculator.hpp"

#endif
