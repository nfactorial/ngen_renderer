// Spec: rhi. Behaviour changes go through it.
#include <ngen/rhi/handle.h>

#include <gtest/gtest.h>

#include <string>

using namespace ngen::rhi;

namespace
{
    using TestHandle = Handle<struct TestTag>;
    using TestPool = HandlePool<TestTag, std::string>;
} // namespace

TEST(Handle, DefaultConstructedIsNullAndZero)
{
    const TestHandle handle;
    EXPECT_TRUE(handle.isNull());
    EXPECT_EQ(handle.index, 0u);
    EXPECT_EQ(handle.generation, 0u);
    EXPECT_EQ(handle, (TestHandle{0, 0}));
}

TEST(Handle, IssuedHandlesAreNotNullAndHaveGenerationOfOneOrMore)
{
    TestPool pool;
    const TestHandle first = pool.insert("a");
    EXPECT_FALSE(first.isNull());
    EXPECT_GE(first.generation, 1u);
}

TEST(HandlePool, ReusedSlotGetsHigherGenerationAndOldHandleStaysStale)
{
    TestPool pool;
    const TestHandle a = pool.insert("A");
    const TestHandle b = pool.insert("B");
    ASSERT_TRUE(pool.remove(a).has_value());
    const TestHandle c = pool.insert("C");

    EXPECT_EQ(c.index, a.index);
    EXPECT_GT(c.generation, a.generation);
    EXPECT_EQ(pool.get(a), nullptr);
    ASSERT_NE(pool.get(c), nullptr);
    EXPECT_EQ(*pool.get(c), "C");
    ASSERT_NE(pool.get(b), nullptr);
    EXPECT_EQ(*pool.get(b), "B");
    EXPECT_EQ(pool.size(), 2u);
}

TEST(HandlePool, RemoveTwiceReturnsValueThenEmpty)
{
    TestPool pool;
    const TestHandle handle = pool.insert("value");

    const std::optional<std::string> first = pool.remove(handle);
    ASSERT_TRUE(first.has_value());
    EXPECT_EQ(*first, "value");
    EXPECT_FALSE(pool.remove(handle).has_value());
    EXPECT_EQ(pool.size(), 0u);
}

TEST(HandlePool, GetRejectsNullAndOutOfRangeHandles)
{
    TestPool pool;
    (void)pool.insert("x");
    EXPECT_EQ(pool.get(TestHandle{}), nullptr);
    EXPECT_EQ(pool.get(TestHandle{99, 1}), nullptr);
    EXPECT_FALSE(pool.remove(TestHandle{}).has_value());
    EXPECT_FALSE(pool.remove(TestHandle{99, 1}).has_value());
    EXPECT_EQ(pool.size(), 1u);
}

TEST(HandlePool, GetRejectsWrongGenerationOfLiveSlot)
{
    TestPool pool;
    const TestHandle handle = pool.insert("x");
    EXPECT_EQ(pool.get(TestHandle{handle.index, handle.generation + 1}), nullptr);
}
