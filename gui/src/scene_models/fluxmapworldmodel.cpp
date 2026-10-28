#include "fluxmapworldmodel.h"

#include "analysis/flux/flux_map.h"
#include "data/components.h"
#include "data/surface.h"
#include "scene_models/color_map_model.h"

#include <QVariantMap>
#include <algorithm>
#include <limits>

namespace SolTrace::GUI::Data {

namespace {

// TODO: We can use something like this all over the place.
struct QVectorBoundsAccumulator {
    bool      valid = false;
    QVector3D min;
    QVector3D max;

    QVectorBoundsAccumulator()
        : min(std::numeric_limits<float>::max(),
              std::numeric_limits<float>::max(),
              std::numeric_limits<float>::max()),
          max(-std::numeric_limits<float>::max(),
              -std::numeric_limits<float>::max(),
              -std::numeric_limits<float>::max()) { }

    void include(QVector3D const& point) {
        min.setX(std::min(min.x(), point.x()));
        min.setY(std::min(min.y(), point.y()));
        min.setZ(std::min(min.z(), point.z()));
        max.setX(std::max(max.x(), point.x()));
        max.setY(std::max(max.y(), point.y()));
        max.setZ(std::max(max.z(), point.z()));
        valid = true;
    }
};

QVariantMap bounds_map(QVectorBoundsAccumulator const& bounds) {
    return QVariantMap {
        { QStringLiteral("valid"), bounds.valid },
        { QStringLiteral("min"), bounds.min },
        { QStringLiteral("max"), bounds.max },
    };
}

void include_transformed_box(QVectorBoundsAccumulator& bounds,
                             BoundingBox const&        box,
                             QVector3D const&          position,
                             QQuaternion const&        rotation) {
    for (int ix = 0; ix < 2; ++ix) {
        for (int iy = 0; iy < 2; ++iy) {
            for (int iz = 0; iz < 2; ++iz) {
                QVector3D corner(ix ? box.max.x() : box.min.x(),
                                 iy ? box.max.y() : box.min.y(),
                                 iz ? box.max.z() : box.min.z());

                bounds.include(position + rotation.rotatedVector(corner));
            }
        }
    }
}

} // namespace

static QString image_name(Entity e) {
    return QString::number(entt::to_integral((entt::entity)e));
}

static QString
image_key(SimulationResult const& results, Entity entity, quint64 revision) {
    return results.id.toString(QUuid::WithoutBraces) + QLatin1Char('/') +
           image_name(entity) + QLatin1Char('/') + QString::number(revision);
}

FluxTextureData::FluxTextureData(QQuick3DObject* parent)
    : QQuick3DTextureData(parent) { }

void FluxTextureData::set_image(QImage const& image) {
    auto rgba = image.convertToFormat(QImage::Format_RGBA8888);

    setSize(rgba.size());
    setFormat(QQuick3DTextureData::RGBA8);
    setHasTransparency(rgba.hasAlphaChannel());
    setTextureData(QByteArray(reinterpret_cast<const char*>(rgba.constBits()),
                              rgba.sizeInBytes()));

    update();
}

// ============================================================================

PendingFluxMapModel::PendingFluxMapModel(QObject* parent)
    : StructModelAdapter(parent) {

    m_compute = new SolTrace::GUI::Analysis::FluxMapComputer(this);

    connect(m_compute,
            &SolTrace::GUI::Analysis::FluxMapComputer::image_progress,
            this,
            &PendingFluxMapModel::on_progress);

    connect(m_compute,
            &SolTrace::GUI::Analysis::FluxMapComputer::image_ready,
            this,
            &PendingFluxMapModel::on_ready);

    connect(m_compute,
            &SolTrace::GUI::Analysis::FluxMapComputer::image_failed,
            this,
            &PendingFluxMapModel::on_failed);
}


void PendingFluxMapModel::reset(SolTrace::GUI::Data::SimulationResultPtr res) {
    emit cleared();

    m_results = res;
    m_latest_requests.clear();
    m_host = res && res->database ? res->database.get() : nullptr;
    m_compute->set_results(res);

    on_changed();

    if (res && res->database) {
        for (auto const& [entity, record] : res->flux_maps) {
            if (!record.artifact) continue;
            m_latest_requests[entity] = record.revision;
            emit ready(entity,
                       record.artifact,
                       res->database.get(),
                       image_key(*res, entity, record.revision));
        }
    }
}

void PendingFluxMapModel::on_changed() {
    store_remove_all();
}

bool PendingFluxMapModel::is_current(
    SolTrace::GUI::Analysis::FluxMapRequestId const& request) const {
    if (!m_results || request.result_id != m_results->id) return false;
    auto const iter = m_latest_requests.find(request.entity);
    return iter != m_latest_requests.end() && iter->second == request.revision;
}

void PendingFluxMapModel::on_ready(
    SolTrace::GUI::Analysis::FluxMapRequestId request,
    SolTrace::GUI::Analysis::BakedFluxMapPtr  image) {
    if (!m_host || !m_results || !is_current(request)) return;

    {
        // The database only carries a lightweight rendering marker. The
        // artifact itself is owned by SimulationResult::flux_maps.
        auto* ptr = const_cast<SolTrace::GUI::Data::Database*>(m_host.data());
        ptr->as_registry().emplace_or_replace<HasFluxMapComponent>(
            request.entity);
    }

    m_results->flux_maps.insert_or_assign(
        request.entity,
        FluxMapRecord { .revision = request.revision, .artifact = image });

    store_remove_by_predicate(
        [request](auto& record) { return record.entity == request.entity; });

    emit ready(request.entity,
               image,
               m_host,
               image_key(*m_results, request.entity, request.revision));

    if (this->rowCount() == 0) { emit all_done(); }
}

void PendingFluxMapModel::on_failed(
    SolTrace::GUI::Analysis::FluxMapRequestId request,
    QString                                   reason) {
    if (!is_current(request)) return;

    emit failed(request.entity, reason);

    this->store_remove_by_predicate(
        [request](FluxMappedPendingItem const& item) {
            return item.entity == request.entity;
        });

    if (this->rowCount() == 0) { emit all_done(); }
}

void PendingFluxMapModel::on_progress(
    SolTrace::GUI::Analysis::FluxMapRequestId request,
    int                                       progress) {
    if (!is_current(request)) return;

    // find and update

    for (int i = 0; i < this->rowCount(); i++) {
        auto* p = get_at(i);

        if (!p) continue;

        if (p->entity == request.entity) {
            auto copy = *p;

            copy.progress = progress;

            store_push_update(i, copy);

            return;
        }
    }
}

static std::optional<Mesh>
find_mesh_for(SolTrace::GUI::Data::SurfaceGenerationOptions surface_options,
              Database const*                               database,
              Entity                                        entity) {
    auto* surface_membership = database->geometry_group_membership.get(entity);

    if (!surface_membership) { return { }; }

    auto* surface =
        database->geometry_parameters.get(surface_membership->group);

    if (!surface) { return { }; }

    auto mesh = SolTrace::GUI::Data::generate_surface(
        surface->surface, surface->aperture, surface_options);
    if (!mesh) { return { }; }

    auto global = database->global_transform.get(entity);
    auto world  = global ? *global
                         : GlobalTransformComponent::compute_for(
                               database->as_registry(), entity);

    for (auto& vertex : mesh->vertex) {
        vertex.position = glm::vec3(
            world.position + world.rotation * glm::dvec3(vertex.position));
        vertex.normal = glm::normalize(
            glm::vec3(world.rotation * glm::dvec3(vertex.normal)));
    }

    return mesh;
}

static bool surface_wraps_u(Database const* database, Entity entity) {
    auto const* membership = database->geometry_group_membership.get(entity);
    if (!membership) return false;

    auto const* geometry = database->geometry_parameters.get(membership->group);
    return geometry && geometry->surface &&
           geometry->surface->my_type == SolTrace::Data::CYLINDER;
}

// void PendingFluxMapModel::start_debug() {
//     // find something with name zero
//     if (!m_host) return;
//     for (auto const& [e, c, g] :
//          m_host->as_registry()
//              .view<IdentityComponent, GeometryGroupMemberComponent>()
//              .each()) {
//         if (c.name == "0") {
//             start_generate_for(e);
//             return;
//         }
//     }
// }

bool PendingFluxMapModel::start_generate_for(Entity entity) {
    if (!m_host || !m_results) return false;

    qDebug() << Q_FUNC_INFO << entity;

    auto mesh_res = std::max(1, mesh_resolution_multiply());

    SolTrace::GUI::Data::SurfaceGenerationOptions surface_options;
    surface_options.subdivide_polygon_apertures = true;
    surface_options.height_field_resolution *= mesh_res;
    surface_options.radial_subdivisions *= mesh_res;
    surface_options.perimeter_subdivisions *= mesh_res;
    surface_options.cylinder_angular_subdivisions *= mesh_res;
    surface_options.cylinder_length_subdivisions *= mesh_res;

    auto mesh = find_mesh_for(surface_options, m_host, entity);

    if (!mesh) {
        qDebug() << Q_FUNC_INFO << "entity has no mesh, bailing";
        return false;
    }

    qDebug() << Q_FUNC_INFO << mesh->vertex.size() << mesh->triangles.size();

    quint64 revision = 1;
    if (auto iter = m_latest_requests.find(entity);
        iter != m_latest_requests.end()) {
        revision = iter->second + 1;
    }
    if (auto iter = m_results->flux_maps.find(entity);
        iter != m_results->flux_maps.end()) {
        revision = std::max(revision, iter->second.revision + 1);
    }

    // Make the old request stale before asking it to cancel, so its eventual
    // cancellation callback cannot remove this request's pending row.
    m_latest_requests[entity] = revision;
    cancel_for(entity);
    store_remove_by_predicate(
        [entity](auto const& item) { return item.entity == entity; });

    auto const selected_color_map =
        ColorMapModel::normalized_id(this->color_map_id());

    auto opts = SolTrace::GUI::Analysis::FluxMapBakeOptions {
        .image_resolution = { this->image_resolution().width(),
                              this->image_resolution().height(), },
        .grid_line_color =
            this->show_mesh_grid() ? this->mesh_line_color() : QColor(),
        .color_map = ColorMapModel::image_for_id(selected_color_map),
        .color_map_id = selected_color_map,
        .color_map_name = ColorMapModel::name_for_id(selected_color_map),
        .color_map_source = ColorMapModel::source_for_id(selected_color_map),
        .dni = this->dni(),
        .smoothing_radius =
            static_cast<unsigned>(std::max(0, this->smoothing_radius())),
        .wrap_u         = surface_wraps_u(m_host, entity),
        .surface_detail = static_cast<unsigned>(mesh_res),
        .include_direct_sun_rays = this->include_direct_sun_rays(),
        .terminal_interactions_only = this->terminal_interactions_only(),
    };

    store_push_append(FluxMappedPendingItem {
        .entity   = entity,
        .progress = 0,
    });

    auto request = SolTrace::GUI::Analysis::FluxMapRequestId {
        .result_id = m_results->id,
        .entity    = entity,
        .revision  = revision,
    };

    if (!m_compute->start_generate_for(request, *mesh, opts)) {
        store_remove_by_predicate(
            [entity](auto const& item) { return item.entity == entity; });
        qDebug() << Q_FUNC_INFO << "generation kickoff failed";
        return false;
    }

    qDebug() << Q_FUNC_INFO << "generation kickoff success";

    return true;
}

void PendingFluxMapModel::cancel_for(Entity entity) {
    m_compute->cancel_specific(entity);
}

void PendingFluxMapModel::cancel_all() {
    m_compute->cancel_all();
}

FluxMapProvider* PendingFluxMapModel::make_new_provider() {
    auto ret = new FluxMapProvider();

    connect(this, &PendingFluxMapModel::ready, ret, &FluxMapProvider::on_ready);
    connect(this, &PendingFluxMapModel::cleared, ret, &FluxMapProvider::clear);

    return ret;
}


// ============================================================================

void FluxMapWorldModel::on_reset() {
    store_remove_all();
}

FluxMapWorldModel::FluxMapWorldModel(QObject* parent)
    : StructModelAdapter(parent) { }

void FluxMapWorldModel::apply_surface_options(
    std::shared_ptr<SurfaceGeometry> const& geometry) const {
    if (!geometry) return;

    auto thickness = std::clamp(m_surface_thickness, 0.0, 1.0);
    geometry->set_add_thickness(thickness > 0.0);
    geometry->set_thickness(thickness);
    geometry->set_subdivision_scale(
        std::clamp<unsigned>(m_subdivision_scale, 1, 10));
}

QVariantMap FluxMapWorldModel::content_bounds() const {
    QVectorBoundsAccumulator bounds;

    for (auto const& item : m_records) {
        if (!item.flux_geometry || item.flux_geometry->vertex_count() == 0) {
            continue;
        }

        include_transformed_box(bounds,
                                item.flux_geometry->bounding_box(),
                                item.flux_position,
                                item.flux_rotation);
    }

    return bounds_map(bounds);
}

void FluxMapWorldModel::set_surface_thickness(double thickness) {
    thickness = std::clamp(thickness, 0.0, 1.0);
    if (m_surface_thickness == thickness) return;

    m_surface_thickness = thickness;

    for (auto const& item : m_records) {
        apply_surface_options(item.flux_geometry);
    }
}

void FluxMapWorldModel::set_subdivision_scale(unsigned scale) {
    scale = std::clamp<unsigned>(scale, 1, 10);
    if (m_subdivision_scale == scale) return;

    m_subdivision_scale = scale;

    for (auto const& item : m_records) {
        apply_surface_options(item.flux_geometry);
    }
}


void FluxMapWorldModel::on_ready(Entity                                   e,
                                 SolTrace::GUI::Analysis::BakedFluxMapPtr img,
                                 Database const*                          db,
                                 QString image_key) {
    // Avoid duplicating map entries; this list should stay small.

    if (!db) return;

    qDebug() << Q_FUNC_INFO << e << img->render.bin_map << db;

    for (int i = 0; i < m_records.size(); ++i) {
        if (m_records[i].flux_entity != e) continue;

        auto updated = m_records[i];
        if (!updated.flux_texture_data) {
            updated.flux_texture_data = std::make_shared<FluxTextureData>();
        }
        updated.flux_texture_data->set_image(img->render.bin_map);
        updated.flux_image_path       = QStringLiteral("image://fluxmap/") +
                                        image_key + QStringLiteral("/bins");
        updated.flux_point_image_path = QStringLiteral("image://fluxmap/") +
                                        image_key + QStringLiteral("/points");
        updated.flux_color_map_id     = img->generation.color_map_id;
        updated.flux_color_map_source = img->generation.color_map_source;
        updated.flux_stats            = img->data.stats;
        store_push_update(i, updated);
        return;
    }

    // get geom params

    auto group = db->geometry_of(e);

    auto geom = std::make_shared<SurfaceGeometry>();

    apply_surface_options(geom);
    geom->set(db, group);

    auto global = GlobalTransformComponent::compute_for(db->as_registry(), e);

    auto position =
        QVector3D(global.position.x, global.position.y, global.position.z);

    auto rotation = QQuaternion(global.rotation.w,
                                global.rotation.x,
                                global.rotation.y,
                                global.rotation.z);

    auto texture_data = std::make_shared<FluxTextureData>();
    texture_data->set_image(img->render.bin_map);

    store_push_append(FluxMappedItem {
        .flux_entity       = e,
        .flux_texture_data = texture_data,
        .flux_image_path   = QStringLiteral("image://fluxmap/") + image_key +
                             QStringLiteral("/bins"),
        .flux_point_image_path = QStringLiteral("image://fluxmap/") +
                                 image_key + QStringLiteral("/points"),
        .flux_color_map_id     = img->generation.color_map_id,
        .flux_color_map_source = img->generation.color_map_source,
        .flux_position         = position,
        .flux_rotation         = rotation,
        .flux_geometry         = geom,
        .flux_stats            = img->data.stats,
    });
}

// ============================================================================

FluxMapProvider::FluxMapProvider()
    : QQuickImageProvider(QQuickImageProvider::ImageType::Image) { }

QImage FluxMapProvider::requestImage(QString const& id,
                                     QSize*         size,
                                     QSize const&   requestedSize) {
    qDebug() << Q_FUNC_INFO << id << requestedSize;

    QString local_id = id;

    QImage ret;

    bool needs_point_map = local_id.endsWith(QStringLiteral("/points"));
    bool needs_bin_map   = local_id.endsWith(QStringLiteral("/bins"));

    if (needs_point_map) {
        local_id.chop(QStringLiteral("/points").size());
    } else if (needs_bin_map) {
        local_id.chop(QStringLiteral("/bins").size());
    }


    {
        QMutexLocker locker(&m_lock);
        auto         iter = m_store.find(local_id);
        if (iter != m_store.end()) {

            if (needs_point_map) {
                ret = iter.value()->render.point_map;
            } else {
                ret = iter.value()->render.bin_map;
            }
        }
    }

    if (size) *size = ret.size();

    qDebug() << Q_FUNC_INFO << ret;

    return ret;
}

void FluxMapProvider::on_ready(Entity                                   k,
                               SolTrace::GUI::Analysis::BakedFluxMapPtr v,
                               Database const*,
                               QString image_key) {
    qDebug() << Q_FUNC_INFO << k << v->render.bin_map << image_key;
    QMutexLocker locker(&m_lock);

    // Drop older revisions for this result/entity so versioned URLs do not
    // turn the provider into an unbounded image cache.
    auto const owner =
        image_key.left(image_key.lastIndexOf('/')) + QLatin1Char('/');
    for (auto iter = m_store.begin(); iter != m_store.end();) {
        if (iter.key().startsWith(owner)) {
            iter = m_store.erase(iter);
        } else {
            ++iter;
        }
    }
    m_store[image_key] = v;
}

void FluxMapProvider::clear() {
    QMutexLocker locker(&m_lock);
    m_store.clear();
}

// ============================================================================


QVector<EntityNamePair> AllComputedMapsModel::rebuild_lists() {
    QVector<EntityNamePair> new_recs;
    m_reverse.clear();

    if (!m_host || !m_results) return { };

    for (auto const& [entity, map] : m_results->flux_maps) {
        if (!map.artifact) continue;
        new_recs.push_back(EntityNamePair::record_for_entity(*m_host, entity));
    }

    for (qsizetype i = 0; i < new_recs.size(); ++i) {
        m_reverse[new_recs[i].entity] = static_cast<int>(i);
    }

    return new_recs;
}

void AllComputedMapsModel::recompute() {
    store_reset(rebuild_lists());
}

void AllComputedMapsModel::ident_changed(entt::entity entity) {
    if (!m_host) return;

    if (auto iter = m_reverse.find(entity); iter != m_reverse.end()) {
        store_push_update(iter->second,
                          EntityNamePair::record_for_entity(*m_host, entity));
    }
}


AllComputedMapsModel::AllComputedMapsModel(QObject* parent)
    : StructModelAdapter(parent) { }

void AllComputedMapsModel::reset(SimulationResultPtr results) {
    if (m_host) disconnect(m_host->identity.self(), nullptr, this, nullptr);

    m_results = std::move(results);
    m_host    = m_results && m_results->database
                    ? const_cast<Database*>(m_results->database.get())
                    : nullptr;
    recompute();

    if (!m_host) return;

    connect(m_host->identity.self(),
            &ComponentAPIBase::changed,
            this,
            &AllComputedMapsModel::ident_changed);
}

void AllComputedMapsModel::map_ready(Entity,
                                     SolTrace::GUI::Analysis::BakedFluxMapPtr,
                                     Database const* database,
                                     QString) {
    if (database == m_host) recompute();
}

int AllComputedMapsModel::index_of(SolTrace::GUI::Data::Entity entity) const {
    if (auto iter = m_reverse.find(entity); iter != m_reverse.end()) {
        return iter->second;
    }

    return -1;
}

SolTrace::GUI::Data::Entity AllComputedMapsModel::entity_at(int index) const {
    auto const& items = vector();
    if (index < 0 || index >= items.size()) { return { }; }

    return items[index].entity;
}

} // namespace SolTrace::GUI::Data
