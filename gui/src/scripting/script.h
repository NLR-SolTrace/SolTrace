#pragma once

#include "scripting/script_db_interface.h"
#include "scripting/script_executor.h"
#include "support/notification.h"
#include "support/qt_helpers.h"
#include "support/structmodel.h"

#include <QObject>
#include <QStringList>
#include <QVector3D>

namespace SolTrace::GUI::App {
class DatabaseModule;
class SimulationModule;
} // namespace SolTrace::GUI::App

namespace SolTrace::GUI::Scripting {

/// One user-editable argument declared in a script header.
///
/// Script headers are parsed into these records and exposed to QML through
/// ScriptPropertyModel. The `value` field is editable; the remaining fields
/// describe how the UI should present and validate that value.
struct ScriptProperty {
    QString     name;
    QString     identifier;
    QString     type;
    QString     description;
    QStringList choices;
    QString     extra;
    bool        min_bounded   = false;
    bool        max_bounded   = false;
    bool        max_inclusive = false;
    double      min           = 0.0;
    double      max           = 0.0;
    double      step          = 1.0;
    int         decimals      = 4;
    QString     unit;
    QString     error;

    QString value;

    RECORD_META(ScriptProperty,
                SM_EXPOSE_RO(name),
                SM_EXPOSE_RO(identifier),
                SM_EXPOSE_RO(type),
                SM_EXPOSE_RO(description),
                SM_EXPOSE_RO(choices),
                SM_EXPOSE_RO(extra),
                SM_EXPOSE_RO(min_bounded),
                SM_EXPOSE_RO(max_bounded),
                SM_EXPOSE_RO(max_inclusive),
                SM_EXPOSE_RO(min),
                SM_EXPOSE_RO(max),
                SM_EXPOSE_RO(step),
                SM_EXPOSE_RO(decimals),
                SM_EXPOSE_RO(unit),
                SM_EXPOSE_RO(error),
                SM_EXPOSE_RW(value));
};

class ScriptPropertyModel : public Support::StructTableModel<ScriptProperty> {
    Q_OBJECT

public:
    explicit ScriptPropertyModel(QObject* parent = nullptr);
};

/// Session state for editing one script position parameter in the viewport.
class ScriptPositionEditor : public QObject {
    Q_OBJECT

    QPointer<ScriptPropertyModel> m_properties;
    int                           m_row = -1;
    QVector3D                     m_position;
    bool                          m_active = false;
    QString                       m_name;
    double                        m_step     = 0.1;
    int                           m_decimals = 4;

    static constexpr int value_role =
        ROLE_FOR_MEMBER(ScriptProperty, value);

public:
    explicit ScriptPositionEditor(ScriptPropertyModel* properties,
                                  QObject* parent = nullptr);

    Q_PROPERTY(QVector3D position READ position WRITE set_position NOTIFY
                   position_changed FINAL)
    Q_PROPERTY(bool active READ active NOTIFY active_changed FINAL)
    Q_PROPERTY(QString name READ name NOTIFY name_changed FINAL)
    Q_PROPERTY(double step READ step NOTIFY metadata_changed FINAL)
    Q_PROPERTY(int decimals READ decimals NOTIFY metadata_changed FINAL)

    QVector3D position() const { return m_position; }
    bool      active() const { return m_active; }
    QString   name() const { return m_name; }
    double    step() const { return m_step; }
    int       decimals() const { return m_decimals; }

    void set_position(QVector3D position);

public slots:
    /// Begin editing the position parameter with this manifest identifier.
    bool begin(QString identifier);

    /// End the current viewport position-editing session.
    void end();

signals:
    void position_changed();
    void active_changed();
    void name_changed();
    void metadata_changed();
};

/// Script-facing simulation controls exposed as `sim`.
class ScriptSimulationInterface : public QObject {
    Q_OBJECT

    QPointer<App::SimulationModule> m_simulation;

public:
    explicit ScriptSimulationInterface(App::SimulationModule*,
                                       QObject* parent = nullptr);

public slots:
    /// Start a simulation using the selected runner and optional config patch.
    bool start(QJsonObject config = { });
};

/// Script-facing open-scene controls exposed as `scenes`.
class ScriptSceneInterface : public QObject {
    Q_OBJECT

    QPointer<App::DatabaseModule> m_databases;
    QPointer<ScriptDBInterface>   m_database_interface;
    QString                       m_working_directory;

public:
    explicit ScriptSceneInterface(App::DatabaseModule*,
                                  ScriptDBInterface*,
                                  QString  working_directory,
                                  QObject* parent = nullptr);

public slots:
    /// Select an open scene by list index.
    bool set_current_index(int index);

    /// Select the first open scene with the given display name.
    bool set_current_name(QString name);

    /// Create a new blank scene and select it.
    bool new_blank(QString name = "Untitled");

    /// Load a scene file relative to the script working directory.
    bool new_from_file(QString relative_path, QString name_override = "");

    /// Export the current scene to JSON relative to the script working
    /// directory.
    bool export_json(QString relative_path);
};

/// User-authored script plus parsed metadata and execution state.
///
/// Script source must start with a comment header block. No whitespace or code
/// may appear before it. The preferred header format is an embedded JSON
/// manifest:
///
///     /*{
///       "schema_version": 1,
///       "title": "Example Script",
///       "description": "Creates a few entities.",
///       "parameters": [
///         {
///           "id": "mirror_count",
///           "name": "Mirror count",
///           "description": "Number of mirrors to create.",
///           "type": "integer",
///           "default": 12,
///           "minimum": 1,
///           "maximum": 100
///         },
///         {
///           "id": "mode",
///           "type": "string",
///           "default": "Flat",
///           "choices": ["Flat", "Curved"]
///         },
///         {
///           "id": "direction",
///           "type": "vec3",
///           "default": [0, 0, 1],
///           "unit": "direction"
///         }
///       ]
///     }
///     *\/
///     const e = db.create()
///     return e
///
/// Every JSON parameter requires `id`, `type`, and `default`. Optional fields
/// include `name`, `description`, `minimum`, `maximum`, `step`, `decimals`,
/// `choices`, and `unit`. Supported types are `integer`, `real`, `string`, and
/// `vec3`/`vector3d`, and `position`. Position parameters have the same
/// JavaScript value as a vec3, but can also be edited in the 3D viewport;
/// choices are currently supported for strings.
///
/// The older TITLE/DESC/PROPERTY format remains supported. Its directives are:
/// - TITLE text
///   Required. Must be the first non-empty directive in the header. The
///   remaining text becomes Script::title.
/// - DESC text
///   Optional and repeatable. Each line is appended to Script::description.
/// - PROPERTY name type initial [extra...]
///   Optional and repeatable. Properties are parsed in header order, exposed in
///   Script::properties, and passed as positional arguments to the evaluated
///   JavaScript function during run(). The initial value is stored as the
///   editable ScriptProperty::value shown in the UI.
///
/// Property types:
/// - integer initial range
///   Integer argument. The range follows the initial value and uses Rust-like
///   syntax: `..0`, `1..`, `1..10`, or `1..=100`. A range is currently
///   required.
/// - real initial range
///   Floating-point argument with the same range syntax as integer.
/// - string
///   Unrestricted string argument. The initial value is one token; no extra
///   arguments are currently used.
/// - vec3 initial [unit]
///   Three-component vector argument. The optional `unit` extra follows the
///   initial value and marks that the vector should be treated as a direction.
///   The current runner accepts values like `1 0 0`, `[1, 0, 0]`, or a single
///   scalar such as `1`, which expands to `[1, 1, 1]`. Header initial values
///   should be written without spaces, for example `[0,0,1]`.
///
/// After the header, the script body is wrapped in a generated JavaScript
/// function. Script::run() converts property values according to the
/// declarations above and calls the generated function with those arguments.
/// PROPERTY identifiers are available as normal JavaScript parameters, and the
/// database script API is available as `db`.
class Script : public QObject {
    Q_OBJECT

    QPointer<SolTrace::GUI::Data::Database> m_database;
    QPointer<App::DatabaseModule>           m_databases;
    QPointer<App::SimulationModule>         m_simulation;

    Q_WRITABLE_PROPERTY(QString, code, { });
    Q_READONLY_PROPERTY(QString, title);
    Q_READONLY_PROPERTY(QString, description);
    Q_READONLY_PROPERTY(bool, valid);
    Q_READONLY_PROPERTY(QStringList, parse_errors);
    QOBJECT_READONLY_PROPERTY(ScriptPropertyModel, properties);
    QOBJECT_READONLY_PROPERTY(ScriptPositionEditor, position_editor);


    // Directory where scripts can pull additional content from
    Q_WRITABLE_PROPERTY(QString, working_directory, { });

    Q_READONLY_PROPERTY(QStringList, builtin_scripts);

public:
    explicit Script(QObject* parent = nullptr);

    /// Attach the script to the database it will inspect and mutate.
    void set_database(SolTrace::GUI::Data::Database*);

    /// Attach app-level services exposed to scripts.
    void set_services(App::DatabaseModule*, App::SimulationModule*);

public slots:
    /// Parse header metadata and validate property declarations.
    bool parse();

    /// Parse, compile, and run the script against the attached database.
    void run();

    /// Emit a script error notification.
    void notify_error(QString message);

    /// Generate markdown documentation for the script database API.
    QString api_markdown();

    /// Save code to a path. This is not the same as our code property
    /// As it could be modified and not committed. True on success.
    bool save_to(QUrl path, QString code);

signals:
    void notify(Support::ANotification);

    void logged(int, QString);

    void runCompleted();
};

} // namespace SolTrace::GUI::Scripting
