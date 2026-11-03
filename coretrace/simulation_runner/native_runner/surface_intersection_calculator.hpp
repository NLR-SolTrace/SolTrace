#ifndef SOLTRACE_SURFACE_INTERSECTION_CALCULATOR_H
#define SOLTRACE_SURFACE_INTERSECTION_CALCULATOR_H

#include <memory>

#include <glm/vec3.hpp>

#include "aperture.hpp"

namespace SolTrace::NativeRunner {

class SurfaceIntersectionCalculator
{
public:
    SurfaceIntersectionCalculator() {}
    virtual ~SurfaceIntersectionCalculator() {}

    // TODO: Not sure what purpose CosKLM serves here. Investigate...
    virtual int intersect(const glm::dvec3 PosLoc,
                          const glm::dvec3 CosLoc,
                          glm::dvec3& PosXYZ,
                          glm::dvec3& CosKLM,
                          glm::dvec3& DFXYZ,
                          double *PathLength) = 0;

    virtual double compute_z_aperture(SolTrace::Data::aperture_ptr ap) = 0;

private:
};

using calculator_ptr = typename std::shared_ptr<SurfaceIntersectionCalculator>;

} // namespace SolTrace::NativeRunner

#endif
