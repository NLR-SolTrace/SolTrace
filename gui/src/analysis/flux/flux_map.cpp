#include "flux_map.h"

#include "analysis/flux/triangle_raster.h"
#include "data/database.h"
#include "support/asynctask.h"
#include "support/grid2d.h"
#include "support/triangle_bvh.h"
#include "vector_utility.hpp"

#include <QDateTime>
#include <QFile>
#include <QImage>
#include <QLoggingCategory>
#include <QPainter>
#include <QTextStream>

#include <algorithm>
#include <cmath>
#include <limits>
#include <optional>
#include <vector>

namespace SolTrace::GUI::Analysis {

Q_LOGGING_CATEGORY(fluxMapLog, "soltrace.gui.fluxmap")

/// A point projected to a specific triangle of a mesh.
struct TriangleProjection {
    size_t    triangle_index = 0;
    glm::vec3 barycentric    = glm::vec3(0.0);
    glm::vec2 uv             = glm::vec2(0.0);
};

/// A per-triangle bin of ray data.
struct TriangleFluxBin {
    /// The area of the triangle in world space.
    float world_area = 0.0;

    /// Accumulated energy for this bin.
    float accumulated_energy = 0.0;

    /// How many rays intersected this triangle.
    size_t hit_count = 0;

    /// The final flux value
    float flux = 0.0f; // W/m^2 or normalized equivalent
};

/// Barycentric interpolation.
template <class T>
static T interpolate(T const& a, T const& b, T const& c, glm::vec3 bary) {
    return a * bary.x + b * bary.y + c * bary.z;
}

/// For a UV, snap to a pixel
static QPointF uv_to_pixel(glm::vec2 uv, QSize const& size) {
    auto width  = std::max(0, size.width() - 1);
    auto height = std::max(0, size.height() - 1);

    return QPointF(uv.x * width, uv.y * height);
}

/// Map UV boundaries to raster cell boundaries. Unlike point coordinates,
/// u/v == 1 lies on the far edge of the final pixel.
static QPointF uv_to_raster_edge(glm::vec2 uv, QSize const& size) {
    return QPointF(uv.x * size.width(), uv.y * size.height());
}

/// Get the area of a triangle in world space.
static float
triangle_area(glm::vec3 const& a, glm::vec3 const& b, glm::vec3 const& c) {
    return 0.5f * glm::length(glm::cross(b - a, c - a));
}


/// Obtain barycentric coords for a point on a triangle.
static std::optional<glm::vec3> barycentric_for_point(QPointF const& p,
                                                      QPointF const& a,
                                                      QPointF const& b,
                                                      QPointF const& c) {
    auto v0 = b - a;
    auto v1 = c - a;
    auto v2 = p - a;

    auto denom = v0.x() * v1.y() - v1.x() * v0.y();

    if (std::abs(denom) < 1e-8f) { return { }; }

    float u = (v2.x() * v1.y() - v1.x() * v2.y()) / denom;
    float v = (v0.x() * v2.y() - v2.x() * v0.y()) / denom;
    float w = 1.0f - u - v;

    return glm::vec3(w, u, v);
}

/// For a given point, find the closest point on a triangle mesh.
static std::optional<TriangleProjection>
project_point_to_triangle(SolTrace::GUI::Data::Mesh const& mesh,
                          Support::TriangleBvh const&      bvh,
                          glm::vec3                        p) {
    auto closest = bvh.closest_point(p);
    if (!closest) return { };

    auto const& v1 = mesh.vertex[mesh.triangles[closest->triangle_index].x];
    auto const& v2 = mesh.vertex[mesh.triangles[closest->triangle_index].y];
    auto const& v3 = mesh.vertex[mesh.triangles[closest->triangle_index].z];

    return TriangleProjection {
        .triangle_index = closest->triangle_index,
        .barycentric    = closest->barycentric,
        .uv = interpolate(v1.uv, v2.uv, v3.uv, closest->barycentric),
    };
}

static bool is_physical_interaction(
    SolTrace::GUI::Data::RayEventType event) {
    using SolTrace::GUI::Data::RayEventType;

    switch (event) {
    case RayEventType::ABSORB:
    case RayEventType::REFLECT:
    case RayEventType::TRANSMIT: return true;
    case RayEventType::CREATE:
    case RayEventType::VIRTUAL:
    case RayEventType::EXIT:
    case RayEventType::UNKNOWN: return false;
    }

    return false;
}

static bool is_direct_sun_interaction(
    SolTrace::GUI::Data::RayRecord const& ray,
    size_t                                interaction_index) {
    for (size_t i = 0; i < interaction_index; ++i) {
        if (is_physical_interaction(ray.events[i].event)) return false;
    }

    return true;
}

/// Make triangle bins; the per-triangle stat bin
static std::vector<TriangleFluxBin> make_triangle_bins(SolTrace::GUI::Data::Mesh const& mesh) {
    std::vector<TriangleFluxBin> triangles;
    triangles.reserve(mesh.triangles.size());

    for (size_t i = 0; i < mesh.triangles.size(); i++) {
        auto const& v1 = mesh.vertex[mesh.triangles[i].x];
        auto const& v2 = mesh.vertex[mesh.triangles[i].y];
        auto const& v3 = mesh.vertex[mesh.triangles[i].z];

        auto world = triangle_area(v1.position, v2.position, v3.position);

        triangles.emplace_back(TriangleFluxBin {
            .world_area = world,
        });
    }

    return triangles;
}

/// Compute the per-face flux retained for mesh exports and diagnostics.
static void compute_triangle_flux(std::vector<TriangleFluxBin>& triangles) {
    for (auto& triangle : triangles) {
        if (triangle.world_area <= 0.0f) { continue; }

        triangle.flux = triangle.accumulated_energy / triangle.world_area;
    }
}

static QVector<float>
extract_face_areas(std::vector<TriangleFluxBin> const& triangles) {
    QVector<float> face_area;
    face_area.reserve(static_cast<qsizetype>(triangles.size()));

    for (auto const& triangle : triangles) {
        face_area.push_back(triangle.world_area);
    }

    return face_area;
}

static QVector<quint64>
extract_face_ray_counts(std::vector<TriangleFluxBin> const& triangles) {
    QVector<quint64> face_ray_count;
    face_ray_count.reserve(static_cast<qsizetype>(triangles.size()));

    for (auto const& triangle : triangles) {
        face_ray_count.push_back(static_cast<quint64>(triangle.hit_count));
    }

    return face_ray_count;
}

static float max_raster_value(Support::Grid2D<float> const& raster) {
    float max_value = 0.0f;

    for (unsigned i = 0; i < raster.size(); ++i) {
        max_value = std::max(max_value, raster[i]);
    }

    return max_value;
}

struct PixelCoverage {
    unsigned index  = 0;
    float    weight = 0.0f;
};

/// Rasterize the amount of world-space surface represented by every UV pixel.
/// Each triangle's sampled coverage is normalized so that its complete world
/// area is preserved, independent of its tessellation density.
static Support::Grid2D<float>
raster_surface_area(std::vector<TriangleFluxBin> const& triangles,
                    SolTrace::GUI::Data::Mesh const&    mesh,
                    QSize const&                        image_size,
                    Support::TaskControl&               control,
                    int                                 progress_low,
                    int                                 progress_high) {
    Support::Grid2D<float> area(image_size.width(), image_size.height());
    area.fill(0.0f);

    auto report_progress = [&](int item, int max_item) {
        auto a = max_item > 0 ? float(item) / float(max_item) : 1.0f;
        int  p = glm::mix(float(progress_low), float(progress_high), a);
        control.setProgressValue(p);
    };

    constexpr int subpixel_samples_per_axis = 4;

    for (size_t tri_index = 0; tri_index < mesh.triangles.size(); ++tri_index) {
        auto const& triangle = triangles[tri_index];
        if (triangle.world_area <= 0.0f) {
            report_progress(int(tri_index + 1), int(triangles.size()));
            continue;
        }

        auto const& indices = mesh.triangles[tri_index];
        auto const& v1      = mesh.vertex[indices.x];
        auto const& v2      = mesh.vertex[indices.y];
        auto const& v3      = mesh.vertex[indices.z];

        QPointF const a = uv_to_raster_edge(v1.uv, image_size);
        QPointF const b = uv_to_raster_edge(v2.uv, image_size);
        QPointF const c = uv_to_raster_edge(v3.uv, image_size);

        int const min_x =
            std::max(0, int(std::floor(std::min({ a.x(), b.x(), c.x() }))));
        int const max_x =
            std::min(image_size.width() - 1,
                     int(std::ceil(std::max({ a.x(), b.x(), c.x() }))));
        int const min_y =
            std::max(0, int(std::floor(std::min({ a.y(), b.y(), c.y() }))));
        int const max_y =
            std::min(image_size.height() - 1,
                     int(std::ceil(std::max({ a.y(), b.y(), c.y() }))));

        std::vector<PixelCoverage> coverage;
        float                      total_coverage = 0.0f;

        for (int y = min_y; y <= max_y; ++y) {
            for (int x = min_x; x <= max_x; ++x) {
                float pixel_coverage = 0.0f;

                for (int sy = 0; sy < subpixel_samples_per_axis; ++sy) {
                    for (int sx = 0; sx < subpixel_samples_per_axis; ++sx) {
                        QPointF const p(
                            x + (sx + 0.5f) / subpixel_samples_per_axis,
                            y + (sy + 0.5f) / subpixel_samples_per_axis);

                        if (contains_with_edge_ownership(p, a, b, c)) {
                            pixel_coverage += 1.0f;
                        }
                    }
                }

                if (pixel_coverage <= 0.0f) continue;
                coverage.push_back(PixelCoverage {
                    .index  = area.index(x, y),
                    .weight = pixel_coverage,
                });
                total_coverage += pixel_coverage;
            }
        }

        if (total_coverage > 0.0f) {
            float const area_per_sample =
                triangle.world_area / total_coverage;
            for (auto const& pixel : coverage) {
                area[pixel.index] += pixel.weight * area_per_sample;
            }
        } else {
            // Preserve triangles smaller than the subpixel sampling pattern.
            auto const center = (a + b + c) / 3.0;
            int const  x = std::clamp(int(std::lround(center.x())),
                                     0,
                                     image_size.width() - 1);
            int const  y = std::clamp(int(std::lround(center.y())),
                                     0,
                                     image_size.height() - 1);
            area(x, y) += triangle.world_area;
        }

        if (control.cancelRequested()) return area;
        report_progress(int(tri_index + 1), int(triangles.size()));
    }

    return area;
}

/// Deposit one ray's energy into neighboring pixels without crossing outside
/// the rasterized receiver surface.
static void splat_energy(Support::Grid2D<float>&       energy,
                         Support::Grid2D<float> const& area,
                         glm::vec2                     uv,
                         float                         ray_energy) {
    auto const width  = int(energy.size_x());
    auto const height = int(energy.size_y());
    auto const p      = uv_to_pixel(uv, QSize(width, height));
    auto const px     = std::clamp(p.x(), 0.0, double(width - 1));
    auto const py     = std::clamp(p.y(), 0.0, double(height - 1));
    auto const x0     = int(std::floor(px));
    auto const y0     = int(std::floor(py));
    auto const x1     = std::min(x0 + 1, width - 1);
    auto const y1     = std::min(y0 + 1, height - 1);
    auto const tx     = float(px - x0);
    auto const ty     = float(py - y0);

    struct Candidate {
        int   x;
        int   y;
        float weight;
    };
    Candidate const candidates[] = {
        { x0, y0, (1.0f - tx) * (1.0f - ty) },
        { x1, y0, tx * (1.0f - ty) },
        { x0, y1, (1.0f - tx) * ty },
        { x1, y1, tx * ty },
    };

    float valid_weight = 0.0f;
    for (auto const& candidate : candidates) {
        if (area(candidate.x, candidate.y) > 0.0f) {
            valid_weight += candidate.weight;
        }
    }

    if (valid_weight > 0.0f) {
        for (auto const& candidate : candidates) {
            if (area(candidate.x, candidate.y) <= 0.0f) continue;
            energy(candidate.x, candidate.y) +=
                ray_energy * candidate.weight / valid_weight;
        }
        return;
    }

    // Degenerate UV triangles may only mark their centroid pixel. Find the
    // closest represented pixel rather than silently dropping the ray energy.
    for (int radius = 0; radius <= 2; ++radius) {
        for (int y = std::max(0, y0 - radius);
             y <= std::min(height - 1, y0 + radius);
             ++y) {
            for (int x = std::max(0, x0 - radius);
                 x <= std::min(width - 1, x0 + radius);
                 ++x) {
                if (area(x, y) <= 0.0f) continue;
                energy(x, y) += ray_energy;
                return;
            }
        }
    }
}

/// A separable Gaussian used to reduce sampling noise in UV space.
static Support::Grid2D<float>
smooth_raster(Support::Grid2D<float> const& source, unsigned requested_radius) {
    int const radius = std::clamp<int>(requested_radius, 1, 64);
    float const sigma = std::max(1.0f, radius / 2.0f);

    std::vector<float> kernel(static_cast<size_t>(radius * 2 + 1));
    for (int offset = -radius; offset <= radius; ++offset) {
        kernel[static_cast<size_t>(offset + radius)] =
            std::exp(-float(offset * offset) / (2.0f * sigma * sigma));
    }

    Support::Grid2D<float> horizontal(source.size_x(), source.size_y());
    Support::Grid2D<float> result(source.size_x(), source.size_y());
    horizontal.fill(0.0f);
    result.fill(0.0f);

    for (int y = 0; y < int(source.size_y()); ++y) {
        for (int x = 0; x < int(source.size_x()); ++x) {
            float sum = 0.0f;
            float weight_sum = 0.0f;
            for (int offset = -radius; offset <= radius; ++offset) {
                int const sample_x = x + offset;
                if (sample_x < 0 || sample_x >= int(source.size_x())) continue;
                float const weight =
                    kernel[static_cast<size_t>(offset + radius)];
                sum += source(sample_x, y) * weight;
                weight_sum += weight;
            }
            horizontal(x, y) = sum / weight_sum;
        }
    }

    for (int y = 0; y < int(source.size_y()); ++y) {
        for (int x = 0; x < int(source.size_x()); ++x) {
            float sum = 0.0f;
            float weight_sum = 0.0f;
            for (int offset = -radius; offset <= radius; ++offset) {
                int const sample_y = y + offset;
                if (sample_y < 0 || sample_y >= int(source.size_y())) continue;
                float const weight =
                    kernel[static_cast<size_t>(offset + radius)];
                sum += horizontal(x, sample_y) * weight;
                weight_sum += weight;
            }
            result(x, y) = sum / weight_sum;
        }
    }

    return result;
}

/// Build a flux-density raster directly from hit UVs. Energy and represented
/// surface area are smoothed independently so a constant flux remains constant
/// at receiver boundaries.
static Support::Grid2D<float>
raster_flux(std::vector<glm::vec2> const& uvs,
            float                         ray_energy,
            Support::Grid2D<float> const& surface_area,
            unsigned                      smoothing_radius) {
    Support::Grid2D<float> energy(surface_area.size_x(), surface_area.size_y());
    energy.fill(0.0f);

    for (auto const& uv : uvs) {
        splat_energy(energy, surface_area, uv, ray_energy);
    }

    Support::Grid2D<float> flux;
    Support::Grid2D<float> smoothed_area;

    if (smoothing_radius > 0) {
        flux = smooth_raster(energy, smoothing_radius);
        energy = Support::Grid2D<float>();
        smoothed_area = smooth_raster(surface_area, smoothing_radius);
    } else {
        flux = std::move(energy);
    }

    for (unsigned i = 0; i < flux.size(); ++i) {
        float const divisor = smoothing_radius > 0 ? smoothed_area[i]
                                                   : surface_area[i];
        if (surface_area[i] > 0.0f && divisor > 0.0f) {
            flux[i] /= divisor;
        } else {
            flux[i] = 0.0f;
        }
    }

    // Normalized smoothing near an aperture boundary can slightly change the
    // integral. Restore exact conservation of the plotted ray power.
    double integrated_power = 0.0;
    for (unsigned i = 0; i < flux.size(); ++i) {
        integrated_power += double(flux[i]) * double(surface_area[i]);
    }

    double const expected_power = double(uvs.size()) * double(ray_energy);
    if (integrated_power > 0.0 && expected_power >= 0.0) {
        float const scale = float(expected_power / integrated_power);
        for (unsigned i = 0; i < flux.size(); ++i) {
            flux[i] *= scale;
        }
    }

    return flux;
}

/// Take a raster bin and color map, and burn that to a QImage
static void colorize_raster(QImage&                       image,
                            Support::Grid2D<float> const& raster,
                            QImage const&                 color_map,
                            float                         max_density) {

    QPainter painter(&image);

    painter.setRenderHint(QPainter::Antialiasing);

    for (int x = 0; x < image.width(); ++x) {
        for (int y = 0; y < image.height(); ++y) {
            float normalized = 0.0f;

            if (max_density > 0.0f) {
                normalized = std::clamp(raster(x, y) / max_density, 0.0f, 1.0f);
            }

            auto sample = QPoint(normalized * (color_map.width() - 1),
                                 color_map.height() / 2);

            painter.setPen(color_map.pixelColor(sample));
            painter.drawPoint(QPoint(x, y));
        }
    }
}

/// Draw the UV structure to an Image
static void raster_mesh_overlay(QPainter&       painter,
                                SolTrace::GUI::Data::Mesh const& mesh,
                                QSize const&    image_size,
                                QColor const&   line_color) {
    if (!line_color.isValid()) { return; }

    painter.save();
    painter.setPen(QPen(line_color, 1.0));

    for (size_t i = 0; i < mesh.triangles.size(); i++) {
        auto const& v1 = mesh.vertex[mesh.triangles[i].x];
        auto const& v2 = mesh.vertex[mesh.triangles[i].y];
        auto const& v3 = mesh.vertex[mesh.triangles[i].z];

        QPolygonF triangle;
        triangle << uv_to_pixel(v1.uv, image_size)
                 << uv_to_pixel(v2.uv, image_size)
                 << uv_to_pixel(v3.uv, image_size);

        painter.drawPolygon(triangle);
    }

    painter.restore();
}

static QImage
make_points_debug_image(std::vector<glm::vec2> const&       uvs,
                        QSize const&                        image_size,
                        SolTrace::GUI::Data::Mesh const&    mesh,
                        QColor const&                       grid_line_color) {
    QImage image(image_size, QImage::Format_RGB32);
    image.fill(Qt::white);

    auto painter = QPainter(&image);
    raster_mesh_overlay(painter, mesh, image_size, grid_line_color);

    painter.save();
    auto const point_width = std::clamp(
        std::min(image_size.width(), image_size.height()) / 512.0,
        1.0,
        4.0);
    painter.setPen(QPen(Qt::red, point_width));

    for (auto const& uv : uvs) {
        QPointF pixel = uv_to_pixel(uv, image_size);
        painter.drawPoint(pixel);
    }

    painter.restore();

    return image;
}

static bool dump_interaction_points_csv(std::vector<glm::vec3> const& points,
                                        QString const&                path) {
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate |
                   QIODevice::Text)) {
        return false;
    }

    QTextStream stream(&file);
    stream << "x,y,z\n";

    for (auto const& point : points) {
        stream << point.x << ',' << point.y << ',' << point.z << '\n';
    }

    return true;
}

static BakedFluxMapStats
compute_flux_map_stats(Support::Grid2D<float> const&       raster,
                       Support::Grid2D<float> const&       surface_area,
                       std::vector<TriangleFluxBin> const& triangles,
                       std::vector<glm::vec3> const&       interaction_points,
                       std::size_t                         source_ray_count,
                       double                              power_per_ray) {
    BakedFluxMapStats stats;
    stats.source_ray_count  = static_cast<quint64>(source_ray_count);
    stats.plotted_ray_count = static_cast<quint64>(interaction_points.size());
    stats.power_per_ray     = power_per_ray;
    stats.plotted_power     = stats.plotted_ray_count * stats.power_per_ray;

    if (!interaction_points.empty()) {
        glm::dvec3 centroid(0.0);
        for (auto const& point : interaction_points) {
            centroid += glm::dvec3(point);
        }
        centroid /= static_cast<double>(interaction_points.size());
        stats.centroid = QVector3D(centroid.x, centroid.y, centroid.z);
    }

    if (raster.size() == 0 || raster.size() != surface_area.size()) return stats;

    double total_area        = 0.0;
    double area_weighted_sum = 0.0;
    double area_weighted_sq  = 0.0;
    double min_flux  = std::numeric_limits<double>::max();
    double peak_flux = 0.0;

    for (unsigned i = 0; i < raster.size(); ++i) {
        const double area = surface_area[i];
        if (area <= 0.0) continue;

        const double value = raster[i];
        total_area += area;
        area_weighted_sum += value * area;
        area_weighted_sq += value * value * area;
        min_flux  = std::min(min_flux, value);
        peak_flux = std::max(peak_flux, value);
    }

    stats.peak_flux        = peak_flux;
    stats.min_flux =
        min_flux == std::numeric_limits<double>::max() ? 0.0 : min_flux;
    stats.average_flux =
        total_area > 0.0 ? area_weighted_sum / total_area : 0.0;

    const double variance = total_area > 0.0
                                ? area_weighted_sq / total_area -
                                      stats.average_flux * stats.average_flux
                                : 0.0;
    stats.sigma_flux = std::sqrt(std::max(0.0, variance));
    stats.uniformity =
        stats.average_flux > 0.0 ? stats.sigma_flux / stats.average_flux : 0.0;

    std::size_t peak_hit_count     = 0;
    double      peak_triangle_flux = 0.0;
    for (auto const& triangle : triangles) {
        if (triangle.flux > peak_triangle_flux) {
            peak_triangle_flux = triangle.flux;
            peak_hit_count     = triangle.hit_count;
        }
    }

    if (peak_hit_count > 0) {
        stats.peak_flux_uncertainty =
            100.0 / std::sqrt(static_cast<double>(peak_hit_count));
    }
    if (source_ray_count > 0) {
        stats.average_flux_uncertainty =
            100.0 / std::sqrt(static_cast<double>(source_ray_count));
    }

    return stats;
}

static QString smoothing_description(unsigned radius) {
    switch (radius) {
    case 0: return QStringLiteral("None");
    case 2: return QStringLiteral("Light (2-bin radius)");
    case 4: return QStringLiteral("Medium (4-bin radius)");
    case 8: return QStringLiteral("Strong (8-bin radius)");
    default:
        return QStringLiteral("%1-bin radius").arg(radius);
    }
}

static void set_generation_metadata(
    QImage&                              image,
    QString const&                       image_kind,
    QString const&                       generated_at,
    entt::entity                         entity,
    FluxMapBakeOptions const&            opts,
    SolTrace::GUI::Data::Mesh const&     mesh,
    BakedFluxMapStats const&             stats) {
    image.setText(QStringLiteral("Software"), QStringLiteral("SolTrace"));
    image.setText(QStringLiteral("Description"), image_kind);
    image.setText(QStringLiteral("Creation Time"), generated_at);
    image.setText(QStringLiteral("SolTrace.FluxMapMetadataVersion"),
                  QStringLiteral("1"));
    image.setText(QStringLiteral("SolTrace.Entity"),
                  QString::number(entt::to_integral(entity)));
    image.setText(QStringLiteral("SolTrace.GridResolution"),
                  QStringLiteral("%1x%2")
                      .arg(opts.image_resolution.x)
                      .arg(opts.image_resolution.y));
    image.setText(QStringLiteral("SolTrace.Smoothing"),
                  smoothing_description(opts.smoothing_radius));
    image.setText(QStringLiteral("SolTrace.SmoothingRadiusBins"),
                  QString::number(opts.smoothing_radius));
    image.setText(QStringLiteral("SolTrace.SurfaceDetail"),
                  QString::number(opts.surface_detail));
    image.setText(QStringLiteral("SolTrace.MeshTriangleCount"),
                  QString::number(
                      static_cast<qulonglong>(mesh.triangles.size())));
    image.setText(QStringLiteral("SolTrace.ShowTriangleGrid"),
                  opts.grid_line_color.isValid() ? QStringLiteral("true")
                                                 : QStringLiteral("false"));
    image.setText(QStringLiteral("SolTrace.ColorMapId"), opts.color_map_id);
    image.setText(QStringLiteral("SolTrace.ColorMapName"),
                  opts.color_map_name);
    image.setText(QStringLiteral("SolTrace.ColorMapSource"),
                  opts.color_map_source);
    image.setText(QStringLiteral("SolTrace.SolarDNI.WPerM2"),
                  QString::number(opts.dni, 'g', 17));
    image.setText(QStringLiteral("SolTrace.IncludeDirectSunRays"),
                  opts.include_direct_sun_rays ? QStringLiteral("true")
                                               : QStringLiteral("false"));
    image.setText(QStringLiteral("SolTrace.TerminalInteractionsOnly"),
                  opts.terminal_interactions_only ? QStringLiteral("true")
                                                  : QStringLiteral("false"));
    image.setText(QStringLiteral("SolTrace.SourceRayCount"),
                  QString::number(stats.source_ray_count));
    image.setText(QStringLiteral("SolTrace.ContributingHits"),
                  QString::number(stats.plotted_ray_count));
    image.setText(QStringLiteral("SolTrace.PowerPerRay.W"),
                  QString::number(stats.power_per_ray, 'g', 17));
    image.setText(QStringLiteral("SolTrace.MappedPower.W"),
                  QString::number(stats.plotted_power, 'g', 17));
}

/// Main fluxmap compute function
Support::Result<BakedFluxMapPtr, QString>
execute_map_generation_for(Support::TaskControl&                    control,
                           FluxMapBakeOptions                       opts,
                           entt::entity                             entity,
                           SolTrace::GUI::Data::SimulationResultPtr results,
                           SolTrace::GUI::Data::Mesh                mesh) {

    qCDebug(fluxMapLog) << Q_FUNC_INFO << "starting map generation";

    // Image size makes no sense, bail
    if (!glm::all(glm::lessThan(glm::uvec2(1), opts.image_resolution))) {
        qCDebug(fluxMapLog) << Q_FUNC_INFO << "image resolution is too small";
        return QStringLiteral("Image resolution is not sufficient");
    }

    // We need to know what rays have hit this entity
    auto iter = results->entity_to_ray_ids.find(entity);

    if (iter == results->entity_to_ray_ids.end()) {
        qCDebug(fluxMapLog) << Q_FUNC_INFO << "no rays for this element";
        return QStringLiteral("No rays have interacted with selected element");
    }

    // TODO: rescope. might also be good to have a little util for all this
    constexpr int PROGRESS_SETUP      = 10;
    constexpr int PROGRESS_ACCUMULATE = 50;
    constexpr int PROGRESS_RASTER     = 90;
    constexpr int PROGRESS_COMPLETE   = 100;

    // Starting setup
    control.setProgressValue(PROGRESS_SETUP);

    qCDebug(fluxMapLog) << Q_FUNC_INFO << "setup complete";

    // Creating image. Use f32 formats for speed.
    auto img = QImage(
        opts.image_resolution.x, opts.image_resolution.y, QImage::Format_RGB32);

    // Fill triangle bins
    auto triangles = make_triangle_bins(mesh);

    // Accell structure
    auto triangle_bvh = Support::TriangleBvh(mesh);

    qCDebug(fluxMapLog) << Q_FUNC_INFO << "ready triangle bins";

    const double result_ray_area_weight =
        std::isfinite(results->ray_area_weight) ? results->ray_area_weight : 0.0;
    const double power_per_ray =
        std::max(0.0, opts.dni) *
        (result_ray_area_weight > 0.0 ? result_ray_area_weight : 1.0);
    std::vector<glm::vec2> interaction_uvs;
    std::vector<glm::vec3> interaction_points;
    interaction_uvs.reserve(iter->second.size());
    interaction_points.reserve(iter->second.size());

    auto report_progress =
        [&](int item, int max_item, int prog_low, int prog_high) {
            auto a = (float)item / (float)max_item;
            int  p = glm::mix((float)prog_low, (float)prog_high, a);
            control.setProgressValue(p);
        };

    size_t ray_count = 0;
    size_t ray_count_chunk =
        std::clamp<size_t>(iter->second.size() / 10, 1, 500);

    // Burn rays to triangle bins
    for (auto ray_index : iter->second) {
        ray_count++;

        if (ray_count % ray_count_chunk == 0) {
            report_progress(ray_count,
                            iter->second.size(),
                            PROGRESS_SETUP,
                            PROGRESS_ACCUMULATE);
        }

        ASYNC_TASK_SYNC_POINT(control);

        auto const& ray = results->records.at(ray_index);
        for (size_t interaction_index = 0;
             interaction_index < ray.events.size();
             ++interaction_index) {
            auto const& interaction = ray.events[interaction_index];

            // TODO: we need to double check that this is ok
            if (interaction.entity != entity) { continue; }
            switch (interaction.event) {
            case SolTrace::GUI::Data::RayEventType::CREATE:
            case SolTrace::GUI::Data::RayEventType::VIRTUAL:
            case SolTrace::GUI::Data::RayEventType::EXIT:
            case SolTrace::GUI::Data::RayEventType::UNKNOWN: continue;
            case SolTrace::GUI::Data::RayEventType::ABSORB:
            case SolTrace::GUI::Data::RayEventType::REFLECT:
            case SolTrace::GUI::Data::RayEventType::TRANSMIT: break;
            }

            if (!opts.include_direct_sun_rays &&
                is_direct_sun_interaction(ray, interaction_index)) {
                continue;
            }

            if (opts.terminal_interactions_only &&
                interaction.event !=
                    SolTrace::GUI::Data::RayEventType::ABSORB) {
                continue;
            }

            auto projection = project_point_to_triangle(
                mesh, triangle_bvh, interaction.location);

            if (!projection.has_value()) { continue; }

            auto& triangle = triangles[projection->triangle_index];

            float hit_energy = static_cast<float>(power_per_ray);
            triangle.hit_count += 1;
            triangle.accumulated_energy += hit_energy;
            interaction_points.push_back(interaction.location);
            interaction_uvs.push_back(projection->uv);
        }
    }

    qCDebug(fluxMapLog) << Q_FUNC_INFO << "triangle bins filled";

    ASYNC_TASK_SYNC_POINT(control);

    control.setProgressValue(PROGRESS_ACCUMULATE);

    // The face values remain useful for exports and diagnostics, but the
    // visible map is accumulated directly in UV space so its gradients do not
    // inherit the mesh's triangle boundaries.
    compute_triangle_flux(triangles);

    auto surface_area = raster_surface_area(triangles,
                                            mesh,
                                            img.size(),
                                            control,
                                            PROGRESS_ACCUMULATE,
                                            PROGRESS_RASTER);

    ASYNC_TASK_SYNC_POINT(control);

    auto raster = raster_flux(interaction_uvs,
                              static_cast<float>(power_per_ray),
                              surface_area,
                              opts.smoothing_radius);

    auto max_density = max_raster_value(raster);

    qCDebug(fluxMapLog) << Q_FUNC_INFO << "rastered bins";

    ASYNC_TASK_SYNC_POINT(control);

    colorize_raster(img, raster, opts.color_map, max_density);

    control.setProgressValue(PROGRESS_RASTER);

    {
        auto painter = QPainter(&img);
        painter.setRenderHint(QPainter::Antialiasing);
        raster_mesh_overlay(painter, mesh, img.size(), opts.grid_line_color);
    }

    auto points_img = make_points_debug_image(interaction_uvs,
                                              img.size(),
                                              mesh,
                                              opts.grid_line_color);
    auto stats = compute_flux_map_stats(raster,
                                        surface_area,
                                        triangles,
                                        interaction_points,
                                        results->records.size(),
                                        power_per_ray);

    if (stats.plotted_ray_count > 0 && results->database) {
        auto const* cached_global =
            results->database->global_transform.get(entity);
        auto const global = cached_global
                                ? *cached_global
                                : SolTrace::GUI::Data::GlobalTransformComponent::
                                      compute_for(results->database->as_registry(),
                                                  entity);
        auto const world_centroid = glm::dvec3(stats.centroid.x(),
                                               stats.centroid.y(),
                                               stats.centroid.z());
        auto const local_centroid =
            glm::inverse(global.rotation) * (world_centroid - global.position);
        stats.local_centroid = QVector3D(local_centroid.x,
                                         local_centroid.y,
                                         local_centroid.z);
    }

    qCDebug(fluxMapLog) << Q_FUNC_INFO << "complete";

    // Convert to the more common format and attach settings to saved images.
    img = img.convertToFormat(QImage::Format_RGBA8888);

    auto const generated_at =
        QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs);
    set_generation_metadata(img,
                            QStringLiteral("SolTrace flux-density map"),
                            generated_at,
                            entity,
                            opts,
                            mesh,
                            stats);
    set_generation_metadata(points_img,
                            QStringLiteral("SolTrace flux-map interaction points"),
                            generated_at,
                            entity,
                            opts,
                            mesh,
                            stats);

    control.setProgressValue(PROGRESS_COMPLETE);

    return std::make_shared<BakedFluxMap>(BakedFluxMap {
        .counts         = std::move(raster),
        .mesh           = std::move(mesh),
        .face_area      = extract_face_areas(triangles),
        .face_ray_count = extract_face_ray_counts(triangles),
        .bin_map        = img,
        .point_map      = points_img,
        .stats            = stats,
        .color_map_id     = opts.color_map_id,
        .color_map_name   = opts.color_map_name,
        .color_map_source = opts.color_map_source,
    });
}

FluxMapComputer::FluxMapComputer(QObject* parent) : QObject(parent) { }

FluxMapComputer::~FluxMapComputer() = default;

void FluxMapComputer::set_results(SolTrace::GUI::Data::SimulationResultPtr p) {
    m_database = p;

    cancel_all();
}


/// Precondition:
/// Mesh must NOT have overlapping UVs
bool FluxMapComputer::start_generate_for(SolTrace::GUI::Data::Entity         e,
                                         SolTrace::GUI::Data::Mesh           mesh,
                                         FluxMapBakeOptions options) {

    if (!m_database) {
        qCDebug(fluxMapLog) << Q_FUNC_INFO << "No database, bailing";
        return false;
    }

    if (options.color_map.isNull()) {
        options.color_map = QImage(":/assets/images/b_to_r_wide.png");

        if (options.color_map.isNull()) {
            qCritical() << "Missing colormaps!";
            return false;
        }
    }

    qCDebug(fluxMapLog) << Q_FUNC_INFO << "Loaded colormap"
                        << options.color_map.size()
                        << options.color_map.sizeInBytes();

    auto task = launch_async_task<BakedFluxMapPtr, QString>(
        e,
        this,
        &FluxMapComputer::image_ready,
        &FluxMapComputer::image_failed,
        execute_map_generation_for,
        options,
        e,
        m_database,
        mesh);

    connect(task,
            &Support::AsyncTaskBase::progress,
            this,
            [this, e](int progress) { emit image_progress(e, progress); });

    // Set up cancelling
    connect(this,
            &FluxMapComputer::cancel_all,
            task,
            &Support::AsyncTaskBase::cancel);

    // Set up targeted cancelling
    connect(this,
            &FluxMapComputer::cancel_specific,
            task,
            [e, task](SolTrace::GUI::Data::Entity item) {
                if (item == e) { task->cancel(); }
            });

    qCDebug(fluxMapLog) << Q_FUNC_INFO << "Job kicked off to thread.";

    return true;
}

} // namespace SolTrace::GUI::Analysis
