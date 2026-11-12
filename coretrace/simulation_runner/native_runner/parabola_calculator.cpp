
#include "parabola_calculator.hpp"

#include <cassert>
#include <cmath>
#include <memory>
#include <stdexcept>
#include <sstream>

#include "simulation_data_export.hpp"
#include "surface.hpp"
// #include "glm::dvec3.hpp"

namespace SolTrace::NativeRunner
{

    ParabolaCalculator::ParabolaCalculator(surface_ptr surf, aperture_ptr ap)
        : aper(ap)
    {
        if (surf == nullptr)
        {
            throw std::invalid_argument("ParabolaCalculator: Surface pointer cannot be null");
        }

        auto para = std::dynamic_pointer_cast<Parabola>(surf);
        if (para == nullptr)
        {
            throw std::invalid_argument("ParabolaCalculator: Surface must be of type Parabola");
        }

        if (ap == nullptr)
        {
            throw std::invalid_argument("ParabolaCalculator: Aperture pointer cannot be null");
        }

        // Curvatures are validated by Parabola (finite, not both zero)
        this->cx = para->vertex_curv_x;
        this->cy = para->vertex_curv_y;

        // std::cout << "cx: " << cx
        //           << "\ncy: " << cy
        //           << std::endl;

        return;
    }

    ParabolaCalculator::~ParabolaCalculator()
    {
    }

    int ParabolaCalculator::intersect(const glm::dvec3 PosLoc,
                                      const glm::dvec3 CosLoc,
                                      glm::dvec3& PosXYZ,
                                      glm::dvec3& CosKLM,
                                      glm::dvec3& DFXYZ,
                                      double *PathLength)
    {
        int sts = 0;

        // std::cout << "Computing parabola intersection" << std::endl;

        double x0 = PosLoc.x, y0 = PosLoc.y, z0 = PosLoc.z;
        double mx = CosLoc.x, my = CosLoc.y, mz = CosLoc.z;
        double cx = this->cx, cy = this->cy;
        double t1, t2;
        double a, b, c;
        glm::dvec3 p1, p2;

        c = 0.5 * (cx * x0 * x0 + cy * y0 * y0) - z0;
        b = (x0 * cx * mx + y0 * cy * my - mz);
        a = 0.5 * (cx * mx * mx + cy * my * my);

        PosXYZ = {};
        CosKLM = {};
        DFXYZ = {};

        // std::cout << "a: " << a
        //           << " b: " << b
        //           << " c: " << c
        //           << std::endl;

        if (fabs(a) < 1e-12)
        {
            // This should only happen if mx == my == 0.0
            t1 = -c / b;
            p1 = PosLoc + t1 * CosLoc;

            if (t1 > 0.0 && this->aper->is_in(p1.x, p1.y)) {
                *PathLength = t1;
                // SetVec3(PosXYZ, x0 + t1 * mx, y0 + t1 * my, z0 + t1 * mz);
                // AddVec3(1.0, PosLoc, t1, CosLoc, PosXYZ);
                PosXYZ = p1;
            } else {
                // Intersection is behind the ray -- same as
                // no solution.
                sts = 1;
                *PathLength = 0.0;
            }
        }
        else
        {
            double scratch = b * b - 4.0 * a * c;
            if (scratch < 0.0)
            {
                // Only imaginary solutions so the ray does
                // NOT intersect the surface
                sts = 1;
                *PathLength = 0.0;
            }
            else
            {
                // TODO: In quadricsurfaceclosedform.cpp checks for
                // PosXYZ[2] > Element->ZAperture. Do we need to do
                // this check here?

                scratch = sqrt(scratch);
                // Negative root
                t1 = -0.5 * (b + scratch) / a;
                // Positive root
                t2 = -0.5 * (b - scratch) / a;

                p1 = PosLoc + t1 * CosLoc;
                p2 = PosLoc + t2 * CosLoc;

                // std::cout << "P1: " << p1
                //           << "\nP2: " << p2
                //           << std::endl;

                if (t1 > 0.0 && this->aper->is_in(p1.x, p1.y)) {
                    *PathLength = t1;
                    // SetVec3(PosXYZ, x0 + t1 * mx, y0 + t1 * my, z0 + t1 * mz);
                    // AddVec3(1.0, PosLoc, t1, CosLoc, PosXYZ);
                    PosXYZ = p1;
                } else if (t2 > 0.0 && this->aper->is_in(p2.x, p2.y)) {
                    *PathLength = t2;
                    // SetVec3(PosXYZ, x0 + t2 * mx, y0 + t2 * my, z0 + t2 * mz);
                    // AddVec3(1.0, PosLoc, t2, CosLoc, PosXYZ);
                    PosXYZ = p2;
                } else {
                    sts = 1;
                    *PathLength = 0.0;
                }
            }
        }

        if (sts == 0)
        {
            this->surface_normal(PosXYZ, DFXYZ);
            CosKLM = CosLoc;
        }

        // glm::dvec3 grad(DFXYZ);
        // glm::dvec3 pos(PosXYZ);
        // std::cout << "Surface normal: " << grad
        //           << "\nPosition: " << pos
        //           << std::endl;

        // std::cout << "Exiting with status: " << sts << std::endl;

        return sts;
    }

    void ParabolaCalculator::surface_normal(const glm::dvec3 PosXYZ,
                                            glm::dvec3& DFXYZ)
    {
        // TODO: Need to default to returning the surface normal of
        // whatever is the "front". Is that the inside of the parabola
        // (which is used currently) or the outside?
        double cx = this->cx;
        double cy = this->cy;
        DFXYZ = {-cx * PosXYZ.x, -cy * PosXYZ.y, 1.0};
        return;
    }

    double ParabolaCalculator::compute_z_aperture(aperture_ptr ap)
    {
        // z(x,y) = 1/2 * (cx * x^2 + cy * y^2)
        double r = ap->radius_circumscribed_circle();
        double c = std::max(this->cx, this->cy);
        double zmax = 0.5 * c * r * r;
        return zmax;
    }

} // namespace SolTrace::NativeRunner
