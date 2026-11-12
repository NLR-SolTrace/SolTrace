#include <gtest/gtest.h>

#include <limits>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <vector>

#include <model_context.hpp>
#include <model_identity.hpp>
#include <model_state.hpp>

#include "common.hpp"

using SolTrace::Data::Element;
using SolTrace::Data::ElementIdAllocator;
using SolTrace::Data::ElementKey;
using SolTrace::Data::ModelContext;
using SolTrace::Data::ModelIdentity;
using SolTrace::Data::ModelState;
using SolTrace::Data::ModelToken;
using SolTrace::Data::PendingElement;
using SolTrace::Data::same_model;

namespace {

// An element that can be created through a ModelContext
class KeyedElement : public SingleElement
{
public:
    explicit KeyedElement(ElementKey key) : SingleElement(key)
    {
        this->key_token = key.token();
    }

    ModelToken key_token;
};

// A model state with a registrar that records what it was given
struct TestModel
{
    std::shared_ptr<ModelState> state = std::make_shared<ModelState>();
    ModelContext context{state};
    std::vector<std::shared_ptr<Element>> registered;
    int calls = 0;
    bool fail = false;

    TestModel()
    {
        state->registrar = [this](std::shared_ptr<Element> el) {
            ++this->calls;
            if (this->fail)
            {
                throw std::runtime_error("registration failed");
            }
            this->registered.push_back(el);
            return el->get_id();
        };
    }
};

} // namespace

// **** ElementIdAllocator **** //

TEST(ElementIdAllocator, FirstIdIsOneAndIdsAreSequential)
{
    ElementIdAllocator ids;
    EXPECT_EQ(ids.next(), 1);
    EXPECT_EQ(ids.next(), 2);
    EXPECT_EQ(ids.next(), 3);
}

TEST(ElementIdAllocator, ReserveAdvancesNextAndNeverFillsGaps)
{
    ElementIdAllocator ids;
    ids.reserve(5);
    EXPECT_EQ(ids.next(), 6);

    // A lower ID that is still free can be reserved, and next() keeps going up
    ids.reserve(3);
    EXPECT_EQ(ids.next(), 7);

    ElementIdAllocator other;
    other.reserve(10);
    EXPECT_EQ(other.next(), 11);
}

TEST(ElementIdAllocator, ReserveRejectsBadAndDuplicateIds)
{
    ElementIdAllocator ids;
    EXPECT_THROW(ids.reserve(0), std::invalid_argument);
    EXPECT_THROW(ids.reserve(-1), std::invalid_argument);
    EXPECT_THROW(ids.reserve(-2), std::invalid_argument);

    EXPECT_EQ(ids.next(), 1);
    EXPECT_THROW(ids.reserve(1), std::invalid_argument);

    ids.reserve(8);
    EXPECT_THROW(ids.reserve(8), std::invalid_argument);

    // A rejected reserve changes nothing
    EXPECT_EQ(ids.next(), 9);
}

TEST(ElementIdAllocator, ResetStartsOver)
{
    ElementIdAllocator ids;
    ids.next();
    ids.next();
    ids.reserve(20);

    ids.reset();
    EXPECT_EQ(ids.next(), 1);
    // The IDs from before the reset are free again
    ids.reserve(20);
    EXPECT_EQ(ids.next(), 21);
}

TEST(ElementIdAllocator, ThrowsWhenNoLargerIdExists)
{
    ElementIdAllocator ids;
    ids.reserve(std::numeric_limits<SolTrace::Data::element_id>::max());
    EXPECT_THROW(ids.next(), std::overflow_error);
}

// **** same_model **** //

TEST(ModelIdentity, SameModel)
{
    auto a = std::make_shared<const ModelIdentity>();
    auto b = std::make_shared<const ModelIdentity>();
    ModelToken ta = a;
    ModelToken ta2 = a;
    ModelToken tb = b;

    EXPECT_TRUE(same_model(ta, ta2));
    EXPECT_TRUE(same_model(ta, ta));
    EXPECT_FALSE(same_model(ta, tb));
    EXPECT_FALSE(same_model(tb, ta));
}

TEST(ModelIdentity, ExpiredAndEmptyTokensAreNeverTheSameModel)
{
    ModelToken expired;
    {
        auto a = std::make_shared<const ModelIdentity>();
        expired = a;
    }
    ModelToken empty;
    auto live = std::make_shared<const ModelIdentity>();
    ModelToken tlive = live;

    EXPECT_FALSE(same_model(expired, expired));
    EXPECT_FALSE(same_model(expired, tlive));
    EXPECT_FALSE(same_model(empty, empty));
    EXPECT_FALSE(same_model(empty, tlive));
}

// **** ModelState **** //

TEST(ModelState, ResetGivesNewIdentityAndRestartsIds)
{
    ModelState state;
    ModelToken before = state.token();
    EXPECT_TRUE(same_model(before, state.token()));
    EXPECT_EQ(state.ids.next(), 1);
    EXPECT_EQ(state.ids.next(), 2);

    state.reset();
    EXPECT_FALSE(same_model(before, state.token()));
    EXPECT_TRUE(before.expired());
    EXPECT_EQ(state.ids.next(), 1);
}

TEST(ModelState, TwoStatesAreDifferentModels)
{
    ModelState a;
    ModelState b;
    EXPECT_FALSE(same_model(a.token(), b.token()));
}

// **** ModelContext **** //

TEST(ModelContext, CreateIssuesSequentialIdsAndStampsTheToken)
{
    TestModel model;
    auto first = model.context.create<KeyedElement>();
    auto second = model.context.create<KeyedElement>();

    EXPECT_EQ(first.id(), 1);
    EXPECT_EQ(second.id(), 2);
    EXPECT_EQ(first->get_id(), 1);
    EXPECT_EQ(second->get_id(), 2);
    EXPECT_TRUE(same_model(first->key_token, model.state->token()));
    EXPECT_TRUE(same_model(first->key_token, second->key_token));
}

TEST(ModelContext, CreateWithIdReservesTheId)
{
    TestModel model;
    auto saved = model.context.create_with_id<KeyedElement>(7);
    EXPECT_EQ(saved.id(), 7);
    EXPECT_EQ(saved->get_id(), 7);

    // New IDs continue above the highest one
    EXPECT_EQ(model.context.create<KeyedElement>().id(), 8);

    EXPECT_THROW((void)model.context.create_with_id<KeyedElement>(7),
                 std::invalid_argument);
    EXPECT_THROW((void)model.context.create_with_id<KeyedElement>(8),
                 std::invalid_argument);
    EXPECT_THROW((void)model.context.create_with_id<KeyedElement>(0),
                 std::invalid_argument);
    EXPECT_THROW((void)model.context.create_with_id<KeyedElement>(-3),
                 std::invalid_argument);
}

TEST(ModelContext, CreateThrowsWhenTheModelIsGone)
{
    ModelContext context{std::make_shared<ModelState>()};
    // The temporary state above is already destroyed
    EXPECT_THROW((void)context.create<KeyedElement>(), std::invalid_argument);
    EXPECT_THROW((void)context.create_with_id<KeyedElement>(1),
                 std::invalid_argument);
    EXPECT_TRUE(context.token().expired());
    EXPECT_FALSE(context.is_registered(1));
}

TEST(ModelContext, IsRegisteredAsksTheModel)
{
    TestModel model;
    EXPECT_FALSE(model.context.is_registered(1));

    model.state->is_registered = [](SolTrace::Data::element_id id) {
        return id == 4;
    };
    EXPECT_TRUE(model.context.is_registered(4));
    EXPECT_FALSE(model.context.is_registered(5));
}

TEST(ModelContext, ContextDoesNotKeepTheModelAlive)
{
    auto state = std::make_shared<ModelState>();
    ModelContext context{state};
    ModelToken token = context.token();
    EXPECT_FALSE(token.expired());

    state.reset();
    EXPECT_TRUE(token.expired());
    EXPECT_TRUE(context.token().expired());
}

// **** PendingElement **** //

TEST(PendingElement, UncommittedElementRegistersNothing)
{
    TestModel model;
    {
        auto pending = model.context.create<KeyedElement>();
        EXPECT_NE(pending.get(), nullptr);
    }
    EXPECT_EQ(model.calls, 0);
    EXPECT_TRUE(model.registered.empty());
}

TEST(PendingElement, CommitRegistersTheElementOnceAndReturnsItsId)
{
    TestModel model;
    auto pending = model.context.create<KeyedElement>();
    auto element = pending.get();

    EXPECT_EQ(pending.commit(), 1);
    ASSERT_EQ(model.registered.size(), 1u);
    EXPECT_EQ(model.registered[0].get(), element.get());

    // The handle still reaches the element but cannot commit again
    EXPECT_EQ(pending.get().get(), element.get());
    EXPECT_THROW(pending.commit(), std::invalid_argument);
    EXPECT_EQ(model.calls, 1);
}

TEST(PendingElement, MovedFromHandleIsEmpty)
{
    TestModel model;
    auto first = model.context.create<KeyedElement>();
    auto second = std::move(first);

    EXPECT_EQ(first.get(), nullptr);
    EXPECT_THROW(first.commit(), std::invalid_argument);
    EXPECT_EQ(second.commit(), 1);
    EXPECT_EQ(model.calls, 1);
}

TEST(PendingElement, ConvertsToAHandleOfABaseType)
{
    TestModel model;
    auto derived = model.context.create<KeyedElement>();
    auto element = derived.get();

    PendingElement<Element> base = std::move(derived);
    EXPECT_EQ(derived.get(), nullptr);
    EXPECT_EQ(base.get().get(), element.get());
    EXPECT_EQ(base.id(), 1);
    EXPECT_EQ(base.commit(), 1);
    ASSERT_EQ(model.registered.size(), 1u);
    EXPECT_EQ(model.registered[0].get(), element.get());
}

TEST(PendingElement, FailedCommitKeepsTheHandleAndTheId)
{
    TestModel model;
    auto pending = model.context.create<KeyedElement>();

    model.fail = true;
    EXPECT_THROW(pending.commit(), std::runtime_error);
    EXPECT_TRUE(model.registered.empty());
    EXPECT_EQ(pending.id(), 1);
    EXPECT_NE(pending.get(), nullptr);

    // The caller can fix the problem and try again
    model.fail = false;
    EXPECT_EQ(pending.commit(), 1);
    EXPECT_EQ(model.registered.size(), 1u);
    EXPECT_EQ(model.calls, 2);
}

TEST(PendingElement, CommitThrowsWhenTheModelIsGone)
{
    TestModel model;
    auto pending = model.context.create<KeyedElement>();

    model.state.reset();
    EXPECT_THROW(pending.commit(), std::invalid_argument);
    EXPECT_EQ(model.calls, 0);
}

TEST(PendingElement, CommitThrowsAfterTheModelWasCleared)
{
    TestModel model;
    auto old_handle = model.context.create<KeyedElement>();

    model.state->reset();
    EXPECT_THROW(old_handle.commit(), std::invalid_argument);
    EXPECT_EQ(model.calls, 0);

    // Elements created after the clear belong to the model, with IDs from 1
    auto new_handle = model.context.create<KeyedElement>();
    EXPECT_EQ(new_handle.id(), 1);
    EXPECT_EQ(new_handle.commit(), 1);
}

TEST(PendingElement, CommitThrowsWhenTheModelCannotRegisterElements)
{
    TestModel model;
    model.state->registrar = nullptr;
    auto pending = model.context.create<KeyedElement>();
    EXPECT_THROW(pending.commit(), std::invalid_argument);
}

TEST(PendingElement, HandleAndKeyRules)
{
    using Handle = PendingElement<KeyedElement>;
    static_assert(!std::is_copy_constructible_v<Handle>);
    static_assert(!std::is_copy_assignable_v<Handle>);
    static_assert(std::is_move_constructible_v<Handle>);
    static_assert(std::is_move_assignable_v<Handle>);

    // A handle converts to a base type but not the other way
    static_assert(std::is_convertible_v<Handle &&, PendingElement<Element>>);
    static_assert(!std::is_convertible_v<PendingElement<Element> &&, Handle>);

    // Only a ModelContext can make a key
    static_assert(!std::is_constructible_v<ElementKey,
                                           SolTrace::Data::element_id,
                                           ModelContext,
                                           ModelToken>);
    SUCCEED();
}

// **** SimulationData **** //

TEST(SimulationDataModel, CopyIsDeletedAndMoveIsAllowed)
{
    static_assert(!std::is_copy_constructible_v<SimulationData>);
    static_assert(!std::is_copy_assignable_v<SimulationData>);
    static_assert(std::is_move_constructible_v<SimulationData>);
    static_assert(std::is_move_assignable_v<SimulationData>);
    SUCCEED();
}

TEST(SimulationDataModel, CreateElementIssuesIdsFromOne)
{
    SimulationData sd;
    auto first = sd.create_element<KeyedElement>();
    auto second = sd.create_element<KeyedElement>();
    auto saved = sd.create_element_with_id<KeyedElement>(9);

    EXPECT_EQ(first.id(), 1);
    EXPECT_EQ(second.id(), 2);
    EXPECT_EQ(saved.id(), 9);
    EXPECT_EQ(sd.create_element<KeyedElement>().id(), 10);

    // Registration is not connected yet
    EXPECT_THROW(first.commit(), std::invalid_argument);
    EXPECT_EQ(sd.get_number_of_elements(), 0);
}

TEST(SimulationDataModel, ContextAsksTheRegistry)
{
    SimulationData sd;
    auto context = sd.context();
    auto id = register_element(sd, make_configured_element(sd));
    ASSERT_TRUE(SolTrace::Data::Element::is_success(id));

    EXPECT_TRUE(context.is_registered(id));
    EXPECT_FALSE(context.is_registered(id + 100));

    sd.remove_element(id);
    EXPECT_FALSE(context.is_registered(id));
}

TEST(SimulationDataModel, ClearGivesTheModelANewIdentity)
{
    SimulationData sd;
    auto context = sd.context();
    ModelToken before = context.token();
    auto old_handle = sd.create_element<KeyedElement>();
    EXPECT_EQ(old_handle.id(), 1);

    sd.clear();

    EXPECT_FALSE(same_model(before, context.token()));
    EXPECT_THROW(old_handle.commit(), std::invalid_argument);
    EXPECT_EQ(sd.create_element<KeyedElement>().id(), 1);
}

TEST(SimulationDataModel, MoveKeepsContextsAndTokensValid)
{
    SimulationData sd;
    auto id = register_element(sd, make_configured_element(sd));
    ASSERT_TRUE(SolTrace::Data::Element::is_success(id));
    auto context = sd.context();
    ModelToken token = context.token();
    auto pending = sd.create_element<KeyedElement>();

    SimulationData moved(std::move(sd));

    EXPECT_EQ(moved.get_number_of_elements(), 1);
    EXPECT_NE(moved.get_element(id), nullptr);
    EXPECT_TRUE(same_model(token, context.token()));
    EXPECT_TRUE(same_model(token, moved.context().token()));
    // The context now asks the new object
    EXPECT_TRUE(context.is_registered(id));
    EXPECT_EQ(context.create<KeyedElement>().id(), pending.id() + 1);
    EXPECT_TRUE(same_model(pending->key_token, context.token()));
}

TEST(SimulationDataModel, MovedFromObjectIsEmptyAndUsable)
{
    SimulationData sd;
    auto id = register_element(sd, make_configured_element(sd));
    ASSERT_TRUE(SolTrace::Data::Element::is_success(id));
    ModelToken token = sd.context().token();

    SimulationData moved(std::move(sd));

    EXPECT_EQ(sd.get_number_of_elements(), 0);
    EXPECT_EQ(sd.get_element(id), nullptr);
    EXPECT_FALSE(same_model(token, sd.context().token()));
    EXPECT_FALSE(sd.context().is_registered(id));

    // It is a working, empty model
    auto new_id = register_element(sd, make_configured_element(sd));
    EXPECT_TRUE(SolTrace::Data::Element::is_success(new_id));
    EXPECT_EQ(sd.get_number_of_elements(), 1);
    EXPECT_TRUE(sd.context().is_registered(new_id));
    // and the object it was moved to is not affected
    EXPECT_EQ(moved.get_number_of_elements(), 1);
}

TEST(SimulationDataModel, MoveAssignmentReplacesTheModel)
{
    SimulationData first;
    auto first_id = register_element(first, make_configured_element(first));
    auto first_context = first.context();

    SimulationData second;
    auto second_id = register_element(second, make_configured_element(second));
    auto second_context = second.context();
    ModelToken second_token = second_context.token();
    ASSERT_TRUE(SolTrace::Data::Element::is_success(first_id));
    ASSERT_TRUE(SolTrace::Data::Element::is_success(second_id));

    second = std::move(first);

    // What was made for the model that `second` had is gone
    EXPECT_TRUE(second_token.expired());
    EXPECT_TRUE(second_context.token().expired());
    EXPECT_FALSE(second_context.is_registered(second_id));
    // What was made for `first` now belongs to `second`
    EXPECT_TRUE(first_context.is_registered(first_id));
    EXPECT_TRUE(same_model(first_context.token(), second.context().token()));
    EXPECT_EQ(second.get_number_of_elements(), 1);
    EXPECT_EQ(first.get_number_of_elements(), 0);
}
