#pragma once

#include "data/database.h"

#include <QHash>
#include <QObject>
#include <QPointer>
#include <QString>
#include <QStringList>
#include <QVector>

namespace SolTrace::GUI::Scripting {

class ScriptSingleReference;
class ScriptReferenceGroup;

/// A named collection of shared entity references in one database.
class ScriptReferenceScope : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString name READ name CONSTANT FINAL)

    QPointer<SolTrace::GUI::Data::Database> m_database;
    QString                                 m_name;
    QHash<QString, ScriptSingleReference*>  m_singles;
    QHash<QString, ScriptReferenceGroup*>   m_groups;

public:
    explicit ScriptReferenceScope(SolTrace::GUI::Data::Database* database,
                                  QString                        name,
                                  QObject* parent = nullptr);

    QString name() const;
    void    set_database(SolTrace::GUI::Data::Database* database);

public slots:
    /// Return the one-entity reference with this name.
    ScriptSingleReference* single(QString name);

    /// Return the multi-entity reference with this name.
    ScriptReferenceGroup* group(QString name);

    /// Discover the single-reference names currently used in this scope.
    QStringList single_names() const;

    /// Discover the group-reference names currently used in this scope.
    QStringList group_names() const;
};

/// A reference that addresses at most one entity.
class ScriptSingleReference : public QObject {
    Q_OBJECT

    QPointer<SolTrace::GUI::Data::Database> m_database;
    QString                                 m_scope;
    QString                                 m_name;

public:
    explicit ScriptSingleReference(SolTrace::GUI::Data::Database* database,
                                   QString                        scope,
                                   QString                        name,
                                   QObject* parent = nullptr);

    void set_database(SolTrace::GUI::Data::Database* database);

public slots:
    /// Return the referenced entity, or an invalid handle when unset.
    SolTrace::GUI::Data::Entity get() const;

    /// Replace this reference without deleting the previously referenced
    /// entity.
    bool set(SolTrace::GUI::Data::Entity entity);

    /// Remove this reference without deleting its entity.
    bool clear();
};

/// A reference that addresses any number of entities.
class ScriptReferenceGroup : public QObject {
    Q_OBJECT

    QPointer<SolTrace::GUI::Data::Database> m_database;
    QString                                 m_scope;
    QString                                 m_name;

public:
    explicit ScriptReferenceGroup(SolTrace::GUI::Data::Database* database,
                                  QString                        scope,
                                  QString                        name,
                                  QObject* parent = nullptr);

    void set_database(SolTrace::GUI::Data::Database* database);

public slots:
    /// Return all valid entities in this group.
    QVector<SolTrace::GUI::Data::Entity> entities() const;

    /// Add an entity. Repeated additions are harmless.
    bool add(SolTrace::GUI::Data::Entity entity);

    /// Remove an entity from the group without deleting it.
    bool remove(SolTrace::GUI::Data::Entity entity);

    bool contains(SolTrace::GUI::Data::Entity entity) const;

    /// Empty the group without deleting its entities.
    bool clear();
};

/// Entry point exposed to JavaScript as the global `refs` object.
class ScriptReferenceInterface : public QObject {
    Q_OBJECT

    QPointer<SolTrace::GUI::Data::Database> m_database;
    QHash<QString, ScriptReferenceScope*>   m_scopes;

public:
    explicit ScriptReferenceInterface(SolTrace::GUI::Data::Database* database,
                                      QObject* parent = nullptr);

public slots:
    /// Retarget this API and all existing scope helpers when the script's
    /// current scene changes.
    void set_database(SolTrace::GUI::Data::Database* database);

    /// Open a shared namespace. Reopening the same namespace addresses the
    /// same database-scoped references.
    ScriptReferenceScope* scope(QString name);

    /// Discover namespaces that currently contain references.
    QStringList scopes() const;
};

} // namespace SolTrace::GUI::Scripting
