#include "modules/database_module.h"

#include "simulation_data.hpp"

#include <gtest/gtest.h>

#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>
#include <QUrl>

#include <filesystem>
#include <vector>

namespace {

namespace SD = SolTrace::Data;
namespace App = SolTrace::GUI::App;
namespace Data = SolTrace::GUI::Data;

using namespace SolTrace::GUI::Support;

std::filesystem::path simple_test_case_path() {
    return std::filesystem::path(SOLTRACE_REPO_ROOT) /
           "gui/assets/examples/simple_test_case.stinput";
}

} // namespace

TEST(DatabaseModuleSave, SaveCurrentWritesLoadableSceneFileAndNotifies) {
    App::DatabaseModule module;
    std::vector<ANotification> notifications;

    QObject::connect(&module,
                     &App::DatabaseModule::notify,
                     [&notifications](ANotification notification)
                     { notifications.push_back(notification); });

    module.load_url(QUrl(), QStringLiteral("save-test"));

    ASSERT_EQ(module.rowCount(), 1);
    ASSERT_NE(module.current_database(), nullptr);

    SD::SimulationData source;
    ASSERT_TRUE(source.import_from_file(simple_test_case_path().string()));
    module.current_database()->import(source);
    EXPECT_TRUE(module.current_database()->modified());

    QTemporaryDir temp_dir;
    ASSERT_TRUE(temp_dir.isValid());

    auto const save_path = temp_dir.filePath(QStringLiteral("saved_scene.json"));
    module.save_current(QUrl::fromLocalFile(save_path));
    EXPECT_FALSE(module.current_database()->modified());

    QFileInfo const saved_file(save_path);
    ASSERT_TRUE(saved_file.isFile());
    EXPECT_GT(saved_file.size(), 0);

    SD::SimulationData saved;
    ASSERT_NO_THROW(saved.import_json_file(save_path.toStdString()));
    EXPECT_EQ(saved.get_number_of_ray_sources(),
              source.get_number_of_ray_sources());
    EXPECT_EQ(saved.get_number_of_elements(), source.get_number_of_elements());

    ASSERT_FALSE(notifications.empty());
    auto const& last_notification = notifications.back();
    EXPECT_EQ(last_notification.type, ANotification::INFO);
    EXPECT_EQ(last_notification.message,
              QStringLiteral("File successfully saved."));
}

TEST(DatabaseModified, TracksPersistentContentButNotSelection) {
    Data::Database database(QStringLiteral("modified-test"));
    EXPECT_FALSE(database.modified());

    auto const entity = database.create();
    database.identity.set(
        entity, Data::IdentityComponent { .name = QStringLiteral("Element") });
    EXPECT_TRUE(database.modified());

    database.set_modified(false);
    database.select(Data::Entity(entity));
    EXPECT_FALSE(database.modified());

    database.set_name(QStringLiteral("renamed"));
    EXPECT_TRUE(database.modified());
}

TEST(DatabaseModuleDelete, SelectsAnotherDatabaseWhenDeletingCurrent) {
    App::DatabaseModule module;
    module.append_new(QStringLiteral("first"));
    module.append_new(QStringLiteral("second"));

    ASSERT_TRUE(module.set_current(0));
    auto* const second = module.get_at(1)->database.data();

    module.delete_db_at_index(0);

    ASSERT_EQ(module.rowCount(), 1);
    EXPECT_EQ(module.current_database(), second);
    EXPECT_EQ(module.get_at(0)->database.data(), second);
}

TEST(DatabaseModuleDelete, ReplacesLastDatabaseWithBlank) {
    App::DatabaseModule module;
    module.append_new(QStringLiteral("only"));
    auto* const deleted = module.current_database();

    module.delete_db_at_index(0);

    ASSERT_EQ(module.rowCount(), 1);
    ASSERT_NE(module.current_database(), nullptr);
    EXPECT_NE(module.current_database(), deleted);
    EXPECT_EQ(module.current_database()->name(), QStringLiteral("Untitled"));
    EXPECT_EQ(module.get_at(0)->database.data(), module.current_database());
}

TEST(DatabaseModuleFiles, ValidateFileAcceptsReadableLocalSceneFilesOnly) {
    App::DatabaseModule module;
    QTemporaryDir       temp_dir;
    ASSERT_TRUE(temp_dir.isValid());

    auto const supported_path =
        temp_dir.filePath(QStringLiteral("scene.STINPUT"));
    QFile supported_file(supported_path);
    ASSERT_TRUE(supported_file.open(QIODevice::WriteOnly));
    supported_file.close();

    auto const unsupported_path =
        temp_dir.filePath(QStringLiteral("scene.txt"));
    QFile unsupported_file(unsupported_path);
    ASSERT_TRUE(unsupported_file.open(QIODevice::WriteOnly));
    unsupported_file.close();

    EXPECT_TRUE(module.validate_file(QUrl::fromLocalFile(supported_path)));
    EXPECT_FALSE(module.validate_file(QUrl::fromLocalFile(unsupported_path)));
    EXPECT_FALSE(module.validate_file(QUrl::fromLocalFile(temp_dir.path())));
    EXPECT_FALSE(module.validate_file(
        QUrl(QStringLiteral("https://example.com/scene.stinput"))));
    EXPECT_FALSE(module.validate_file(QUrl::fromLocalFile(
        temp_dir.filePath(QStringLiteral("missing.json")))));
}

TEST(DatabaseModuleFiles, RecentFilesAreSharedOrderedLimitedAndPersisted) {
    App::DatabaseModule module;
    QTemporaryDir       temp_dir;
    ASSERT_TRUE(temp_dir.isValid());
    module.clear_recent_files();

    QStringList paths;
    for (int i = 0; i < 6; ++i) {
        auto const path =
            temp_dir.filePath(QStringLiteral("scene-%1.json").arg(i));
        QFile file(path);
        ASSERT_TRUE(file.open(QIODevice::WriteOnly));
        file.close();
        paths.push_back(path);
        module.add_recent_file(QUrl::fromLocalFile(path));
    }

    ASSERT_EQ(module.recent_files()->rowCount(), 5);
    EXPECT_EQ(module.recent_files()->roleNames().value(Qt::UserRole), "url");
    ASSERT_NE(module.recent_files()->get_at(0), nullptr);
    EXPECT_EQ(module.recent_files()->get_at(0)->url,
              QUrl::fromLocalFile(paths.back()).toString());

    module.add_recent_file(QUrl::fromLocalFile(paths[2]));
    ASSERT_EQ(module.recent_files()->rowCount(), 5);
    ASSERT_NE(module.recent_files()->get_at(0), nullptr);
    EXPECT_EQ(module.recent_files()->get_at(0)->url,
              QUrl::fromLocalFile(paths[2]).toString());

    App::DatabaseModule restored;
    EXPECT_EQ(restored.recent_files()->urls(), module.recent_files()->urls());
    restored.clear_recent_files();
}
