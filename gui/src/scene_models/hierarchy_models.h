#pragma once

#include "scene_models/entity_name_model.h"

#include <QAbstractItemModel>
#include <QPointer>
#include <QVector>

namespace SolTrace::GUI::Data {

/// A hierarchical model of every element in a database.
///
/// QModelIndex internal IDs contain the element entity, so indexes can be
/// reconstructed cheaply when the view needs to reveal a selected element.
class ElementTreeModel : public QAbstractItemModel {
    Q_OBJECT

    QPointer<Database> m_host;
    QVector<Entity>    m_roots;
    bool               m_rebuild_scheduled = false;

    static quintptr encode(Entity entity);
    static Entity   decode(QModelIndex const& index);

    int row_of(Entity entity) const;

private slots:
    void schedule_rebuild();
    void rebuild();
    void identity_changed(entt::entity entity);

public:
    explicit ElementTreeModel(QObject* parent = nullptr);
    ~ElementTreeModel() override = default;

    QModelIndex index(int                row,
                      int                column,
                      QModelIndex const& parent = { }) const override;
    QModelIndex parent(QModelIndex const& child) const override;
    int rowCount(QModelIndex const& parent = { }) const override;
    int columnCount(QModelIndex const& parent = { }) const override;
    QVariant data(QModelIndex const& index,
                  int                role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    /// Observe a database and rebuild the hierarchy.
    void reset(Database* database);

    /// Return the model index for entity, including nested entities.
    Q_INVOKABLE QModelIndex index_for_entity(Entity entity) const;
};

} // namespace SolTrace::GUI::Data
