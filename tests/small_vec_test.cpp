#include "small_vec.hpp"

#include <memory>
#include <numeric>
#include <stdexcept>
#include <string>

#include <gtest/gtest.h>

namespace {
    template<typename T, std::size_t N>
    using SmallVec = small_vec::small_vec<T, N>;

    template<typename T, std::size_t N>
    void expectValues(const SmallVec<T, N>& values, std::initializer_list<T> expected) {
        ASSERT_EQ(values.size(), expected.size());
        auto expectedIt = expected.begin();
        for (std::size_t i = 0; i < values.size(); ++i, ++expectedIt) {
            EXPECT_EQ(values[i], *expectedIt);
        }
    }
}

TEST(SmallVecTest, DefaultConstructionIsEmptyWithInlineCapacity) {
    SmallVec<int, 3> values;
    EXPECT_TRUE(values.empty());
    EXPECT_EQ(values.size(), 0U);
    EXPECT_EQ(values.capacity(), 3U);
}

TEST(SmallVecTest, InitializerListStoresValues) {
    SmallVec<int, 4> values{1, 2, 3};
    expectValues(values, {1, 2, 3});
    EXPECT_EQ(values.capacity(), 4U);
}

TEST(SmallVecTest, InitializerListGrowsPastInlineCapacity) {
    SmallVec<int, 2> values{1, 2, 3, 4};
    expectValues(values, {1, 2, 3, 4});
    EXPECT_GE(values.capacity(), 4U);
}

TEST(SmallVecTest, EmplaceBackGrowsAndPreservesElements) {
    SmallVec<std::string, 2> values;
    values.emplace_back("one");
    values.emplace_back("two");
    const auto* oldData = values.data();
    std::string& inserted = values.emplace_back(3, 'x');

    EXPECT_EQ(inserted, "xxx");
    EXPECT_EQ(values.size(), 3U);
    EXPECT_GT(values.capacity(), 2U);
    EXPECT_NE(values.data(), oldData);
    EXPECT_EQ(values[0], "one");
    EXPECT_EQ(values[1], "two");
    EXPECT_EQ(values[2], "xxx");
}

TEST(SmallVecTest, PushBackSupportsMoveOnlyElements) {
    SmallVec<std::unique_ptr<int>, 1> values;
    values.push_back(std::make_unique<int>(7));
    values.push_back(std::make_unique<int>(9));

    ASSERT_EQ(values.size(), 2U);
    ASSERT_NE(values[0], nullptr);
    ASSERT_NE(values[1], nullptr);
    EXPECT_EQ(*values[0], 7);
    EXPECT_EQ(*values[1], 9);
}

TEST(SmallVecTest, ReserveChangesCapacityWithoutChangingValues) {
    SmallVec<int, 2> values{4, 5};
    values.reserve(8);
    expectValues(values, {4, 5});
    EXPECT_GE(values.capacity(), 8U);
    values.reserve(3);
    EXPECT_GE(values.capacity(), 8U);
}

TEST(SmallVecTest, CopyConstructionCopiesInlineAndHeapStorage) {
    SmallVec<int, 3> inlineValues{1, 2};
    SmallVec<int, 3> inlineCopy(inlineValues);
    inlineCopy[0] = 8;
    EXPECT_EQ(inlineValues[0], 1);
    EXPECT_EQ(inlineCopy[0], 8);

    SmallVec<int, 2> heapValues{3, 4, 5};
    SmallVec<int, 2> heapCopy(heapValues);
    heapCopy[1] = 9;
    EXPECT_EQ(heapValues[1], 4);
    EXPECT_EQ(heapCopy[1], 9);
}

TEST(SmallVecTest, CopyAssignmentReplacesContentsAndSupportsSelfAssignment) {
    SmallVec<int, 2> source{1, 2, 3};
    SmallVec<int, 2> destination{7};
    destination = source;
    expectValues(destination, {1, 2, 3});

    destination = destination;
    expectValues(destination, {1, 2, 3});
}

TEST(SmallVecTest, MoveConstructionTransfersHeapStorage) {
    SmallVec<int, 2> source{1, 2, 3};
    const auto* sourceData = source.data();
    SmallVec<int, 2> destination(std::move(source));

    EXPECT_EQ(destination.data(), sourceData);
    expectValues(destination, {1, 2, 3});
    EXPECT_TRUE(source.empty());
    EXPECT_EQ(source.capacity(), 2U);
}

TEST(SmallVecTest, MoveConstructionMovesInlineStorage) {
    SmallVec<int, 3> source{1, 2};
    SmallVec<int, 3> destination(std::move(source));

    expectValues(destination, {1, 2});
    EXPECT_TRUE(source.empty());
    EXPECT_EQ(source.capacity(), 3U);
}

TEST(SmallVecTest, MoveAssignmentTransfersValues) {
    SmallVec<int, 2> source{1, 2, 3};
    SmallVec<int, 2> destination{8};
    destination = std::move(source);

    expectValues(destination, {1, 2, 3});
    EXPECT_LE(source.size(), source.capacity());
}

TEST(SmallVecTest, ResizeDefaultConstructsAndDestroysElements) {
    SmallVec<std::string, 2> values{"a"};
    values.resize(3);
    ASSERT_EQ(values.size(), 3U);
    EXPECT_EQ(values[0], "a");
    EXPECT_TRUE(values[1].empty());
    EXPECT_TRUE(values[2].empty());

    values.resize(1);
    expectValues(values, {std::string("a")});
}

TEST(SmallVecTest, FrontBackAndPopBackWork) {
    SmallVec<int, 3> values{2, 4, 6};
    EXPECT_EQ(values.front(), 2);
    EXPECT_EQ(values.back(), 6);
    values.pop_back();
    EXPECT_EQ(values.back(), 4);
    EXPECT_EQ(values.size(), 2U);
}

TEST(SmallVecTest, AtChecksBounds) {
    SmallVec<int, 2> values{3};
    EXPECT_EQ(values.at(0), 3);
    EXPECT_THROW(values.at(1), std::out_of_range);
}

TEST(SmallVecTest, ClearLeavesContainerEmptyAndReusable) {
    SmallVec<int, 2> values{1, 2, 3};
    values.clear();
    EXPECT_TRUE(values.empty());
    values.push_back(5);
    expectValues(values, {5});
}

TEST(SmallVecTest, SwapHandlesInlineAndHeapStorage) {
    SmallVec<int, 2> inlineValues{1};
    SmallVec<int, 2> heapValues{7, 8, 9};
    inlineValues.swap(heapValues);

    expectValues(inlineValues, {7, 8, 9});
    expectValues(heapValues, {1});
}

TEST(SmallVecTest, SwapHandlesTwoInlineBuffersWithDifferentSizes) {
    SmallVec<int, 4> first{1};
    SmallVec<int, 4> second{7, 8, 9};
    using std::swap;
    swap(first, second);

    expectValues(first, {7, 8, 9});
    expectValues(second, {1});
}

TEST(SmallVecTest, IteratorsSupportForwardAndReverseTraversal) {
    SmallVec<int, 3> values{1, 2, 3};
    EXPECT_EQ(values.end() - values.begin(), 3);
    EXPECT_EQ(std::accumulate(values.begin(), values.end(), 0), 6);
    EXPECT_EQ(*values.rbegin(), 3);
    EXPECT_EQ(*values.rend().base(), 1);
}

TEST(SmallVecTest, EqualityComparesContents) {
    SmallVec<int, 2> first{1, 2, 3};
    SmallVec<int, 2> same{1, 2, 3};
    SmallVec<int, 2> different{1, 2, 4};
    EXPECT_TRUE(first == same);
    EXPECT_FALSE(first == different);
}
