#include "script_references.h"

#include "data/components.h"
#include "data/database.h"

#include <QDebug>
#include <QSet>

#include <algorithm>
#include <utility>

namespace SolTrace::GUI::Scripting {

namespace {

using SolTrace::GUI::Data::Entity;
using SolTrace::GUI::Data::ScriptReference;
using SolTrace::GUI::Data::ScriptReferenceComponent;
using SolTrace::GUI::Data::ScriptReferenceKind;

QString normalize_name(QString name) {
    return name.trimmed();
}

bool matches(ScriptReference const& reference,
             QString const&         scope,
             QString const&         name,
             ScriptReferenceKind    kind) {
    return reference.scope == scope && reference.name == name &&
           reference.kind == kind;
}

QVector<Entity> referenced_entities(SolTrace::GUI::Data::Database* database,
                                    QString const&                 scope,
                                    QString const&                 name,
                                    ScriptReferenceKind            kind) {
    QVector<Entity> result;
    if (!database) return result;

    auto const& registry = database->as_registry();
    for (auto const& [entity, component] :
         registry.view<ScriptReferenceComponent>().each()) {
        if (std::any_of(component.references.cbegin(),
                        component.references.cend(),
                        [&](ScriptReference const& reference) {
                            return matches(reference, scope, name, kind);
                        })) {
            result.push_back(entity);
        }
    }
    return result;
}

bool has_reference(SolTrace::GUI::Data::Database* database,
                   Entity                         entity,
                   ScriptReference const&         reference) {
    if (!database || !database->valid(entity)) return false;
    auto const* component =
        database->as_registry().try_get<ScriptReferenceComponent>(entity);
    return component && component->references.contains(reference);
}

bool add_reference(SolTrace::GUI::Data::Database* database,
                   Entity                         entity,
                   ScriptReference                reference) {
    if (!database || !database->valid(entity)) return false;
    if (has_reference(database, entity, reference)) return true;

    auto& registry = database->as_registry();
    if (auto* component = registry.try_get<ScriptReferenceComponent>(entity)) {
        component->references.push_back(std::move(reference));
    } else {
        registry.emplace<ScriptReferenceComponent>(
            entity,
            ScriptReferenceComponent {
                .references = { std::move(reference) },
            });
    }
    return true;
}

bool remove_reference(SolTrace::GUI::Data::Database* database,
                      Entity                         entity,
                      ScriptReference const&         reference) {
    if (!database || !database->valid(entity)) return false;

    auto& registry  = database->as_registry();
    auto* component = registry.try_get<ScriptReferenceComponent>(entity);
    if (!component) return false;

    auto const old_size = component->references.size();
    auto [first, last]  = std::ranges::remove(component->references, reference);
    component->references.erase(first, last);
    auto const removed = component->references.size() != old_size;

    if (component->references.isEmpty()) {
        registry.remove<ScriptReferenceComponent>(entity);
    }
    return removed;
}

bool clear_reference(SolTrace::GUI::Data::Database* database,
                     QString const&                 scope,
                     QString const&                 name,
                     ScriptReferenceKind            kind) {
    auto const entities  = referenced_entities(database, scope, name, kind);
    bool       removed   = false;
    auto const reference = ScriptReference {
        .scope = scope,
        .name  = name,
        .kind  = kind,
    };
    for (auto entity : entities) {
        removed = remove_reference(database, entity, reference) || removed;
    }
    return removed;
}

QStringList names_for(SolTrace::GUI::Data::Database* database,
                      QString const&                 scope,
                      ScriptReferenceKind            kind) {
    QSet<QString> names;
    if (!database) return { };

    for (auto const& [entity, component] :
         database->as_registry().view<ScriptReferenceComponent>().each()) {
        Q_UNUSED(entity);
        for (auto const& reference : component.references) {
            if (reference.scope == scope && reference.kind == kind) {
                names.insert(reference.name);
            }
        }
    }

    auto result = names.values();
    result.sort();
    return result;
}

} // namespace

ScriptReferenceScope::ScriptReferenceScope(
    SolTrace::GUI::Data::Database* database,
    QString                        name,
    QObject*                       parent)
    : QObject(parent), m_database(database), m_name(std::move(name)) { }

QString ScriptReferenceScope::name() const {
    return m_name;
}

void ScriptReferenceScope::set_database(
    SolTrace::GUI::Data::Database* database) {
    m_database = database;
    for (auto* reference : std::as_const(m_singles)) {
        reference->set_database(database);
    }
    for (auto* reference : std::as_const(m_groups)) {
        reference->set_database(database);
    }
}

ScriptSingleReference* ScriptReferenceScope::single(QString name) {
    name = normalize_name(std::move(name));
    if (name.isEmpty()) {
        qWarning() << "Script reference names cannot be empty";
        return nullptr;
    }
    if (auto* existing = m_singles.value(name)) return existing;

    auto* reference = new ScriptSingleReference(m_database, m_name, name, this);
    m_singles.insert(std::move(name), reference);
    return reference;
}

ScriptReferenceGroup* ScriptReferenceScope::group(QString name) {
    name = normalize_name(std::move(name));
    if (name.isEmpty()) {
        qWarning() << "Script reference names cannot be empty";
        return nullptr;
    }
    if (auto* existing = m_groups.value(name)) return existing;

    auto* reference = new ScriptReferenceGroup(m_database, m_name, name, this);
    m_groups.insert(std::move(name), reference);
    return reference;
}

QStringList ScriptReferenceScope::single_names() const {
    return names_for(m_database, m_name, ScriptReferenceKind::Single);
}

QStringList ScriptReferenceScope::group_names() const {
    return names_for(m_database, m_name, ScriptReferenceKind::Group);
}

ScriptSingleReference::ScriptSingleReference(
    SolTrace::GUI::Data::Database* database,
    QString                        scope,
    QString                        name,
    QObject*                       parent)
    : QObject(parent),
      m_database(database),
      m_scope(std::move(scope)),
      m_name(std::move(name)) { }

void ScriptSingleReference::set_database(
    SolTrace::GUI::Data::Database* database) {
    m_database = database;
}

Entity ScriptSingleReference::get() const {
    auto const entities = referenced_entities(
        m_database, m_scope, m_name, ScriptReferenceKind::Single);
    return entities.isEmpty() ? Entity { } : entities.front();
}

bool ScriptSingleReference::set(Entity entity) {
    if (!m_database || !m_database->valid(entity)) return false;

    clear_reference(m_database, m_scope, m_name, ScriptReferenceKind::Single);
    return add_reference(m_database,
                         entity,
                         ScriptReference {
                             .scope = m_scope,
                             .name  = m_name,
                             .kind  = ScriptReferenceKind::Single,
                         });
}

bool ScriptSingleReference::clear() {
    return clear_reference(
        m_database, m_scope, m_name, ScriptReferenceKind::Single);
}

ScriptReferenceGroup::ScriptReferenceGroup(
    SolTrace::GUI::Data::Database* database,
    QString                        scope,
    QString                        name,
    QObject*                       parent)
    : QObject(parent),
      m_database(database),
      m_scope(std::move(scope)),
      m_name(std::move(name)) { }

void ScriptReferenceGroup::set_database(
    SolTrace::GUI::Data::Database* database) {
    m_database = database;
}

QVector<Entity> ScriptReferenceGroup::entities() const {
    return referenced_entities(
        m_database, m_scope, m_name, ScriptReferenceKind::Group);
}

bool ScriptReferenceGroup::add(Entity entity) {
    return add_reference(m_database,
                         entity,
                         ScriptReference {
                             .scope = m_scope,
                             .name  = m_name,
                             .kind  = ScriptReferenceKind::Group,
                         });
}

bool ScriptReferenceGroup::remove(Entity entity) {
    return remove_reference(m_database,
                            entity,
                            ScriptReference {
                                .scope = m_scope,
                                .name  = m_name,
                                .kind  = ScriptReferenceKind::Group,
                            });
}

bool ScriptReferenceGroup::contains(Entity entity) const {
    return has_reference(m_database,
                         entity,
                         ScriptReference {
                             .scope = m_scope,
                             .name  = m_name,
                             .kind  = ScriptReferenceKind::Group,
                         });
}

bool ScriptReferenceGroup::clear() {
    return clear_reference(
        m_database, m_scope, m_name, ScriptReferenceKind::Group);
}

ScriptReferenceInterface::ScriptReferenceInterface(
    SolTrace::GUI::Data::Database* database,
    QObject*                       parent)
    : QObject(parent), m_database(database) { }

void ScriptReferenceInterface::set_database(
    SolTrace::GUI::Data::Database* database) {
    if (m_database == database) return;
    m_database = database;
    for (auto* scope : std::as_const(m_scopes)) {
        scope->set_database(database);
    }
}

ScriptReferenceScope* ScriptReferenceInterface::scope(QString name) {
    name = normalize_name(std::move(name));
    if (name.isEmpty()) {
        qWarning() << "Script reference scopes cannot be empty";
        return nullptr;
    }
    if (auto* existing = m_scopes.value(name)) return existing;

    auto* reference_scope = new ScriptReferenceScope(m_database, name, this);
    m_scopes.insert(std::move(name), reference_scope);
    return reference_scope;
}

QStringList ScriptReferenceInterface::scopes() const {
    QSet<QString> names;
    if (!m_database) return { };

    for (auto const& [entity, component] :
         m_database->as_registry().view<ScriptReferenceComponent>().each()) {
        Q_UNUSED(entity);
        for (auto const& reference : component.references) {
            names.insert(reference.scope);
        }
    }

    auto result = names.values();
    result.sort();
    return result;
}

} // namespace SolTrace::GUI::Scripting
