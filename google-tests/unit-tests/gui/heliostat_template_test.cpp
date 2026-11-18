#include "data/components.h"
#include "data/database.h"
#include "scripting/script_executor.h"
#include "scripting/script_references.h"

#include <QFile>
#include <QJsonArray>
#include <QStringList>

#include <gtest/gtest.h>

#include <utility>

namespace SolTrace::GUI::Scripting {
namespace {

TEST(DatabaseGroups, ParametersExistWhenGroupsAreAnnounced) {
    SolTrace::GUI::Data::Database database(QStringLiteral("Groups"));
    bool material_was_complete = false;
    bool geometry_was_complete = false;

    QObject::connect(
        database.material_root.self(),
        &SolTrace::GUI::Data::ComponentAPIBase::changed,
        [&database, &material_was_complete](entt::entity group) {
            material_was_complete =
                database.material_parameters.get(group) != nullptr;
        });
    QObject::connect(
        database.geometry_root.self(),
        &SolTrace::GUI::Data::ComponentAPIBase::changed,
        [&database, &geometry_was_complete](entt::entity group) {
            geometry_was_complete =
                database.geometry_parameters.get(group) != nullptr;
        });

    database.add_material_group(QStringLiteral("Material"), { });
    database.add_geometry_group(QStringLiteral("Geometry"), { });

    EXPECT_TRUE(material_was_complete);
    EXPECT_TRUE(geometry_was_complete);
}

QString heliostat_source() {
    QFile file(QString::fromUtf8(SOLTRACE_REPO_ROOT) +
               QStringLiteral(
                   "/gui/assets/scripts/templates/heliostat.js"));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return { };

    auto source = QString::fromUtf8(file.readAll());
    auto header_end = source.indexOf(QStringLiteral("*/"));
    if (header_end < 0) return { };

    auto const arguments = QStringList {
        QStringLiteral("scope_id"),
        QStringLiteral("heliostat_name"),
        QStringLiteral("position"),
        QStringLiteral("target_position"),
        QStringLiteral("aperture_width"),
        QStringLiteral("aperture_height"),
        QStringLiteral("panels_x"),
        QStringLiteral("panels_y"),
        QStringLiteral("gap_x"),
        QStringLiteral("gap_y"),
        QStringLiteral("focal_length_x"),
        QStringLiteral("focal_length_y"),
        QStringLiteral("canting_method"),
        QStringLiteral("canting_distance"),
        QStringLiteral("canting_sun_azimuth"),
        QStringLiteral("canting_sun_zenith"),
        QStringLiteral("reflectivity"),
        QStringLiteral("slope_error"),
        QStringLiteral("specularity_error"),
    };

    return QStringLiteral("(function(%1) {\n%2\n})")
        .arg(arguments.join(QStringLiteral(", ")),
             source.mid(header_end + 2));
}

QVector<QJsonValue> heliostat_arguments(int     panels_x,
                                        int     panels_y,
                                        QString canting) {
    return {
        QStringLiteral("test.heliostat"),
        QStringLiteral("Test Heliostat"),
        QJsonArray { 0.0, 0.0, 0.0 },
        QJsonArray { 0.0, 0.0, 100.0 },
        12.0,
        10.0,
        panels_x,
        panels_y,
        0.05,
        0.05,
        0.0,
        0.0,
        std::move(canting),
        100.0,
        180.0,
        10.0,
        0.95,
        0.0,
        0.0,
    };
}

ScriptExecutionResult run_heliostat(SolTrace::GUI::Data::Database& database,
                                    int panels_x,
                                    int panels_y,
                                    QString canting) {
    return ScriptExecutor::execute(ScriptExecutionRequest {
        .source            = heliostat_source(),
        .source_name       = QStringLiteral("Heliostat Template"),
        .arguments         = heliostat_arguments(
            panels_x, panels_y, std::move(canting)),
        .database          = &database,
    });
}

TEST(HeliostatTemplate, ReusesAndResizesReferencedSceneContent) {
    SolTrace::GUI::Data::Database database(QStringLiteral("Template"));

    auto first = run_heliostat(database, 3, 2, QStringLiteral("NONE"));
    ASSERT_TRUE(first.success) << first.error.toStdString();
    EXPECT_EQ(database.as_registry()
                  .view<SolTrace::GUI::Data::ElementComponent>()
                  .size(),
              7u);
    EXPECT_EQ(database.as_registry()
                  .view<SolTrace::GUI::Data::MaterialGroupComponent>()
                  .size(),
              1u);
    EXPECT_EQ(database.as_registry()
                  .view<SolTrace::GUI::Data::GeometryGroupComponent>()
                  .size(),
              1u);

    auto second =
        run_heliostat(database, 3, 2, QStringLiteral("ON_AXIS"));
    ASSERT_TRUE(second.success) << second.error.toStdString();
    EXPECT_EQ(database.as_registry()
                  .view<SolTrace::GUI::Data::ElementComponent>()
                  .size(),
              7u);

    auto third =
        run_heliostat(database, 2, 1, QStringLiteral("OFF_AXIS"));
    ASSERT_TRUE(third.success) << third.error.toStdString();
    EXPECT_EQ(database.as_registry()
                  .view<SolTrace::GUI::Data::ElementComponent>()
                  .size(),
              3u);

    ScriptReferenceInterface references(&database);
    auto* scope = references.scope(QStringLiteral("test.heliostat"));
    auto  root = scope->single(QStringLiteral("root"))->get();
    auto  material = scope->single(QStringLiteral("material"))->get();
    auto  geometry = scope->single(QStringLiteral("geometry"))->get();
    auto  facets = scope->group(QStringLiteral("facets"))->entities();
    ASSERT_TRUE(database.valid(root));
    ASSERT_EQ(facets.size(), 2);
    EXPECT_EQ(database.children_of(root).size(), 2u);
    for (auto facet : facets) {
        EXPECT_EQ(database.material_of(facet), material);
        EXPECT_EQ(database.geometry_of(facet), geometry);
    }
}

} // namespace
} // namespace SolTrace::GUI::Scripting
