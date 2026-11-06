#pragma once

#include <QImage>
#include <QObject>
#include <QUuid>

#include "baked_flux_map.h"
#include "data/mesh.h"
#include "jobs/job_run_common.h"
#include "support/qt_helpers.h"

namespace SolTrace::GUI::Analysis {

/// Options for rasterizing ray/surface hits into a flux-map image.
struct FluxMapBakeOptions {
    // TODO: change to support UV aspect ratio
    glm::uvec2 image_resolution = { 1024, 1024 };

    /// The color of grid lines (ie, UV grids). If null, does not draw lines.
    QColor grid_line_color = QColor();

    /// The color map to use. If null, uses a classic blue to red map.
    QImage color_map;

    /// Stable color-map identity retained with the baked map and metadata.
    QString color_map_id     = "classic";
    QString color_map_name   = "Classic Blue–Red";
    QString color_map_source = "qrc:/assets/images/b_to_r_wide.png";

    /// Solar direct normal irradiance in W/m^2.
    double dni = 1000.0;

    /// Radius of the image-space Gaussian smoothing kernel, in flux bins.
    unsigned smoothing_radius = 2;

    /// Treat the corresponding UV axis as periodic while splatting and
    /// smoothing. This joins the duplicated UV edges of wrapped surfaces.
    bool wrap_u = false;
    bool wrap_v = false;

    /// Surface tessellation multiplier used to prepare the projection mesh.
    unsigned surface_detail = 1;

    /// Include hits that occur before any prior physical ray interaction.
    bool include_direct_sun_rays = true;

    /// Include only absorbing interactions at the mapped element.
    bool terminal_interactions_only = false;
};

/// Uniquely identifies one computation, including the result it belongs to.
struct FluxMapRequestId {
    QUuid                       result_id;
    SolTrace::GUI::Data::Entity entity;
    quint64                     revision = 0;

    bool operator==(FluxMapRequestId const&) const = default;
};

inline QDebug operator<<(QDebug debug, FluxMapRequestId const& request) {
    QDebugStateSaver saver(debug);
    debug.nospace() << "FluxMapRequest(" << request.result_id << ", "
                    << request.entity << ", revision " << request.revision
                    << ')';
    return debug;
}

/// Asynchronous flux-map generator for simulation results.
///
/// The input mesh is expected to be a UV-unwrapped receiver surface. Generated
/// maps accumulate ray energy directly in UV image space and use the mesh to
/// determine the physical surface area represented by each pixel.
class FluxMapComputer : public QObject {
    Q_OBJECT

    SolTrace::GUI::Data::SimulationResultPtr m_database;

public:
    /// Create a flux-map computer with no attached result set.
    explicit FluxMapComputer(QObject* parent);
    ~FluxMapComputer() override;

    /// Set the current simulation results
    void set_results(SolTrace::GUI::Data::SimulationResultPtr);

public slots:
    /// Start generating a flux map for the given entity, its mesh, and options.
    /// The mesh MUST NOT have overlapping UVs.
    bool
    start_generate_for(SolTrace::GUI::Analysis::FluxMapRequestId,
                       SolTrace::GUI::Data::Mesh                   mesh,
                       SolTrace::GUI::Analysis::FluxMapBakeOptions options);

signals:
    /// Issued when the computation has completed.
    void image_ready(SolTrace::GUI::Analysis::FluxMapRequestId,
                     SolTrace::GUI::Analysis::BakedFluxMapPtr);

    /// Issued when the computation has failed.
    void image_failed(SolTrace::GUI::Analysis::FluxMapRequestId,
                      QString reason);

    /// Issued as the map is being generated, 0-100.
    void image_progress(SolTrace::GUI::Analysis::FluxMapRequestId, int);

    /// Issued (or can be issued externally) to cancel all map generation.
    void cancel_all();

    /// Issued (or can be issued externally) to cancel a specific map gen.
    void cancel_specific(SolTrace::GUI::Data::Entity);
};

} // namespace SolTrace::GUI::Analysis

Q_DECLARE_METATYPE(SolTrace::GUI::Analysis::FluxMapRequestId)
