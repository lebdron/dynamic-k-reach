#include <gtest/gtest.h>
#include <KReach.h>
#include <fstream>

class KReachTest : public ::testing::Test
{

};

TEST_F(KReachTest, ReferenceIndexTest)
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

    Mapper mapper;

    {
        using degree_t = uint32_t;
        std::unordered_map<vertex_t, degree_t> degree;
        for (const auto &e : edges){
            degree[e.first]++;
            degree[e.second]++;
        }
        std::multimap<degree_t, vertex_t, std::greater<degree_t>> order;
        for (const auto &v : degree){
            order.insert(std::make_pair(v.second, v.first));
        }
        for (const auto &v : order){
            mapper.insert(v.second);
        }
        ASSERT_EQ(mapper.size(), order.size());
        ASSERT_EQ(mapper.size(), degree.size());
    }
    ASSERT_EQ(mapper.size(), size_t(10));

    Graph graph(10);

    for (const auto &e : edges){
        graph.insert(mapper.query(e.first), mapper.query(e.second));
    }

    Index index;
    index[mapper.query(2)];
    index[mapper.query(4)];
    index[mapper.query(7)];
    index[mapper.query(9)];
    index.insert(mapper.query(2), mapper.query(2));
    index.insert(mapper.query(4), mapper.query(4));
    index.insert(mapper.query(7), mapper.query(7));
    index.insert(mapper.query(9), mapper.query(9));
    index.insert(mapper.query(2), mapper.query(4));
    index.insert(mapper.query(2), mapper.query(7));
    index.insert(mapper.query(4), mapper.query(2));
    index.insert(mapper.query(4), mapper.query(7));
    index.insert(mapper.query(4), mapper.query(9));
    index.insert(mapper.query(7), mapper.query(9));

    WeightFunction weight;
    weight.define(mapper.query(2), mapper.query(2), 0);
    weight.define(mapper.query(4), mapper.query(4), 0);
    weight.define(mapper.query(7), mapper.query(7), 0);
    weight.define(mapper.query(9), mapper.query(9), 0);
    weight.define(mapper.query(2), mapper.query(4), 1);
    weight.define(mapper.query(2), mapper.query(7), 3);
    weight.define(mapper.query(4), mapper.query(2), 2);
    weight.define(mapper.query(4), mapper.query(7), 2);
    weight.define(mapper.query(4), mapper.query(9), 3);
    weight.define(mapper.query(7), mapper.query(9), 1);

    ASSERT_EQ(graph, kr.graph);
    ASSERT_EQ(index, kr.index);
    ASSERT_EQ(weight, kr.weight);
    ASSERT_EQ(mapper, kr.mapper);
}

TEST_F(KReachTest, QueryTest)
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

    /*Case 1*/
    ASSERT_TRUE(kr.query(2, 7));
    ASSERT_FALSE(kr.query(2, 9));

    /*Case 2*/
    ASSERT_TRUE(kr.query(4, 8));
    ASSERT_FALSE(kr.query(4, 10));

    /*Case 3*/
    ASSERT_TRUE(kr.query(1, 4));
    ASSERT_FALSE(kr.query(1, 7));

    /*Case 4*/
    ASSERT_TRUE(kr.query(3, 6));
    ASSERT_FALSE(kr.query(3, 8));

    /*Self reachable*/
    ASSERT_TRUE(kr.query(2, 2));
    ASSERT_TRUE(kr.query(3, 3));

    /*1-hop reachable*/
    ASSERT_TRUE(kr.query(1, 2));
    ASSERT_TRUE(kr.query(2, 4));
}