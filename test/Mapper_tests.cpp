#include <gtest/gtest.h>
#include <Mapper.h>

class MapperTest : public ::testing::Test
{

};

TEST_F(MapperTest, InsertAfterRemoval)
{
    Mapper mapper;
    vertex_t v1 = mapper.insert(1);

    mapper.remove(1);

    vertex_t v = mapper.insert(4);

    ASSERT_EQ(v, v1);
}

TEST_F(MapperTest, PresentTest)
{
    Mapper mapper;
    mapper.insert(1);
    mapper.insert(2);

    ASSERT_TRUE(mapper.present(1));
    ASSERT_TRUE(mapper.present(2));

    mapper.remove(2);

    ASSERT_TRUE(mapper.present(1));
    ASSERT_FALSE(mapper.present(2));
}