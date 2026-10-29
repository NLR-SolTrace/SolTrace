#include "data/components.h"
#include "data/database.h"
#include "scripting/script_db_interface.h"

#include <gtest/gtest.h>

#include <memory>

namespace SolTrace::GUI::Data {
namespace {

TEST(DatabaseTags, SanitizesNamesAndTracksHiddenState) {
    Database database(QStringLiteral("Tags"));

    auto const first  = Entity(database.create_tag(QStringLiteral(" Build ")));
    auto const second = Entity(database.create_tag(QStringLiteral("Build"), true));

    EXPECT_EQ(database.name_of(first), QStringLiteral("Build"));
    EXPECT_EQ(database.name_of(second), QStringLiteral("Build_1"));
    EXPECT_FALSE(database.is_hidden_tag(first));
    EXPECT_TRUE(database.is_hidden_tag(second));

    database.set_tag_hidden(first, true);
    database.set_tag_hidden(second, false);
    EXPECT_TRUE(database.is_hidden_tag(first));
    EXPECT_FALSE(database.is_hidden_tag(second));
}

TEST(DatabaseTags, MaintainsUniqueBidirectionalMembership) {
    Database database(QStringLiteral("Tags"));
    auto const item = Entity(database.create());
    auto const tag  = Entity(database.create_tag(QStringLiteral("Builder")));

    database.assign_tag(item, tag);
    database.assign_tag(item, tag);

    ASSERT_EQ(database.tags_for(item).size(), 1u);
    EXPECT_EQ(database.tags_for(item).front(), tag);
    ASSERT_EQ(database.tagged_with(tag).size(), 1);
    EXPECT_EQ(database.tagged_with(tag).front(), item);

    database.unassign_tag(item, tag);
    EXPECT_TRUE(database.tags_for(item).empty());
    EXPECT_TRUE(database.tagged_with(tag).empty());
    EXPECT_FALSE(
        database.as_registry().all_of<TagMembershipComponent>(item));
}

TEST(DatabaseTags, RejectsInvalidAndSelfMembership) {
    Database database(QStringLiteral("Tags"));
    auto const item = Entity(database.create());
    auto const tag  = Entity(database.create_tag(QStringLiteral("Builder")));

    database.assign_tag(Entity {}, tag);
    database.assign_tag(item, Entity {});
    database.assign_tag(tag, tag);

    EXPECT_TRUE(database.tags_for(item).empty());
    EXPECT_TRUE(database.tags_for(tag).empty());
    EXPECT_TRUE(database.tagged_with(tag).empty());
}

TEST(DatabaseTags, CleansMembershipWhenEitherSideIsDestroyed) {
    Database database(QStringLiteral("Tags"));
    auto const first  = Entity(database.create());
    auto const second = Entity(database.create());
    auto const tag    = Entity(database.create_tag(QStringLiteral("Builder")));

    database.assign_tag(first, tag);
    database.assign_tag(second, tag);
    database.as_registry().destroy(first);

    ASSERT_EQ(database.tagged_with(tag).size(), 1);
    EXPECT_EQ(database.tagged_with(tag).front(), second);

    database.delete_tag(tag);
    EXPECT_FALSE(database.valid(tag));
    EXPECT_TRUE(database.tags_for(second).empty());
    EXPECT_FALSE(
        database.as_registry().all_of<TagMembershipComponent>(second));
}

TEST(DatabaseTags, HiddenTagsAndMembershipSurviveClone) {
    Database database(QStringLiteral("Tags"));
    auto const item = database.add_element(QStringLiteral("Generated"));
    auto const tag  = Entity(database.create_tag(
        QStringLiteral("__soltrace_scene_builder/example"), true));
    database.assign_tag(item, tag);

    std::unique_ptr<Database> clone(
        database.clone(QStringLiteral("Tags Clone")));

    Entity cloned_tag;
    for (auto const& [entity, identity] :
         clone->as_registry().view<TagComponent, IdentityComponent>().each()) {
        if (identity.name == QStringLiteral("__soltrace_scene_builder/example")) {
            cloned_tag = entity;
            break;
        }
    }

    ASSERT_TRUE(cloned_tag.is_valid());
    EXPECT_TRUE(clone->is_hidden_tag(cloned_tag));
    auto const members = clone->tagged_with(cloned_tag);
    ASSERT_EQ(members.size(), 1);
    EXPECT_EQ(clone->name_of(members.front()), QStringLiteral("Generated"));
}

TEST(ScriptDatabaseTags, ExposesVisibleTagsAndProtectsHiddenTags) {
    Database database(QStringLiteral("Tags"));
    SolTrace::GUI::Scripting::ScriptDBInterface script(&database);

    auto const visible = script.create_tag(QStringLiteral("Generated"));
    auto const hidden  = Entity(database.create_tag(
        QStringLiteral("__soltrace_scene_builder/example"), true));

    EXPECT_EQ(script.ensure_tag(QStringLiteral("Generated")), visible);
    EXPECT_EQ(script.find_tag(QStringLiteral("Generated")), visible);
    EXPECT_FALSE(script.find_tag(database.name_of(hidden)).is_valid());
    EXPECT_FALSE(script.ensure_tag(database.name_of(hidden)).is_valid());

    auto const exposed_tags = script.get_all_tags();
    ASSERT_EQ(exposed_tags.size(), 1);
    EXPECT_EQ(exposed_tags.front(), visible);

    auto const element = script.create();
    EXPECT_TRUE(script.assign_tag(element, visible));
    EXPECT_FALSE(script.assign_tag(element, hidden));

    ASSERT_EQ(script.get_tags(element).size(), 1);
    EXPECT_EQ(script.get_tags(element).front(), visible);
    ASSERT_EQ(script.get_tagged(visible).size(), 1);
    EXPECT_EQ(script.get_tagged(visible).front(), element);
    EXPECT_TRUE(script.get_tagged(hidden).empty());

    script.destroy(hidden);
    EXPECT_TRUE(database.valid(hidden));
    EXPECT_FALSE(script.delete_tag(hidden));

    script.set_identity(visible, QStringLiteral("Renamed"));
    EXPECT_EQ(database.name_of(visible), QStringLiteral("Generated"));

    EXPECT_TRUE(script.delete_tag(visible));
    EXPECT_TRUE(database.valid(element));
    EXPECT_TRUE(script.get_tags(element).empty());
}

} // namespace
} // namespace SolTrace::GUI::Data
