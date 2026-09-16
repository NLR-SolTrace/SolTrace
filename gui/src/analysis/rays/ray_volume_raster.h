#pragma once

#include "data/simulationresult.h"
#include "support/asynctask.h"
#include "support/grid3d.h"

#include <glm/glm.hpp>
#include <glm/vec3.hpp>

namespace SolTrace::GUI::Analysis {

/// Rasterize ray paths from a result set into a sparse 3D volume.
///
/// This is currently a work-in-progress analysis primitive used by the flux
/// module for volume visualization.
Result<SolTrace::GUI::Analysis::SparseGrid3D<float>, QString>
compute_ray_volume_raster(TaskControl&            promise,
                          unsigned                resolution,
                          SolTrace::GUI::Data::SimulationResultPtr results);

} // namespace SolTrace::GUI::Analysis
