
#include <gtest/gtest.h>

#include <container.hpp>

#include "common.hpp"

TEST(Container, Interface)
{
    using key = int;
    const int NUM_ITEMS = 5;

    struct TestItem
    {
        key a;
        TestItem(key b) : a(b) {}
    };

    using TestContainer = SolTrace::Data::Container<key, TestItem>;

    TestContainer my_container;
    std::set<key> ids;

    for (key i = 0; i < NUM_ITEMS; ++i)
    {
        key kval = my_container.add_item(TestContainer::make_pointer(i));
        ids.insert(kval);
    }

    // Test number of items is correct and so insertion works correctly
    EXPECT_EQ(ids.size(), my_container.get_number_of_items());

    // Test getting item in container works correctly
    for (auto iter = ids.begin(); iter != ids.end(); ++iter)
    {
        TestContainer::value_pointer ptr = my_container.get_item(*iter);
        EXPECT_EQ(ptr->a, *iter);
    }

    // Test behavior for getting item that is not in the container
    EXPECT_EQ(my_container.get_item(NUM_ITEMS + 1), nullptr);

    // Iterator covers whole container
    auto citer = my_container.get_iterator();
    while (!my_container.is_at_end(citer))
    {
        auto nitems = ids.erase(citer->first);
        EXPECT_EQ(nitems, 1);
        ++citer;
    }
    EXPECT_EQ(ids.size(), 0);

    // Constant iterator covers whole container
    auto const_iter = my_container.get_const_iterator();
    while (!my_container.is_at_end(const_iter))
    {
        ids.insert(const_iter->first);
        ++const_iter;
    }
    EXPECT_EQ(ids.size(), my_container.get_number_of_items());

    // Test for removing item that isn't in container
    EXPECT_FALSE(my_container.remove_item(NUM_ITEMS + 1));
    EXPECT_TRUE(my_container.remove_item(0));
    EXPECT_EQ(my_container.get_item(0), nullptr);
    EXPECT_EQ(my_container.get_number_of_items(), NUM_ITEMS - 1);
}
