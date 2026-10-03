#include "determine_interaction_type.hpp"

#include <sstream>

#include <simulation_data_export.hpp>

#include "mtrand.hpp"
#include "native_runner_types.hpp"
#include "trace_logger.hpp"

namespace SolTrace::NativeRunner
{
    using SolTrace::Result::RayEvent;

    bool determine_interaction_type(trace_logger_ptr logger,
                                    int_fast64_t stage,
                                    unsigned thread_id,
                                    MTRand &myrng,
                                    const OpticalPropertySet* optics,
                                    const glm::dvec3 &LastDFXYZ,
                                    const glm::dvec3 &LastCosRaySurfElement,
                                    bool LastHitBackSide,
                                    RayEvent &rev)
    {
        bool good = true;
        rev = RayEvent::VIRTUAL;

        double TestValue;
        auto UnitLastDFXYZ = glm::dvec3{0.0};
        double IncidentAngle = 0;

        const OpticalSide side = LastHitBackSide == false ? OpticalSide::Front : OpticalSide::Back;

        // TODO: Implement tables...
        switch (optics->get_interaction_type())
        {
        case InteractionType::REFRACTION:
            // if (optics->UseTransmissivityTable)
            // {
            //     int npoints = optics->TransmissivityTable.size();
            //     int m = 0;

            //     UnitLastDFXYZ[0] = -LastDFXYZ[0] / sqrt(DOT(LastDFXYZ, LastDFXYZ));
            //     UnitLastDFXYZ[1] = -LastDFXYZ[1] / sqrt(DOT(LastDFXYZ, LastDFXYZ));
            //     UnitLastDFXYZ[2] = -LastDFXYZ[2] / sqrt(DOT(LastDFXYZ, LastDFXYZ));
            //     IncidentAngle = acos(DOT(LastCosRaySurfElement, UnitLastDFXYZ)) * 1000.; //[mrad]
            //     if (IncidentAngle >= optics->TransmissivityTable[npoints - 1].angle)
            //     {
            //         TestValue = optics->TransmissivityTable[npoints - 1].trans;
            //     }
            //     else
            //     {
            //         while (optics->TransmissivityTable[m].angle < IncidentAngle)
            //             m++;

            //         if (m == 0)
            //             TestValue = optics->TransmissivityTable[m].trans;
            //         else
            //             TestValue = (optics->TransmissivityTable[m].trans + optics->TransmissivityTable[m - 1].trans) / 2.0;
            //     }
            // }
            // else
            // {
            //     TestValue = optics->transmitivity;
            //     rev = RayEvent::TRANSMIT;
            // }
            TestValue = optics->get_transmissivity(side);
            rev = RayEvent::TRANSMIT;
            break;
        case InteractionType::REFLECTION:
            // if (optics->UseReflectivityTable)
            // {
            //     int npoints = optics->ReflectivityTable.size();
            //     int m = 0;
            //     UnitLastDFXYZ[0] = -LastDFXYZ[0] / sqrt(DOT(LastDFXYZ, LastDFXYZ));
            //     UnitLastDFXYZ[1] = -LastDFXYZ[1] / sqrt(DOT(LastDFXYZ, LastDFXYZ));
            //     UnitLastDFXYZ[2] = -LastDFXYZ[2] / sqrt(DOT(LastDFXYZ, LastDFXYZ));
            //     IncidentAngle = acos(DOT(LastCosRaySurfElement, UnitLastDFXYZ)) * 1000.; //[mrad]
            //     if (IncidentAngle >= optics->ReflectivityTable[npoints - 1].angle)
            //     {
            //         TestValue = optics->ReflectivityTable[npoints - 1].refl;
            //     }
            //     else
            //     {
            //         while (optics->ReflectivityTable[m].angle < IncidentAngle)
            //             m++;

            //         if (m == 0)
            //             TestValue = optics->ReflectivityTable[m].refl;
            //         else
            //             TestValue = (optics->ReflectivityTable[m].refl + optics->ReflectivityTable[m - 1].refl) / 2.0;
            //     }
            // }
            // else
            // {
            //     TestValue = optics->reflectivity;
            //     rev = RayEvent::REFLECT;
            // }
            TestValue = optics->get_reflectivity(side);
            rev = RayEvent::REFLECT;
            break;
        default:
            good = false;
            std::stringstream ss;
            ss << "Bad optical interaction."
               << " Type: " << static_cast<int>(optics->get_interaction_type())
               << " Stage: " << stage
               << " Thread: " << thread_id
               << "\n";
            logger->error_log(ss.str());
            return false;
        }

        // Apply MonteCarlo probability of absorption. Limited
        // for now, but can make more complex later on if desired
        if (TestValue <= myrng())
        {
            // ray was fully absorbed
            rev = RayEvent::ABSORB;
        }

        return good;
    }

} // namespace SolTrace::NativeRunner
