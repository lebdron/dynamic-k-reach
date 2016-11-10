#include <gtest/gtest.h>
#include <UnionIterator.h>
#include <IntersectionIterator.h>

class IteratorTest : public ::testing::Test
{

};

TEST_F(IteratorTest, UnionTest)
{
    Adjacent a({1, 2, 3});
    Adjacent b({2, 3, 4});
    UnionIterator begin(a.begin(), a.end(), b.begin(), b.end()), end(a.end(), a.end(), b.end(), b.end());
    Adjacent neighbors;
    std::set_union(a.begin(), a.end(), b.begin(), b.end(), std::inserter(neighbors, neighbors.end()));
    ASSERT_TRUE(std::equal(neighbors.begin(), neighbors.end(), begin));
    ASSERT_TRUE(std::equal(begin, end, neighbors.begin()));
}

TEST_F(IteratorTest, IntersectionTest)
{
    Adjacent a({1, 2, 3});
    Adjacent b({2, 3, 4});
    IntersectionIterator begin(a.begin(), a.end(), b.begin(), b.end()), end(a.end(), a.end(), b.end(), b.end());
    Adjacent common;
    std::set_intersection(a.begin(), a.end(), b.begin(), b.end(), std::inserter(common, common.end()));
    ASSERT_TRUE(std::equal(common.begin(), common.end(), begin));
    ASSERT_TRUE(std::equal(begin, end, common.begin()));
}

TEST_F(IteratorTest, EraseDuringTraversal)
{
    Adjacent a({2, 3, 4, 5, 6, 7}), b({5, 6, 7});
    IntersectionIterator begin(a.begin(), a.end(), b.begin(), b.end()), end(a.end(), a.end(), b.end(), b.end());
    ASSERT_EQ(*begin, vertex_t(5));
    a.erase(5);
    ++begin;
    ASSERT_EQ(*begin, vertex_t(6));
    a.insert({8, 9, 10});
    b.insert({8, 9, 10});
    vertex_t i = 7;
    ++begin;
    while (begin != end){
        ASSERT_EQ(*begin, i);
        ++begin;
        ++i;
    }
}

TEST_F(IteratorTest, EmptyIntersection)
{
    Adjacent a({1, 2, 3}), b({4, 5, 6});
    IntersectionIterator begin(a.begin(), a.end(), b.begin(), b.end()), end(a.end(), a.end(), b.end(), b.end());
    ASSERT_TRUE(begin == end);
    ASSERT_FALSE(begin != end);
}