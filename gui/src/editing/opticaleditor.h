#pragma once

#include "data/database.h"
#include "data/database_observer.h"
#include "simulation_data_export.hpp"
#include "support/structmodel.h"

#include <QObject>
#include <QVector>
#include <QtTypes>
#include <entt/entity/fwd.hpp>

#include <vector>

namespace SolTrace::GUI::Data {

struct AngularOpticalPoint {
    double angle = 0.0;
    double value = 0.0;

    bool operator==(AngularOpticalPoint const&) const = default;

    RECORD_META(AngularOpticalPoint, SM_EXPOSE_RW(angle), SM_EXPOSE_RW(value))
};

class AngularOpticalTableModel
    : public Support::StructTableModel<AngularOpticalPoint> {
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    explicit AngularOpticalTableModel(QObject* parent = nullptr);

    bool setData(QModelIndex const& index,
                 QVariant const&    value,
                 int                role = Qt::EditRole) override;

    std::vector<SD::AngularTablePoint> points() const;
    void reset_points(std::vector<SD::AngularTablePoint> const& points);

public slots:
    void append();
    void remove(int index);
    int  count() const;

signals:
    void changed();
    void countChanged();
};

/// QML-facing editor for one optical side of a material group.
///
/// Mutating setters patch MaterialComponent in the observed database.
class OpticalPropertiesObject : public QObject, public DatabaseObserver {
    Q_OBJECT

    entt::entity m_current_group = entt::null;
    bool         m_back          = false;

    SD::OpticalSide m_side = SD::OpticalSide::Front;

    AngularOpticalTableModel* m_reflectivity_table   = nullptr;
    AngularOpticalTableModel* m_transmissivity_table = nullptr;

    SolTrace::Data::OpticalPropertySet const* get_properties() const;

    template <class F>
    void patch_properties(F&& f) {
        if (!database()) return;

        database()->material_parameters.try_patch(
            m_current_group,
            [this, &f](MaterialComponent& c) { f(c.optics, m_side); });
    }

private:
    // TODO: interaction is now unified front and back.

    Q_PROPERTY(qint32 interaction_type READ interaction_type WRITE
                   set_interaction_type NOTIFY interaction_type_changed)

    Q_PROPERTY(
        qint32 error_distribution_type READ error_distribution_type WRITE
            set_error_distribution_type NOTIFY error_distribution_type_changed)

    Q_PROPERTY(double transmissivity READ transmissivity WRITE
                   set_transmissivity NOTIFY transmissivity_changed)

    Q_PROPERTY(double reflectivity READ reflectivity WRITE set_reflectivity
                   NOTIFY reflectivity_changed)

    Q_PROPERTY(
        bool reflectivity_uses_table READ reflectivity_uses_table WRITE
            set_reflectivity_uses_table NOTIFY reflectivity_uses_table_changed)

    Q_PROPERTY(bool transmissivity_uses_table READ transmissivity_uses_table
                   WRITE set_transmissivity_uses_table NOTIFY
                       transmissivity_uses_table_changed)

    Q_PROPERTY(AngularOpticalTableModel* reflectivity_table READ
                   reflectivity_table CONSTANT)

    Q_PROPERTY(AngularOpticalTableModel* transmissivity_table READ
                   transmissivity_table CONSTANT)

    Q_PROPERTY(double slope_error READ slope_error WRITE set_slope_error NOTIFY
                   slope_error_changed)

    Q_PROPERTY(double specularity_error READ specularity_error WRITE
                   set_specularity_error NOTIFY specularity_error_changed)

    Q_PROPERTY(
        double refraction_index_front READ refraction_index_front WRITE
            set_refraction_index_front NOTIFY refraction_index_front_changed)

    Q_PROPERTY(
        double refraction_index_back READ refraction_index_back WRITE
            set_refraction_index_back NOTIFY refraction_index_back_changed)

    // UX helpers
    Q_PROPERTY(double transmissivity_min READ transmissivity_min CONSTANT)
    Q_PROPERTY(double transmissivity_max READ transmissivity_max CONSTANT)
    Q_PROPERTY(double reflectivity_min READ reflectivity_min CONSTANT)
    Q_PROPERTY(double reflectivity_max READ reflectivity_max CONSTANT)

    void set_new_database_connections(Database* ptr) override;

    void trigger_all_changed();
    void sync_table_models();
    void update_reflectivity_table();
    void update_transmissivity_table();

private slots:
    void parameters_changed(entt::entity);

public:
    explicit OpticalPropertiesObject(bool back, QObject* parent = nullptr);
    ~OpticalPropertiesObject() override;

    /// Observe database material group and selected optical side.
    void set(Database*, entt::entity group);

public:
    qint32 interaction_type() const;
    qint32 error_distribution_type() const;

    double transmissivity() const;
    double reflectivity() const;
    bool   reflectivity_uses_table() const;
    bool   transmissivity_uses_table() const;
    double slope_error() const;
    double specularity_error() const;
    double refraction_index_front() const;
    double refraction_index_back() const;

    AngularOpticalTableModel* reflectivity_table() const {
        return m_reflectivity_table;
    }
    AngularOpticalTableModel* transmissivity_table() const {
        return m_transmissivity_table;
    }

public slots:
    /// Update the interaction model, for example
    /// absorption/reflection/refraction.
    void set_interaction_type(qint32 v);

    /// Update the optical error distribution model.
    void set_error_distribution_type(qint32 v);

    /// Update transmissivity, clamped by the UI to [0, 1].
    void set_transmissivity(double v);

    /// Update reflectivity, clamped by the UI to [0, 1].
    void set_reflectivity(double v);

    /// Toggle angle-dependent reflectivity without discarding either value.
    void set_reflectivity_uses_table(bool enabled);

    /// Toggle angle-dependent transmissivity without discarding either value.
    void set_transmissivity_uses_table(bool enabled);

    /// Update slope error.
    void set_slope_error(double v);

    /// Update specularity error.
    void set_specularity_error(double v);

    /// Update incident-side refraction index.
    void set_refraction_index_front(double v);

    /// Update outgoing-side refraction index.
    void set_refraction_index_back(double v);

    // presets
    /// Apply an ideal absorber preset.
    void set_ideal_absorption();

    /// Apply an ideal mirror preset.
    void set_ideal_reflection();

    /// Apply an ideal transmitter preset.
    void set_ideal_transmission();

    /// Apply an ideal transmitter preset with explicit refraction indices.
    void set_ideal_transmission_with_indices(double n_front, double n_back);

signals:
    void interaction_type_changed();
    void error_distribution_type_changed();

    void transmissivity_changed();
    void reflectivity_changed();
    void reflectivity_uses_table_changed();
    void transmissivity_uses_table_changed();
    void slope_error_changed();
    void specularity_error_changed();
    void refraction_index_front_changed();
    void refraction_index_back_changed();

private:
    double transmissivity_min() const { return 0.0; }
    double transmissivity_max() const { return 1.0; }
    double reflectivity_min() const { return 0.0; }
    double reflectivity_max() const { return 1.0; }
};

} // namespace SolTrace::GUI::Data
