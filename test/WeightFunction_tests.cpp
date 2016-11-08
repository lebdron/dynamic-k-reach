#include <gtest/gtest.h>
#include "WeightFunction.h"

class WeightFunctionTest : public ::testing::Test
{

};

TEST_F(WeightFunctionTest, IsSymmetric)
{
    WeightFunction f;
    f(1, 2) = 5;
    ASSERT_TRUE(f.defined(2, 1) == f.defined(1, 2));
    ASSERT_EQ(f(1, 2), f(2, 1));
}

TEST_F(WeightFunctionTest, HasherValues)
{
    EdgeHasher hasher;
    Edge a(0, 0);
    Edge b(1, 1);
    ASSERT_EQ(hasher(a) + 1, hasher(b));
    Edge c(10, 20);
    Edge d(20, 10);
    ASSERT_EQ(hasher(c), hasher(d));
}