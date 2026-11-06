#include "opticaleditor.h"

#include <magic_enum/magic_enum.hpp>

#include <algorithm>
#include <cmath>
#include <utility>

namespace SolTrace::GUI::Data {

namespace {

constexpr double kMaximumAngleMrad = 3141.592;
constexpr double kRightAngleMrad   = 1570.79632679489661923;

} // namespace

AngularOpticalTableModel::AngularOpticalTableModel(QObject* parent)
    : StructTableModel(parent) { }

bool AngularOpticalTableModel::setData(QModelIndex const& index,
                                       QVariant const&    value,
                                       int                role) {
    if (!index.isValid() || index.row() < 0 ||
        index.row() >= m_records.size()) {
        return false;
    }

    bool   ok        = false;
    double candidate = value.toDouble(&ok);
    if (!ok || !std::isfinite(candidate)) return false;

    if (index.column() == 0) {
        if (candidate < 0.0 || candidate > kMaximumAngleMrad) return false;
        if (index.row() > 0 && candidate <= m_records[index.row() - 1].angle) {
            return false;
        }
        if (index.row() + 1 < m_records.size() &&
            candidate >= m_records[index.row() + 1].angle) {
            return false;
        }
    } else if (index.column() == 1) {
        if (candidate < 0.0 || candidate > 1.0) return false;
    } else {
        return false;
    }

    if (!Support::StructTableModel<AngularOpticalPoint>::setData(
            index, candidate, role)) {
        return false;
    }

    emit changed();
    return true;
}

std::vector<SD::AngularTablePoint> AngularOpticalTableModel::points() const {
    std::vector<SD::AngularTablePoint> result;
    result.reserve(m_records.size());

    for (auto const& point : m_records) {
        result.push_back({ .angle = point.angle, .value = point.value });
    }

    return result;
}

void AngularOpticalTableModel::reset_points(
    std::vector<SD::AngularTablePoint> const& points) {
    QVector<AngularOpticalPoint> records;
    records.reserve(static_cast<qsizetype>(points.size()));

    for (auto const& point : points) {
        records.push_back({ .angle = point.angle, .value = point.value });
    }

    if (records == m_records) return;

    auto old_count = m_records.size();
    Support::StructTableModel<AngularOpticalPoint>::reset(std::move(records));
    if (old_count != m_records.size()) emit countChanged();
}

void AngularOpticalTableModel::append() {
    double angle = 0.0;
    double value = 1.0;

    if (!m_records.isEmpty()) {
        if (m_records.back().angle >= kMaximumAngleMrad) return;
        angle = std::min(m_records.back().angle + 100.0, kMaximumAngleMrad);
        value = m_records.back().value;
    }

    Support::StructTableModel<AngularOpticalPoint>::append(
        { .angle = angle, .value = value });
    emit countChanged();
    emit changed();
}

void AngularOpticalTableModel::remove(int index) {
    if (m_records.size() <= 1 || index < 0 || index >= m_records.size()) return;

    remove_at(index);
    emit countChanged();
    emit changed();
}

int AngularOpticalTableModel::count() const {
    return rowCount();
}

OpticalPropertiesObject::OpticalPropertiesObject(bool back, QObject* parent)
    : QObject(parent),
      m_current_group(entt::null),
      m_back(back),
      m_side(m_back ? SD::OpticalSide::Back : SD::OpticalSide::Front),
      m_reflectivity_table(new AngularOpticalTableModel(this)),
      m_transmissivity_table(new AngularOpticalTableModel(this)) {
    connect(m_reflectivity_table,
            &AngularOpticalTableModel::changed,
            this,
            &OpticalPropertiesObject::update_reflectivity_table);
    connect(m_transmissivity_table,
            &AngularOpticalTableModel::changed,
            this,
            &OpticalPropertiesObject::update_transmissivity_table);
}

OpticalPropertiesObject::~OpticalPropertiesObject() = default;

void OpticalPropertiesObject::trigger_all_changed() {
    qDebug() << Q_FUNC_INFO << m_back;
    sync_table_models();
    emit interaction_type_changed();
    emit error_distribution_type_changed();
    emit transmissivity_changed();
    emit reflectivity_changed();
    emit reflectivity_uses_table_changed();
    emit transmissivity_uses_table_changed();
    emit slope_error_changed();
    emit specularity_error_changed();
    emit refraction_index_front_changed();
    emit refraction_index_back_changed();
}

void OpticalPropertiesObject::sync_table_models() {
    auto const* properties = get_properties();

    if (!properties) {
        m_reflectivity_table->reset_points({ });
        m_transmissivity_table->reset_points({ });
        return;
    }

    m_reflectivity_table->reset_points(
        properties->get_reflectivity_table(m_side));
    m_transmissivity_table->reset_points(
        properties->get_transmissivity_table(m_side));
}

void OpticalPropertiesObject::update_reflectivity_table() {
    auto points = m_reflectivity_table->points();
    if (points.empty()) return;

    patch_properties([this, &points](SD::OpticalPropertySet& prop, auto) {
        auto was_enabled = prop.uses_reflectivity_table(m_side);
        prop.set_reflectivity_table(m_side, points);
        if (!was_enabled) prop.disable_reflectivity_table(m_side);
    });
}

void OpticalPropertiesObject::update_transmissivity_table() {
    auto points = m_transmissivity_table->points();
    if (points.empty()) return;

    patch_properties([this, &points](SD::OpticalPropertySet& prop, auto) {
        auto was_enabled = prop.uses_transmissivity_table(m_side);
        prop.set_transmissivity_table(m_side, points);
        if (!was_enabled) prop.disable_transmissivity_table(m_side);
    });
}

void OpticalPropertiesObject::set(Database* db, entt::entity group) {
    observe(db);

    if (!db or !db->material_parameters.get(group)) {
        m_current_group = entt::null;
        trigger_all_changed();
        return;
    }

    m_current_group = group;

    trigger_all_changed();
}

void OpticalPropertiesObject::set_new_database_connections(Database* ptr) {
    add_connection(connect(ptr->material_parameters.self(),
                           &ComponentAPIBase::changed,
                           this,
                           &OpticalPropertiesObject::parameters_changed));
}

void OpticalPropertiesObject::parameters_changed(entt::entity e) {
    if (this->m_current_group != e) return;

    trigger_all_changed();
}

SolTrace::Data::OpticalPropertySet const*
OpticalPropertiesObject::get_properties() const {
    if (!database()) return nullptr;

    auto* ptr = database()->material_parameters.get(m_current_group);

    if (!ptr) return nullptr;

    return &ptr->optics;
}

qint32 OpticalPropertiesObject::interaction_type() const {
    auto* ptr = get_properties();

    if (!ptr) return static_cast<qint32>(SD::InteractionType::UNKNOWN);

    return static_cast<qint32>(ptr->get_interaction_type());
}

qint32 OpticalPropertiesObject::error_distribution_type() const {
    auto* ptr = get_properties();

    if (!ptr) return static_cast<qint32>(SD::DistributionType::UNKNOWN);

    return static_cast<qint32>(ptr->get_error_distribution(m_side));
}

double OpticalPropertiesObject::transmissivity() const {
    auto* ptr = get_properties();

    if (!ptr) return { };

    return ptr->get_transmissivity(m_side);
}

double OpticalPropertiesObject::reflectivity() const {
    auto* ptr = get_properties();

    if (!ptr) return { };

    return ptr->get_reflectivity(m_side);
}

bool OpticalPropertiesObject::reflectivity_uses_table() const {
    auto const* ptr = get_properties();
    return ptr && ptr->uses_reflectivity_table(m_side);
}

bool OpticalPropertiesObject::transmissivity_uses_table() const {
    auto const* ptr = get_properties();
    return ptr && ptr->uses_transmissivity_table(m_side);
}

double OpticalPropertiesObject::slope_error() const {
    auto* ptr = get_properties();

    if (!ptr) return { };

    return ptr->get_slope_error(m_side);
}

double OpticalPropertiesObject::specularity_error() const {
    auto* ptr = get_properties();

    if (!ptr) return { };

    return ptr->get_specularity_error(m_side);
}

double OpticalPropertiesObject::refraction_index_front() const {
    auto* ptr = get_properties();

    if (!ptr) return { };

    double front;
    double back;

    ptr->get_refraction_indices(front, back);

    return front;
}

double OpticalPropertiesObject::refraction_index_back() const {
    auto* ptr = get_properties();

    if (!ptr) return { };

    double front;
    double back;

    ptr->get_refraction_indices(front, back);

    return back;
}


void OpticalPropertiesObject::set_interaction_type(qint32 v) {
    patch_properties([this, &v](SD::OpticalPropertySet& prop, auto side) {
        auto value = static_cast<SD::InteractionType>(v);
        if (!magic_enum::enum_contains(value)) {
            value = SD::InteractionType::REFLECTION;
        }

        prop.set_interaction_type(value);
        emit interaction_type_changed();
    });
}

void OpticalPropertiesObject::set_error_distribution_type(qint32 v) {
    patch_properties([this, &v](SD::OpticalPropertySet& prop, auto side) {
        auto value = static_cast<SD::DistributionType>(v);
        if (!magic_enum::enum_contains(value)) {
            value = SD::DistributionType::DIFFUSE;
        }

        auto slope_error = prop.get_slope_error(m_side);
        auto spec_error  = prop.get_specularity_error(m_side);

        prop.set_errors(m_side, value, slope_error, spec_error);

        emit error_distribution_type_changed();
    });
}

#define SETTER(MEMBER)                                                         \
    patch_properties([this, &v](SD::OpticalPropertySet& prop, auto side) {     \
        prop.set_##MEMBER(m_side, v);                                          \
        emit MEMBER##_changed();                                               \
    });

void OpticalPropertiesObject::set_transmissivity(double v) {
    patch_properties([this, &v](SD ::OpticalPropertySet& prop, auto side) {
        prop.set_transmissivity(m_side, v);
        transmissivity_changed();
    });
}

void OpticalPropertiesObject::set_reflectivity(double v) {
    patch_properties([this, &v](SD ::OpticalPropertySet& prop, auto side) {
        prop.set_reflectivity(m_side, v);
        reflectivity_changed();
    });
}

void OpticalPropertiesObject::set_reflectivity_uses_table(bool enabled) {
    if (enabled == reflectivity_uses_table()) return;

    if (enabled && m_reflectivity_table->count() == 0) {
        auto value = reflectivity();
        m_reflectivity_table->reset_points(
            { { 0.0, value }, { kRightAngleMrad, value } });
    }

    auto points = m_reflectivity_table->points();
    patch_properties(
        [this, enabled, &points](SD::OpticalPropertySet& prop, auto) {
            if (enabled) {
                prop.set_reflectivity_table(m_side, points);
            } else {
                prop.disable_reflectivity_table(m_side);
            }
        });
}

void OpticalPropertiesObject::set_transmissivity_uses_table(bool enabled) {
    if (enabled == transmissivity_uses_table()) return;

    if (enabled && m_transmissivity_table->count() == 0) {
        auto value = transmissivity();
        m_transmissivity_table->reset_points(
            { { 0.0, value }, { kRightAngleMrad, value } });
    }

    auto points = m_transmissivity_table->points();
    patch_properties(
        [this, enabled, &points](SD::OpticalPropertySet& prop, auto) {
            if (enabled) {
                prop.set_transmissivity_table(m_side, points);
            } else {
                prop.disable_transmissivity_table(m_side);
            }
        });
}

void OpticalPropertiesObject::set_slope_error(double v) {
    patch_properties([this, &v](SD ::OpticalPropertySet& prop, auto side) {
        auto type       = prop.get_error_distribution(m_side);
        auto spec_error = prop.get_specularity_error(m_side);

        prop.set_errors(m_side, type, v, spec_error);

        slope_error_changed();
    });
}

void OpticalPropertiesObject::set_specularity_error(double v) {
    patch_properties([this, &v](SD ::OpticalPropertySet& prop, auto side) {
        auto type        = prop.get_error_distribution(m_side);
        auto slope_error = prop.get_slope_error(m_side);

        prop.set_errors(m_side, type, slope_error, v);

        specularity_error_changed();
    });
}

void OpticalPropertiesObject::set_refraction_index_front(double v) {
    patch_properties([this, &v](SD ::OpticalPropertySet& prop, auto side) {
        double front;
        double back;

        prop.get_refraction_indices(front, back);

        prop.set_refraction_indices(v, back);
        emit refraction_index_front_changed();
    });
}

void OpticalPropertiesObject::set_refraction_index_back(double v) {
    patch_properties([this, &v](SD ::OpticalPropertySet& prop, auto side) {
        double front;
        double back;

        prop.get_refraction_indices(front, back);

        prop.set_refraction_indices(front, v);
        emit refraction_index_back_changed();
    });
}

void OpticalPropertiesObject::set_ideal_absorption() {
    patch_properties([this](SD::OpticalPropertySet& prop, auto side) {
        prop.set_ideal_absorption(m_side);
    });
    trigger_all_changed();
}

void OpticalPropertiesObject::set_ideal_reflection() {
    patch_properties([this](SD::OpticalPropertySet& prop, auto side) {
        prop.set_ideal_reflection(m_side);
    });
    trigger_all_changed();
}

void OpticalPropertiesObject::set_ideal_transmission() {
    patch_properties([](SD::OpticalPropertySet& prop, auto) {
        prop.set_ideal_transmission();
    });
    trigger_all_changed();
}

void OpticalPropertiesObject::set_ideal_transmission_with_indices(
    double n_front,
    double n_back) {

    patch_properties([n_front, n_back](SD::OpticalPropertySet& prop, auto) {
        prop.set_ideal_transmission(n_front, n_back);
    });
    trigger_all_changed();
}

} // namespace SolTrace::GUI::Data
