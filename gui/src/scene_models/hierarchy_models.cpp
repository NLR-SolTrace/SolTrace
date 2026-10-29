#include "scene_models/hierarchy_models.h"

#include "data_io/database_notification.h"

#include <QTimer>

namespace SolTrace::GUI::Data {

namespace {
constexpr int name_role = ROLE_FOR_MEMBER(EntityNamePair, name);
constexpr int entity_role = ROLE_FOR_MEMBER(EntityNamePair, entity);
constexpr int has_children_role = ROLE_FOR_MEMBER(EntityNamePair, has_children);
}

quintptr ElementTreeModel::encode(Entity entity) {
    return static_cast<quintptr>(entt::to_integral(entity.value)) + 1;
}

Entity ElementTreeModel::decode(QModelIndex const& index) {
    if (!index.isValid() || index.internalId() == 0) return { };

    using EntityValue = std::underlying_type_t<entt::entity>;
    return Entity(static_cast<entt::entity>(
        static_cast<EntityValue>(index.internalId() - 1)));
}

int ElementTreeModel::row_of(Entity entity) const {
    if (!m_host || !m_host->valid(entity)) return -1;

    auto const parent_entity = Entity(m_host->parent_of(entity));
    if (!parent_entity.is_valid()) return m_roots.indexOf(entity);

    auto const siblings = m_host->children_of(parent_entity);
    for (qsizetype row = 0; row < std::ssize(siblings); ++row) {
        if (siblings[static_cast<size_t>(row)] == entity) {
            return static_cast<int>(row);
        }
    }
    return -1;
}

QModelIndex ElementTreeModel::index(int                row,
                                    int                column,
                                    QModelIndex const& parent_index) const {
    if (!m_host || row < 0 || column != 0) return { };

    Entity entity;
    if (!parent_index.isValid()) {
        if (row >= m_roots.size()) return { };
        entity = m_roots[row];
    } else {
        auto const parent_entity = decode(parent_index);
        if (!m_host->valid(parent_entity)) return { };

        auto const children = m_host->children_of(parent_entity);
        if (row >= std::ssize(children)) return { };
        entity = children[static_cast<size_t>(row)];
    }

    if (!m_host->valid(entity)) return { };
    return createIndex(row, column, encode(entity));
}

QModelIndex ElementTreeModel::parent(QModelIndex const& child) const {
    if (!m_host || !child.isValid()) return { };

    auto const entity = decode(child);
    if (!m_host->valid(entity)) return { };

    auto const parent_entity = Entity(m_host->parent_of(entity));
    if (!parent_entity.is_valid()) return { };

    auto const row = row_of(parent_entity);
    if (row < 0) return { };
    return createIndex(row, 0, encode(parent_entity));
}

int ElementTreeModel::rowCount(QModelIndex const& parent_index) const {
    if (!m_host || parent_index.column() > 0) return 0;
    if (!parent_index.isValid()) return m_roots.size();

    auto const parent_entity = decode(parent_index);
    if (!m_host->valid(parent_entity)) return 0;
    return static_cast<int>(m_host->children_of(parent_entity).size());
}

int ElementTreeModel::columnCount(QModelIndex const&) const {
    return 1;
}

QVariant ElementTreeModel::data(QModelIndex const& model_index, int role) const {
    if (!m_host || !model_index.isValid()) return { };

    auto const entity = decode(model_index);
    if (!m_host->valid(entity)) return { };

    if (role == Qt::DisplayRole || role == name_role) {
        return m_host->name_of(entity);
    }
    if (role == entity_role) return QVariant::fromValue(entity);
    if (role == has_children_role) {
        return !m_host->children_of(entity).empty();
    }
    return { };
}

QHash<int, QByteArray> ElementTreeModel::roleNames() const {
    auto roles = Support::get_name_map<EntityNamePair>();
    roles.insert(Qt::DisplayRole, QByteArrayLiteral("display"));
    return roles;
}

void ElementTreeModel::schedule_rebuild() {
    if (m_rebuild_scheduled) return;
    m_rebuild_scheduled = true;
    QTimer::singleShot(0, this, &ElementTreeModel::rebuild);
}

void ElementTreeModel::rebuild() {
    m_rebuild_scheduled = false;

    beginResetModel();
    m_roots.clear();
    if (m_host) {
        auto view = m_host->as_registry().view<ElementComponent>(
            entt::exclude<ChildOfComponent>);
        m_roots.reserve(static_cast<qsizetype>(view.size_hint()));
        for (auto [entity] : view.each()) m_roots.push_back(Entity(entity));
    }
    endResetModel();
}

void ElementTreeModel::identity_changed(entt::entity raw_entity) {
    auto const model_index = index_for_entity(Entity(raw_entity));
    if (!model_index.isValid()) return;
    emit dataChanged(model_index, model_index, { Qt::DisplayRole, name_role });
}

ElementTreeModel::ElementTreeModel(QObject* parent)
    : QAbstractItemModel(parent) { }

void ElementTreeModel::reset(Database* database) {
    if (m_host) {
        disconnect(m_host->identity.self(), nullptr, this, nullptr);
        disconnect(m_host->parent.self(), nullptr, this, nullptr);
        disconnect(m_host->children.self(), nullptr, this, nullptr);
        disconnect(m_host->element_tag.self(), nullptr, this, nullptr);
    }

    m_host = database;
    rebuild();
    if (!database) return;

    connect(database->identity.self(),
            &ComponentAPIBase::changed,
            this,
            &ElementTreeModel::identity_changed);

    auto connect_structure = [this](ComponentAPIBase* api) {
        connect(api,
                &ComponentAPIBase::changed,
                this,
                &ElementTreeModel::schedule_rebuild);
        connect(api,
                &ComponentAPIBase::removed,
                this,
                &ElementTreeModel::schedule_rebuild);
    };
    connect_structure(database->parent.self());
    connect_structure(database->children.self());
    connect_structure(database->element_tag.self());
}

QModelIndex ElementTreeModel::index_for_entity(Entity entity) const {
    if (!m_host || !m_host->valid(entity)) return { };
    if (!m_host->as_registry().all_of<ElementComponent>(entity)) return { };

    auto const row = row_of(entity);
    if (row < 0) return { };
    return createIndex(row, 0, encode(entity));
}

} // namespace SolTrace::GUI::Data
