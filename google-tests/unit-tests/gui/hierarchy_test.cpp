#include "data/database.h"
#include "scripting/script_db_interface.h"

#include <gtest/gtest.h>

namespace SolTrace::GUI::Data {
namespace {

TEST(DatabaseHierarchy, RejectsInvalidRelationshipsWithoutChangingHierarchy) {
    Database database(QStringLiteral("Hierarchy"));
    auto const parent = database.add_element(QStringLiteral("Parent"));
    auto const child  = database.add_element(QStringLiteral("Child"));
    auto const leaf   = database.add_element(QStringLiteral("Leaf"));
    auto const other  = Entity(database.create());

    ASSERT_TRUE(database.set_parent(child, parent));
    ASSERT_TRUE(database.set_parent(leaf, child));

    EXPECT_FALSE(database.can_set_parent(parent, parent));
    EXPECT_FALSE(database.set_parent(parent, parent));
    EXPECT_FALSE(database.can_set_parent(parent, leaf));
    EXPECT_FALSE(database.set_parent(parent, leaf));
    EXPECT_FALSE(database.can_set_parent(child, other));
    EXPECT_FALSE(database.set_parent(child, other));

    EXPECT_EQ(Entity(database.parent_of(child)), parent);
    EXPECT_EQ(Entity(database.parent_of(leaf)), child);
    EXPECT_FALSE(Entity(database.parent_of(parent)).is_valid());
}

TEST(DatabaseHierarchy, RepeatedAssignmentIsIdempotent) {
    Database database(QStringLiteral("Hierarchy"));
    auto const parent = database.add_element(QStringLiteral("Parent"));
    auto const child  = database.add_element(QStringLiteral("Child"));

    ASSERT_TRUE(database.set_parent(child, parent));
    ASSERT_TRUE(database.set_parent(child, parent));

    auto const children = database.children_of(parent);
    ASSERT_EQ(children.size(), 1u);
    EXPECT_EQ(children.front(), child);
}

TEST(ScriptDatabaseHierarchy, ExposesSafeParentAndChildOperations) {
    Database database(QStringLiteral("Hierarchy"));
    SolTrace::GUI::Scripting::ScriptDBInterface script(&database);

    auto const parent = script.create();
    auto const child  = script.create();

    EXPECT_TRUE(script.can_set_parent(child, parent));
    EXPECT_TRUE(script.set_parent(child, parent));
    EXPECT_EQ(script.get_parent(child), parent);

    auto const children = script.get_children(parent);
    ASSERT_EQ(children.size(), 1);
    EXPECT_EQ(children.front(), child);

    EXPECT_FALSE(script.set_parent(parent, child));
    EXPECT_EQ(script.get_parent(child), parent);

    EXPECT_TRUE(script.clear_parent(child));
    EXPECT_FALSE(script.get_parent(child).is_valid());
    EXPECT_TRUE(script.get_children(parent).empty());
    EXPECT_TRUE(script.clear_parent(child));
}

} // namespace
} // namespace SolTrace::GUI::Data
