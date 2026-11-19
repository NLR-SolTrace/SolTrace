#include "data/components.h"
#include "data/database.h"
#include "scripting/script_executor.h"

#include <QJsonArray>

#include <gtest/gtest.h>

#include <utility>

namespace SolTrace::GUI::Scripting {
namespace {

ScriptExecutionRequest request_for(SolTrace::GUI::Data::Database& database,
                                   QString                         source) {
    return ScriptExecutionRequest {
        .source      = std::move(source),
        .source_name = QStringLiteral("Executor Test"),
        .database    = &database,
    };
}

TEST(ScriptExecutor, UsesFreshEngineForEveryInvocation) {
    SolTrace::GUI::Data::Database database(QStringLiteral("Script"));

    auto first = ScriptExecutor::execute(request_for(
        database,
        QStringLiteral("(function() { globalThis.persisted = 42; })")));
    ASSERT_TRUE(first.success) << first.error.toStdString();

    auto second = ScriptExecutor::execute(request_for(
        database,
        QStringLiteral(
            "(function() {"
            " if (typeof persisted !== 'undefined')"
            "   throw new Error('state leaked');"
            "})")));
    EXPECT_TRUE(second.success) << second.error.toStdString();
}

TEST(ScriptExecutor, ConvertsArgumentsAndForwardsConsoleOutput) {
    SolTrace::GUI::Data::Database database(QStringLiteral("Script"));
    auto request = request_for(
        database,
        QStringLiteral(
            "(function(label, position) {"
            " const entity = db.create();"
            " db.set_identity(entity, label);"
            " db.set_transform(entity, { position: position });"
            " console.log('created', label);"
            "})"));
    request.arguments = {
        QStringLiteral("Mirror"),
        QJsonArray { 1.0, 2.0, 3.0 },
    };

    QVector<QString> messages;
    auto result = ScriptExecutor::execute(
        request,
        [&messages](ScriptLogLevel, QString const& message) {
            messages.push_back(message);
        });

    ASSERT_TRUE(result.success) << result.error.toStdString();
    ASSERT_EQ(messages.size(), 1);
    EXPECT_EQ(messages.front(), QStringLiteral("created Mirror"));

    auto const elements =
        database.as_registry().view<SolTrace::GUI::Data::ElementComponent>();
    ASSERT_EQ(elements.size(), 1u);
    auto const entity = *elements.begin();
    EXPECT_EQ(database.name_of(entity), QStringLiteral("Mirror"));
    auto const* transform = database.transform.get(entity);
    ASSERT_NE(transform, nullptr);
    EXPECT_EQ(transform->position, glm::dvec3(1.0, 2.0, 3.0));
}

TEST(ScriptExecutor, OmitsOptionalAppCapabilitiesByDefault) {
    SolTrace::GUI::Data::Database database(QStringLiteral("Script"));
    auto result = ScriptExecutor::execute(request_for(
        database,
        QStringLiteral(
            "(function() {"
            " if (typeof sim !== 'undefined') throw new Error('sim exposed');"
            " if (typeof scenes !== 'undefined')"
            "   throw new Error('scenes exposed');"
            " if (typeof refs === 'undefined')"
            "   throw new Error('refs missing');"
            "})")));

    EXPECT_TRUE(result.success) << result.error.toStdString();
}

TEST(ScriptExecutor, ReturnsStructuredEvaluationAndRuntimeErrors) {
    SolTrace::GUI::Data::Database database(QStringLiteral("Script"));

    auto evaluation = ScriptExecutor::execute(
        request_for(database, QStringLiteral("(function(")));
    EXPECT_FALSE(evaluation.success);
    EXPECT_TRUE(evaluation.error.contains(QStringLiteral("evaluation")));

    auto runtime = ScriptExecutor::execute(request_for(
        database,
        QStringLiteral("(function() { throw new Error('failure'); })")));
    EXPECT_FALSE(runtime.success);
    EXPECT_TRUE(runtime.error.contains(QStringLiteral("runtime")));
    EXPECT_TRUE(runtime.error.contains(QStringLiteral("failure")));
    EXPECT_GE(runtime.line, 1);
}

} // namespace
} // namespace SolTrace::GUI::Scripting
