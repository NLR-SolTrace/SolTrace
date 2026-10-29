#include "script_executor.h"

#include "scripting/script_db_interface.h"
#include "scripting/script_references.h"

#include <QDebug>
#include <QJSEngine>
#include <QJSValue>
#include <QStringList>
#include <QVariant>

#include <memory>

namespace SolTrace::GUI::Scripting {

namespace {

QString script_value_to_string(QJSValue const& value) {
    if (value.isUndefined()) return { };
    if (value.isNull()) return QStringLiteral("null");
    return value.toString();
}

QString format_script_log(QJSValue const& a,
                          QJSValue const& b,
                          QJSValue const& c,
                          QJSValue const& d,
                          QJSValue const& e,
                          QJSValue const& f,
                          QJSValue const& g,
                          QJSValue const& h) {
    QStringList parts;
    for (auto const& value : { a, b, c, d, e, f, g, h }) {
        if (!value.isUndefined()) parts << script_value_to_string(value);
    }
    return parts.join(QLatin1Char(' '));
}

void report(ScriptLogHandler const& handler,
            ScriptLogLevel          level,
            QString const&          message) {
    if (handler) handler(level, message);
}

ScriptExecutionResult script_error(QString const&          phase,
                                   QJSValue const&         value,
                                   ScriptLogHandler const& handler) {
    auto const error_name = value.property("name").toString();
    auto const line       = value.property("lineNumber").toInt();
    auto const detail     = value.toString();
    auto const heading = QStringLiteral("Script %1 exception: %2")
                             .arg(phase,
                                  error_name.isEmpty()
                                      ? QStringLiteral("Error")
                                      : error_name);
    auto const location = QStringLiteral("Line number %1: %2")
                              .arg(line)
                              .arg(detail);

    qCritical().noquote() << "[Script]" << heading;
    qCritical().noquote() << "[Script]" << location;
    report(handler, ScriptLogLevel::Error, heading);
    report(handler, ScriptLogLevel::Error, location);

    return ScriptExecutionResult {
        .success = false,
        .error   = heading + QLatin1Char('\n') + location,
        .line    = line,
    };
}

} // namespace

ScriptConsole::ScriptConsole(QObject* parent) : QObject(parent) { }

void ScriptConsole::log(QJSValue a,
                        QJSValue b,
                        QJSValue c,
                        QJSValue d,
                        QJSValue e,
                        QJSValue f,
                        QJSValue g,
                        QJSValue h) {
    auto const message = format_script_log(a, b, c, d, e, f, g, h);
    qInfo().noquote() << "[Script]" << message;
    emit logged(static_cast<int>(ScriptLogLevel::Log), message);
}

void ScriptConsole::warn(QJSValue a,
                         QJSValue b,
                         QJSValue c,
                         QJSValue d,
                         QJSValue e,
                         QJSValue f,
                         QJSValue g,
                         QJSValue h) {
    auto const message = format_script_log(a, b, c, d, e, f, g, h);
    qWarning().noquote() << "[Script]" << message;
    emit logged(static_cast<int>(ScriptLogLevel::Warn), message);
}

void ScriptConsole::error(QJSValue a,
                          QJSValue b,
                          QJSValue c,
                          QJSValue d,
                          QJSValue e,
                          QJSValue f,
                          QJSValue g,
                          QJSValue h) {
    auto const message = format_script_log(a, b, c, d, e, f, g, h);
    qCritical().noquote() << "[Script]" << message;
    emit logged(static_cast<int>(ScriptLogLevel::Error), message);
}

ScriptExecutionResult
ScriptExecutor::execute(ScriptExecutionRequest const& request,
                        ScriptLogHandler const&      log_handler) {
    if (!request.database) {
        auto const message = QStringLiteral("No database available.");
        qCritical().noquote() << "[Script]" << message;
        report(log_handler, ScriptLogLevel::Error, message);
        return { .success = false, .error = message };
    }

    auto engine = std::make_unique<QJSEngine>();
    engine->installExtensions(QJSEngine::TranslationExtension |
                              QJSEngine::GarbageCollectionExtension);

    auto database_api =
        std::make_unique<ScriptDBInterface>(request.database);
    database_api->update_working_directory(request.working_directory);
    engine->globalObject().setProperty(
        QStringLiteral("db"), engine->newQObject(database_api.get()));

    auto references =
        std::make_unique<ScriptReferenceInterface>(request.database);
    QObject::connect(database_api.get(),
                     &ScriptDBInterface::database_changed,
                     references.get(),
                     &ScriptReferenceInterface::set_database);
    engine->globalObject().setProperty(
        QStringLiteral("refs"), engine->newQObject(references.get()));

    if (request.expose_simulation) {
        if (!request.simulation_api_factory) {
            auto const message =
                QStringLiteral("Simulation script API is unavailable.");
            report(log_handler, ScriptLogLevel::Error, message);
            return { .success = false, .error = message };
        }
        auto* simulation_api = request.simulation_api_factory(engine.get());
        if (!simulation_api) {
            auto const message =
                QStringLiteral("Simulation script API creation failed.");
            report(log_handler, ScriptLogLevel::Error, message);
            return { .success = false, .error = message };
        }
        if (!simulation_api->parent()) simulation_api->setParent(engine.get());
        engine->globalObject().setProperty(QStringLiteral("sim"),
                                           engine->newQObject(simulation_api));
    }

    if (request.expose_scenes) {
        if (!request.scene_api_factory) {
            auto const message = QStringLiteral("Scene script API is unavailable.");
            report(log_handler, ScriptLogLevel::Error, message);
            return { .success = false, .error = message };
        }
        auto* scene_api =
            request.scene_api_factory(database_api.get(), engine.get());
        if (!scene_api) {
            auto const message =
                QStringLiteral("Scene script API creation failed.");
            report(log_handler, ScriptLogLevel::Error, message);
            return { .success = false, .error = message };
        }
        if (!scene_api->parent()) scene_api->setParent(engine.get());
        engine->globalObject().setProperty(QStringLiteral("scenes"),
                                           engine->newQObject(scene_api));
    }

    auto console = std::make_unique<ScriptConsole>();
    QObject::connect(
        console.get(),
        &ScriptConsole::logged,
        [log_handler](int level, QString const& message) {
            report(log_handler,
                   static_cast<ScriptLogLevel>(level),
                   message);
        });
    engine->globalObject().setProperty(
        QStringLiteral("console"), engine->newQObject(console.get()));

    QStringList stack_trace;
    auto callable = engine->evaluate(request.source,
                                     request.source_name,
                                     1,
                                     &stack_trace);
    if (callable.isError() || !stack_trace.isEmpty()) {
        return script_error(QStringLiteral("evaluation"),
                            callable,
                            log_handler);
    }

    if (!callable.isCallable()) {
        auto const message =
            QStringLiteral("Script did not produce a callable function.");
        qCritical().noquote() << "[Script]" << message;
        report(log_handler, ScriptLogLevel::Error, message);
        return { .success = false, .error = message };
    }

    QJSValueList arguments;
    arguments.reserve(request.arguments.size());
    for (auto const& argument : request.arguments) {
        arguments.push_back(
            engine->toScriptValue(argument.toVariant()));
    }

    auto result = callable.call(arguments);
    if (result.isError()) {
        return script_error(QStringLiteral("runtime"), result, log_handler);
    }

    return { .success = true };
}

} // namespace SolTrace::GUI::Scripting
