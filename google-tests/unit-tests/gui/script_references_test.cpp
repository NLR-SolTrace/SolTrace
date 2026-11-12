#include "data/database.h"
#include "scripting/script_db_interface.h"
#include "scripting/script_executor.h"
#include "scripting/script_references.h"

#include <gtest/gtest.h>

namespace SolTrace::GUI::Scripting
{
namespace
{

TEST(ScriptReferences, SinglesReplaceReferencesWithoutDeletingEntities)
{
    SolTrace::GUI::Data::Database database(QStringLiteral("References"));
    ScriptDBInterface             db(&database);
    ScriptReferenceInterface      references(&database);

    auto const first  = db.create();
    auto const second = db.create();
    auto*      single = references.scope(QStringLiteral("nlr.field"))
                            ->single(QStringLiteral("receiver"));

    ASSERT_TRUE(single->set(first));
    EXPECT_EQ(single->get(), first);

    ASSERT_TRUE(single->set(second));
    EXPECT_EQ(single->get(), second);
    EXPECT_TRUE(database.valid(first));
    EXPECT_TRUE(database.valid(second));

    EXPECT_TRUE(single->clear());
    EXPECT_FALSE(single->get().is_valid());
    EXPECT_FALSE(single->clear());
}

TEST(ScriptReferences, GroupsAreSharedIdempotentAndNonDestructive)
{
    SolTrace::GUI::Data::Database database(QStringLiteral("References"));
    ScriptDBInterface             db(&database);
    ScriptReferenceInterface      first_interface(&database);
    ScriptReferenceInterface      second_interface(&database);

    auto const first  = db.create();
    auto const second = db.create();
    auto*      group  = first_interface.scope(QStringLiteral("nlr.field"))
                            ->group(QStringLiteral("mirrors"));

    EXPECT_TRUE(group->add(first));
    EXPECT_TRUE(group->add(first));
    EXPECT_TRUE(group->add(second));
    EXPECT_EQ(group->entities().size(), 2);

    auto* shared = second_interface.scope(QStringLiteral("nlr.field"))
                       ->group(QStringLiteral("mirrors"));
    EXPECT_TRUE(shared->contains(first));
    EXPECT_TRUE(shared->contains(second));
    EXPECT_TRUE(shared->remove(first));
    EXPECT_FALSE(shared->remove(first));
    EXPECT_TRUE(database.valid(first));
    EXPECT_EQ(group->entities(),
              QVector<SolTrace::GUI::Data::Entity>({ second }));
}

TEST(ScriptReferences, DeletedEntitiesDisappearAndNamesAreDiscoverable)
{
    SolTrace::GUI::Data::Database database(QStringLiteral("References"));
    ScriptDBInterface             db(&database);
    ScriptReferenceInterface      references(&database);

    auto const entity = db.create();
    auto*      scope  = references.scope(QStringLiteral("nlr.field"));
    ASSERT_TRUE(scope->single(QStringLiteral("receiver"))->set(entity));
    ASSERT_TRUE(scope->group(QStringLiteral("mirrors"))->add(entity));

    EXPECT_EQ(references.scopes(),
              QStringList({ QStringLiteral("nlr.field") }));
    EXPECT_EQ(scope->single_names(),
              QStringList({ QStringLiteral("receiver") }));
    EXPECT_EQ(scope->group_names(), QStringList({ QStringLiteral("mirrors") }));

    db.destroy_entity(entity);
    EXPECT_FALSE(scope->single(QStringLiteral("receiver"))->get().is_valid());
    EXPECT_TRUE(scope->group(QStringLiteral("mirrors"))->entities().isEmpty());
    EXPECT_TRUE(references.scopes().isEmpty());
}

TEST(ScriptReferences, ExistingHelpersFollowTheCurrentDatabase)
{
    SolTrace::GUI::Data::Database first_database(QStringLiteral("First"));
    SolTrace::GUI::Data::Database second_database(QStringLiteral("Second"));
    ScriptDBInterface             first_db(&first_database);
    ScriptDBInterface             second_db(&second_database);
    ScriptReferenceInterface      references(&first_database);

    auto const first_entity  = first_db.create();
    auto const second_entity = second_db.create();
    auto*      receiver      = references.scope(QStringLiteral("nlr.field"))
                         ->single(QStringLiteral("receiver"));

    ASSERT_TRUE(receiver->set(first_entity));
    references.set_database(&second_database);
    EXPECT_FALSE(receiver->get().is_valid());
    ASSERT_TRUE(receiver->set(second_entity));

    references.set_database(&first_database);
    EXPECT_EQ(receiver->get(), first_entity);
    references.set_database(&second_database);
    EXPECT_EQ(receiver->get(), second_entity);
}

TEST(ScriptReferences, SurviveStatelessScriptInvocations)
{
    SolTrace::GUI::Data::Database database(QStringLiteral("References"));

    auto first = ScriptExecutor::execute(ScriptExecutionRequest {
        .source      = QStringLiteral("(function() {"
                                      " const entity = db.create();"
                                      " const field = refs.scope('nlr.field');"
                                      " field.single('receiver').set(entity);"
                                      " field.group('members').add(entity);"
                                      "})"),
        .source_name = QStringLiteral("Reference setup"),
        .database    = &database,
    });
    ASSERT_TRUE(first.success) << first.error.toStdString();

    auto second = ScriptExecutor::execute(ScriptExecutionRequest {
        .source = QStringLiteral(
            "(function() {"
            " const field = refs.scope('nlr.field');"
            " const receiver = field.single('receiver').get();"
            " if (!db.valid(receiver)) throw new Error('missing single');"
            " if (field.group('members').entities().length !== 1)"
            "   throw new Error('missing group member');"
            " db.set_identity(receiver, 'Shared Receiver');"
            "})"),
        .source_name = QStringLiteral("Reference lookup"),
        .database    = &database,
    });
    ASSERT_TRUE(second.success) << second.error.toStdString();

    auto const elements =
        database.as_registry().view<SolTrace::GUI::Data::ElementComponent>();
    ASSERT_EQ(elements.size(), 1u);
    EXPECT_EQ(database.name_of(*elements.begin()),
              QStringLiteral("Shared Receiver"));
}

} // namespace
} // namespace SolTrace::GUI::Scripting
