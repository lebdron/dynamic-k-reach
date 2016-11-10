#include <gtest/gtest.h>
#include <DynamicKReachReindex.h>
#include <fstream>

class DynamicKReachReindexTest : public ::testing::Test
{

};

TEST_F(DynamicKReachReindexTest, ReferenceInsertTest)
{
    std::vector<Edge> edges;
    {
        std::ifstream fin("data/sample");
        ASSERT_TRUE(fin.is_open());
        for (vertex_t s, t; fin >> s >> t;) {
            edges.push_back(Edge(s, t));
        }
    }

    KReach kr;
    kr.construct_index(edges, 3);

    DynamicKReachReindex dkrr;
    dkrr.construct_index(edges, 3);

    ASSERT_EQ(kr.index[kr.mapper.query(7)], dkrr.index[dkrr.mapper.query(7)]);
    ASSERT_EQ(kr.index[kr.mapper.query(9)], dkrr.index[dkrr.mapper.query(9)]);
    ASSERT_EQ(kr.index[kr.mapper.query(2)], dkrr.index[dkrr.mapper.query(2)]);
    ASSERT_EQ(kr.index[kr.mapper.query(4)], dkrr.index[dkrr.mapper.query(4)]);

    ASSERT_FALSE(dkrr.graph[dkrr.mapper.query(3)].out.count(dkrr.mapper.query(8)));

    dkrr.insert_edge(3, 8);

    ASSERT_EQ(kr.index[kr.mapper.query(7)].out, dkrr.index[dkrr.mapper.query(7)].out);
    ASSERT_EQ(kr.index[kr.mapper.query(7)].in, dkrr.index[dkrr.mapper.query(7)].in);
    ASSERT_EQ(kr.index[kr.mapper.query(9)], dkrr.index[dkrr.mapper.query(9)]);
    ASSERT_NE(kr.index[kr.mapper.query(2)], dkrr.index[dkrr.mapper.query(2)]);
    ASSERT_NE(kr.index[kr.mapper.query(4)], dkrr.index[dkrr.mapper.query(4)]);

    ASSERT_TRUE(dkrr.graph[dkrr.mapper.query(3)].out.count(dkrr.mapper.query(8)));

    ASSERT_TRUE(dkrr.index.count(dkrr.mapper.query(3)));

    ASSERT_FALSE(dkrr.index.count(dkrr.mapper.query(8)));

    AdjacencyList three;
    three.out.insert({dkrr.mapper.query(2), dkrr.mapper.query(4), dkrr.mapper.query(3)});
    three.in.insert({dkrr.mapper.query(2), dkrr.mapper.query(4), dkrr.mapper.query(3)});

    ASSERT_EQ(dkrr.index[dkrr.mapper.query(3)].out, three.out);
    ASSERT_EQ(dkrr.index[dkrr.mapper.query(3)].in, three.in);

    ASSERT_TRUE(dkrr.query(2, 8));
    ASSERT_FALSE(dkrr.query(1, 8));
}

TEST_F(DynamicKReachReindexTest, ReferenceRemoveTest)
{
    std::vector<Edge> edges;
    {
        std::ifstream fin("data/sample");
        ASSERT_TRUE(fin.is_open());
        for (vertex_t s, t; fin >> s >> t;) {
            edges.push_back(Edge(s, t));
        }
    }

    DynamicKReachReindex dkrr;
    dkrr.construct_index(edges, 3);

    dkrr.remove_edge(4, 5);

    for (const auto &p : {1, 2, 3, 4, 6}){
        for (const auto &q : {5, 7, 8, 9, 10}){
            ASSERT_FALSE(dkrr.query(p, q));
        }
    }

    dkrr.remove_edge(5, 7);
    for (const auto &v : {1, 2, 3, 4, 6, 7, 8, 9, 10}){
        ASSERT_FALSE(dkrr.query(5, v));
        ASSERT_FALSE(dkrr.query(v, 5));
    }
    ASSERT_TRUE(dkrr.query(5, 5));
}