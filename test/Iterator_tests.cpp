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