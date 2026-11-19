#pragma once

#include "scene_models/entity_name_model.h"

namespace SolTrace::GUI::Data {

/// A model providing all element entities in a database.
class AllElementsModel : public Support::StructModelAdapter<EntityNamePair> {
    Q_OBJECT

    QPointer<Database> m_host;

    std::unordered_map<entt::entity, int> m_reverse;

    QVector<EntityNamePair> rebuild_lists();

private slots:
    void recompute();
    void record_changed(entt::entity);

public:
    explicit AllElementsModel(QObject* parent = nullptr);
    ~AllElementsModel() override = default;

    /// Observe a database and rebuild the element list.
    void reset(Database* database);
};

} // namespace SolTrace::GUI::Data
