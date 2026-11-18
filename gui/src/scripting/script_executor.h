#pragma once

#include <QJSValue>
#include <QJsonValue>
#include <QObject>
#include <QString>
#include <QVector>

#include <functional>

namespace SolTrace::GUI::Data {
class Database;
} // namespace SolTrace::GUI::Data

namespace SolTrace::GUI::Scripting {

class ScriptDBInterface;

/// Severity for messages emitted by user scripts.
enum class ScriptLogLevel { Log, Warn, Error };

using ScriptLogHandler =
    std::function<void(ScriptLogLevel level, QString const& message)>;

/// Console object exposed to scripts for log/warn/error output.
class ScriptConsole : public QObject {
    Q_OBJECT

public:
    explicit ScriptConsole(QObject* parent = nullptr);

public slots:
    // Horrible, but that's how this is going to have to be.

    void log(QJSValue a = { },
             QJSValue b = { },
             QJSValue c = { },
             QJSValue d = { },
             QJSValue e = { },
             QJSValue f = { },
             QJSValue g = { },
             QJSValue h = { });
    void warn(QJSValue a = { },
              QJSValue b = { },
              QJSValue c = { },
              QJSValue d = { },
              QJSValue e = { },
              QJSValue f = { },
              QJSValue g = { },
              QJSValue h = { });
    void error(QJSValue a = { },
               QJSValue b = { },
               QJSValue c = { },
               QJSValue d = { },
               QJSValue e = { },
               QJSValue f = { },
               QJSValue g = { },
               QJSValue h = { });

signals:
    void logged(int, QString);
};

using ScriptObjectMaker = std::function<QObject*(QObject* parent)>;
using ScriptSceneObjectMaker =
    std::function<QObject*(ScriptDBInterface*, QObject* parent)>;

/// All inputs needed for one JavaScript invocation.
struct ScriptExecutionRequest {
    QString             source;
    QString             source_name;
    QString             working_directory;
    QVector<QJsonValue> arguments;

    SolTrace::GUI::Data::Database* database = nullptr;

    // Possibly over-engineered here, but fun
    ScriptSceneObjectMaker scene_api_factory;
    ScriptObjectMaker      simulation_api_factory;
};

/// Structured outcome of one stateless JavaScript invocation.
struct ScriptExecutionResult {
    bool    success = false;
    QString error;
    int     line = -1;
};

/// Creates a fresh JavaScript engine for every invocation and destroys all
/// engine-owned state before returning.
class ScriptExecutor {
public:
    static ScriptExecutionResult
    execute(ScriptExecutionRequest const& request,
            ScriptLogHandler const&       log_handler = { });
};

} // namespace SolTrace::GUI::Scripting
