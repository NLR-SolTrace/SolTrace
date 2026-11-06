#pragma once

#include "data/mesh.h"
#include "support/grid2d.h"

#include <QColor>
#include <QImage>
#include <QMetaType>
#include <QObject>
#include <QSize>
#include <QString>
#include <QVector3D>
#include <QtGlobal>
#include <qqmlintegration.h>

#include <vector>

namespace SolTrace::GUI::Analysis {

/// Settings and provenance retained with a generated flux map.
struct FluxMapGenerationInfo {
    QSize    image_resolution;
    unsigned smoothing_radius = 0;
    unsigned surface_detail   = 1;
    bool     wrap_u           = false;
    bool     wrap_v           = false;
    bool     show_mesh_grid   = false;
    QColor   mesh_line_color;
    double   dni                        = 1000.0;
    bool     include_direct_sun_rays    = true;
    bool     terminal_interactions_only = false;
    QString  color_map_id;
    QString  color_map_name;
    QString  color_map_source;
    QString  generated_at;
};

/// Summary statistics computed with the baked flux map.
struct BakedFluxMapStats {
    Q_GADGET
    QML_VALUE_TYPE(baked_flux_map_stats);
    Q_PROPERTY(quint64 source_ray_count MEMBER source_ray_count)
    Q_PROPERTY(quint64 plotted_ray_count MEMBER plotted_ray_count)
    Q_PROPERTY(double power_per_ray MEMBER power_per_ray)
    Q_PROPERTY(double plotted_power MEMBER plotted_power)
    Q_PROPERTY(double peak_flux MEMBER peak_flux)
    Q_PROPERTY(double min_flux MEMBER min_flux)
    Q_PROPERTY(double average_flux MEMBER average_flux)
    Q_PROPERTY(double sigma_flux MEMBER sigma_flux)
    Q_PROPERTY(double uniformity MEMBER uniformity)
    Q_PROPERTY(double peak_flux_uncertainty MEMBER peak_flux_uncertainty)
    Q_PROPERTY(double average_flux_uncertainty MEMBER average_flux_uncertainty)
    Q_PROPERTY(QVector3D centroid MEMBER centroid)
    Q_PROPERTY(QVector3D local_centroid MEMBER local_centroid)

public:
    /// Total rays in the simulation result used for this map.
    quint64 source_ray_count = 0;

    /// Number of plotted ray/surface interactions that contributed to this map.
    quint64 plotted_ray_count = 0;

    /// Power represented by each plotted ray interaction, in W when scaled.
    double power_per_ray = 1.0;

    /// Integrated power represented by plotted interactions, in W when scaled.
    double plotted_power = 0.0;

    /// Flux statistics over the rasterized map values.
    double peak_flux    = 0.0;
    double min_flux     = 0.0;
    double average_flux = 0.0;
    double sigma_flux   = 0.0;
    double uniformity   = 0.0;

    /// Monte Carlo uncertainty estimates in percent where counts are known.
    double peak_flux_uncertainty    = 0.0;
    double average_flux_uncertainty = 0.0;

    /// Average world-space location of plotted interactions.
    QVector3D centroid;

    /// Average plotted-interaction location in the target entity's local space.
    QVector3D local_centroid;

    bool operator==(BakedFluxMapStats const&) const = default;
};

/// Numeric flux-map data that can be exported or rendered again.
struct FluxMapData {
    /// Per-pixel flux values used with a color map to generate images.
    Support::Grid2D<float> counts;

    /// Mesh used when projecting ray hits into the map.
    SolTrace::GUI::Data::Mesh mesh;

    /// Per-face world-space area, ordered with mesh.triangles.
    QVector<float> face_area;

    /// Per-face ray interaction count, ordered with mesh.triangles.
    QVector<quint64> face_ray_count;

    /// Projected interactions retained so the point view can be rerendered.
    std::vector<glm::vec2> interaction_uvs;

    /// Summary statistics for the generated map.
    BakedFluxMapStats stats;
};

/// Disposable presentation derived from FluxMapData.
struct FluxMapRender {
    /// A generated image using counts and a color map
    QImage bin_map;

    /// A generated image showing UV points of intersecting rays
    QImage point_map;
};

/// A computed flux artifact and its current rendered presentation.
struct BakedFluxMap {
    FluxMapData   data;
    FluxMapRender render;

    /// Complete settings used to produce this artifact.
    FluxMapGenerationInfo generation;
};

using BakedFluxMapPtr = std::shared_ptr<BakedFluxMap const>;

} // namespace SolTrace::GUI::Analysis

Q_DECLARE_METATYPE(SolTrace::GUI::Analysis::BakedFluxMapStats)
