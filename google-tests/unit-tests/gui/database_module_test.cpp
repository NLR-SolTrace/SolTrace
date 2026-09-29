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

    QTemporaryDir temp_dir;
    ASSERT_TRUE(temp_dir.isValid());

    auto const save_path = temp_dir.filePath(QStringLiteral("saved_scene.json"));
    module.save_current(QUrl::fromLocalFile(save_path));

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
