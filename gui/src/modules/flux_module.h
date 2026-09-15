#pragma once

#include "database/fluxmapworldmodel.h"
#include "database/mesh_qml_bridge.h"
#include "database/models/element_models.h"
#include "module_common.h"
#include "utilities/notification.h"
#include "utilities/qt_helpers.h"
#include <QObject>
#include <QVector3D>

namespace SolTrace::GUI::App {

/**
 * @class FluxModule
 * @brief Flux analysis module.
 *
 * Provides access to flux distribution results from the simulation.
 * Shares ResultsBackend with Intersections — both modules read from
 * the same simulation result data.
 *
 * QML access pattern: App.flux.results
 */
class FluxModule : public QObject {
    Q_OBJECT

    QPointer<db::FluxMapProvider> m_image_provider;

    // TODO: add front or back filtering

    db::SimulationResultPtr m_results;

    QOBJECT_READONLY_PROPERTY(db::AllElementsModel, entity_model);
    QOBJECT_READONLY_PROPERTY(db::AllComputedMapsModel, computed_maps_model);
    QOBJECT_READONLY_PROPERTY(db::PendingFluxMapModel, pending_flux_maps);
    QOBJECT_READONLY_PROPERTY(db::FluxMapWorldModel, flux_map_world_model);

    Q_READONLY_PROPERTY(bool, ray_volume_flux_in_progress);
    QOBJECT_READONLY_PROPERTY(db::QMLMesh, ray_iso_volume);

    Q_WRITABLE_PROPERTY(db::Entity, current_entity, { });
    Q_READONLY_PROPERTY(QString, current_entity_name);
    Q_READONLY_PROPERTY(QVector3D, current_entity_position);
    Q_READONLY_PROPERTY(analysis::BakedFluxMapStats, current_flux_stats);

    Q_WRITABLE_PROPERTY(bool, show_flux_volume, true);
    Q_WRITABLE_PROPERTY(bool, show_other_geometry, false);
    Q_WRITABLE_PROPERTY(double, dni, 1000.0);

    // Hack
    Q_WRITABLE_PROPERTY(QString, current_image, { });

    qint64           m_batch_max = 0;
    QSet<db::Entity> m_batch_pending;

private:
    void refresh_current_flux_stats();

    void maybe_update_batch(db::Entity);

private slots:
    void
    flux_map_ready(db::Entity, analysis::BakedFluxMapPtr, db::Database const*);

    void flux_vol_ready(QUuid const&, analysis::SparseGrid3D<float>);
    void flux_vol_failed(QUuid const&, QString);

    void iso_surf_ready(QUuid const&, db::Mesh);
    void iso_surf_failed(QUuid const&, QString);

public:
    explicit FluxModule(QQmlEngine*, QObject* parent = nullptr);

public slots:
    /// Set the result set used for all flux computations and scene models.
    void set_results(db::SimulationResultPtr);

    /// Select the entity whose flux map/statistics are shown in the UI.
    void select_entity(db::Entity);

    /// Generate a surface flux map for current_entity.
    void start_generate();

    /// Generate a volumetric ray-density raster from the current result.
    void start_generate_volume_flux(unsigned resolution);

    /// Generate an isosurface mesh from the current volumetric raster.
    void start_generate_isosurface(float value);

    /// Save a flux image to disk
    void save_image(QString requested_image, QUrl path);

    /// Start a long running process batch generating everything
    void start_generate_batch();

    void cancel_batch();

signals:
    void notify(ANotification);

    void started_batch();
    void batch_progress(qint64, qint64);
};

} // namespace SolTrace::GUI::App
