#include <gtest/gtest.h>
#include <DynamicKReach.h>
#include <fstream>
#include <DynamicKReachReindex.h>
#include <IntersectionIterator.h>
#include <sys/time.h>

class DynamicKReachTest : public ::testing::Test
{

};

double GetCurrentTimeSec()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec * 1e-6;
}

TEST_F(DynamicKReachTest, InsertTest)
{
    std::vector<Edge> edges;
    {
        std::ifstream fin("data/sample");
        ASSERT_TRUE(fin.is_open());
        for (vertex_t s, t; fin >> s >> t;) {
            edges.push_back(Edge(s, t));
        }
    }

    DynamicKReach dkr;
    dkr.construct_index(edges, 3);

    DynamicKReachReindex dkrr;
    dkrr.construct_index(edges, 3);

    dkr.insert_edge(3, 8);
    dkrr.insert_edge(3, 8);

    ASSERT_EQ(dkr.graph, dkrr.graph);
    ASSERT_EQ(dkr.index[dkr.mapper.query(2)].out, dkrr.index[dkrr.mapper.query(2)].out);
    ASSERT_EQ(dkr.index[dkrr.mapper.query(2)].in, dkrr.index[dkrr.mapper.query(2)].in);

    ASSERT_EQ(dkr.index[dkr.mapper.query(4)].out, dkrr.index[dkrr.mapper.query(4)].out);
    ASSERT_EQ(dkr.index[dkrr.mapper.query(4)].in, dkrr.index[dkrr.mapper.query(4)].in);

    ASSERT_EQ(dkr.index[dkr.mapper.query(7)].out, dkrr.index[dkrr.mapper.query(7)].out);
    ASSERT_EQ(dkr.index[dkrr.mapper.query(7)].in, dkrr.index[dkrr.mapper.query(7)].in);

    ASSERT_EQ(dkr.index[dkr.mapper.query(9)].out, dkrr.index[dkrr.mapper.query(9)].out);
    ASSERT_EQ(dkr.index[dkrr.mapper.query(9)].in, dkrr.index[dkrr.mapper.query(9)].in);

    ASSERT_EQ(dkr.index[dkr.mapper.query(3)].out, dkrr.index[dkrr.mapper.query(3)].out);
    ASSERT_EQ(dkr.index[dkrr.mapper.query(3)].in, dkrr.index[dkrr.mapper.query(3)].in);
    ASSERT_EQ(dkr.weight, dkrr.weight);
    ASSERT_EQ(dkr.mapper, dkrr.mapper);

    dkr.insert_edge(5, 9);
    dkrr.insert_edge(5, 9);

    ASSERT_EQ(dkr.graph, dkrr.graph);
    ASSERT_EQ(dkr.index[dkr.mapper.query(2)].out, dkrr.index[dkrr.mapper.query(2)].out);
    ASSERT_EQ(dkr.index[dkrr.mapper.query(2)].in, dkrr.index[dkrr.mapper.query(2)].in);

    ASSERT_EQ(dkr.index[dkr.mapper.query(4)].out, dkrr.index[dkrr.mapper.query(4)].out);
    ASSERT_EQ(dkr.index[dkrr.mapper.query(4)].in, dkrr.index[dkrr.mapper.query(4)].in);

    ASSERT_EQ(dkr.index[dkr.mapper.query(7)].out, dkrr.index[dkrr.mapper.query(7)].out);
    ASSERT_EQ(dkr.index[dkrr.mapper.query(7)].in, dkrr.index[dkrr.mapper.query(7)].in);

    ASSERT_EQ(dkr.index[dkr.mapper.query(9)].out, dkrr.index[dkrr.mapper.query(9)].out);
    ASSERT_EQ(dkr.index[dkrr.mapper.query(9)].in, dkrr.index[dkrr.mapper.query(9)].in);

    ASSERT_EQ(dkr.index[dkr.mapper.query(3)].out, dkrr.index[dkrr.mapper.query(3)].out);
    ASSERT_EQ(dkr.index[dkrr.mapper.query(3)].in, dkrr.index[dkrr.mapper.query(3)].in);
    ASSERT_EQ(dkr.weight, dkrr.weight);
    ASSERT_EQ(dkr.mapper, dkrr.mapper);
}

TEST_F(DynamicKReachTest, RemoveTest)
{
    std::vector<Edge> edges;
    {
        std::ifstream fin("data/sample");
        ASSERT_TRUE(fin.is_open());
        for (vertex_t s, t; fin >> s >> t;) {
            edges.push_back(Edge(s, t));
        }
    }

    DynamicKReach dkr;
    dkr.construct_index(edges, 3);

    DynamicKReachReindex dkrr;
    dkrr.construct_index(edges, 3);

    dkr.remove_edge(5, 7);
    dkrr.remove_edge(5, 7);

    ASSERT_EQ(dkr.graph, dkrr.graph);
    ASSERT_EQ(dkr.index[dkr.mapper.query(2)].out, dkrr.index[dkrr.mapper.query(2)].out);
    ASSERT_EQ(dkr.index[dkrr.mapper.query(2)].in, dkrr.index[dkrr.mapper.query(2)].in);

    ASSERT_EQ(dkr.index[dkr.mapper.query(4)].out, dkrr.index[dkrr.mapper.query(4)].out);
    ASSERT_EQ(dkr.index[dkrr.mapper.query(4)].in, dkrr.index[dkrr.mapper.query(4)].in);

    ASSERT_EQ(dkr.index[dkr.mapper.query(7)].out, dkrr.index[dkrr.mapper.query(7)].out);
    ASSERT_EQ(dkr.index[dkrr.mapper.query(7)].in, dkrr.index[dkrr.mapper.query(7)].in);

    ASSERT_EQ(dkr.index[dkr.mapper.query(9)].out, dkrr.index[dkrr.mapper.query(9)].out);
    ASSERT_EQ(dkr.index[dkrr.mapper.query(9)].in, dkrr.index[dkrr.mapper.query(9)].in);

    ASSERT_EQ(dkr.index[dkr.mapper.query(3)].out, dkrr.index[dkrr.mapper.query(3)].out);
    ASSERT_EQ(dkr.index[dkrr.mapper.query(3)].in, dkrr.index[dkrr.mapper.query(3)].in);
    ASSERT_EQ(dkr.weight, dkrr.weight);
    ASSERT_EQ(dkr.mapper, dkrr.mapper);
}

TEST_F(DynamicKReachTest, RemovalBadCasesTest)
{
    /*a*/
    std::vector<Edge> edges = {Edge(1, 2), Edge(1, 3), Edge(2, 4), Edge(3, 4)};
    DynamicKReach dkr;
    dkr.construct_index(edges, 3);

    ASSERT_TRUE(dkr.query(1, 4));

    dkr.remove_edge(3, 4);

    ASSERT_TRUE(dkr.query(1, 4));

    /*b*/
    edges = {Edge(1, 2), Edge(2, 1), Edge(1, 3)};
    dkr.construct_index(edges, 3);

    ASSERT_TRUE(dkr.query(1, 3));
    ASSERT_TRUE(dkr.query(2, 3));

    dkr.remove_edge(1, 3);

    ASSERT_FALSE(dkr.query(1, 3));
    ASSERT_FALSE(dkr.query(2, 3));

    /*c*/
    edges = {Edge(1, 2), Edge(2, 1), Edge(2, 3), Edge(3, 2), Edge(3, 4), Edge(4, 3), Edge(1, 4), Edge(4, 1)};
    dkr.construct_index(edges, 3);

    ASSERT_TRUE(dkr.query(1, 4));

    dkr.remove_edge(1, 4);

    ASSERT_TRUE(dkr.query(1, 4));
    ASSERT_TRUE(dkr.query(1, 3));
    ASSERT_TRUE(dkr.query(2, 4));
}

TEST_F(DynamicKReachTest, RemovalStressTest)
{
    std::vector<Edge> edges;
    {
        std::ifstream fin("data/lastfm");
        ASSERT_TRUE(fin.is_open());
        for (vertex_t s, t; fin >> s >> t;) {
            edges.push_back(Edge(s, t));
        }
    }

    DynamicKReachReindex dkrr;
    dkrr.construct_index(edges, 3);
    DynamicKReach dkr;
    dkr.construct_index(edges, 3);

    auto rand_edge = std::bind(std::uniform_int_distribution<size_t>(0, edges.size() - 1),
                               std::default_random_engine());

    std::cout << std::endl;
    for (int i = 0; i < 100; ++i){
        std::cout << "Iteration: " << i << std::endl;
        size_t e = rand_edge();
        std::cout << "Random edge: " << edges[e].first << " " << edges[e].second << std::endl;

        dkr.remove_edge(edges[e].first, edges[e].second);
        dkrr.remove_edge(edges[e].first, edges[e].second);

        ASSERT_EQ(dkr.mapper, dkrr.mapper);
        ASSERT_EQ(dkr.graph, dkrr.graph);
        for (const auto &v : dkr.index){
//            std::cout << "Indexed vertex: " << v.first << std::endl;
            ASSERT_EQ(dkr.index.at(v.first).out, dkrr.index.at(v.first).out);
            ASSERT_EQ(dkr.index.at(v.first).in, dkrr.index.at(v.first).in);
        }
        ASSERT_EQ(dkr.weight, dkrr.weight);
    }
}

TEST_F(DynamicKReachTest, VertexRemovalStressTest)
{
    std::vector<Edge> edges;
    std::unordered_set<vertex_t> vertices;
    {
        std::ifstream fin("data/lastfm");
        ASSERT_TRUE(fin.is_open());
        for (vertex_t s, t; fin >> s >> t;) {
            edges.push_back(Edge(s, t));
            vertices.insert({s, t});
        }
    }

    DynamicKReachReindex dkrr;
    dkrr.construct_index(edges, 3);
    DynamicKReach dkr;
    dkr.construct_index(edges, 3);

    auto rand_vertex = std::bind(std::uniform_int_distribution<size_t>(0, vertices.size() - 1),
                               std::default_random_engine());

    std::cout << std::endl;
    for (int i = 0; i < 100; ++i){
        std::cout << "Iteration: " << i << std::endl;
        size_t vtx = rand_vertex();

        auto it = vertices.begin();
        std::advance(it, vtx);

        std::cout << "Random vertex: " << *it << std::endl;

        dkr.remove_vertex(*it);
        dkrr.remove_vertex(*it);

        ASSERT_EQ(dkr.mapper, dkrr.mapper);
        ASSERT_EQ(dkr.graph, dkrr.graph);
        ASSERT_EQ(dkr.index, dkrr.index);
        for (const auto &v : dkr.index){
//            std::cout << "Indexed vertex: " << v.first << std::endl;
            ASSERT_EQ(dkr.index.at(v.first).out, dkrr.index.at(v.first).out);
            ASSERT_EQ(dkr.index.at(v.first).in, dkrr.index.at(v.first).in);
        }
        ASSERT_EQ(dkr.weight, dkrr.weight);
    }
}

TEST_F(DynamicKReachTest, InsertStressTest)
{
    std::vector<Edge> edges;
    std::unordered_set<vertex_t> vertices;
    {
        std::ifstream fin("data/lastfm");
        ASSERT_TRUE(fin.is_open());
        for (vertex_t s, t; fin >> s >> t;) {
            edges.push_back(Edge(s, t));
            vertices.insert({s, t});
        }
    }

    DynamicKReachReindex dkrr;
    dkrr.construct_index(edges, 3);
    DynamicKReach dkr;
    dkr.construct_index(edges, 3);

    auto rand_vertex = std::bind(std::uniform_int_distribution<size_t>(0, vertices.size() - 1),
                                 std::default_random_engine());

    std::cout << std::endl;
    for (int i = 0; i < 100; ++i){
        std::cout << "Iteration: " << i << std::endl;
        size_t vtx = rand_vertex();
        size_t vtx2 = rand_vertex();

        auto it = vertices.begin();
        std::advance(it, vtx);

        auto it2 = vertices.begin();
        std::advance(it, vtx2);

        std::cout << "Random pair of vertices: " << *it << " " << *it2 << std::endl;

        dkr.insert_edge(*it ,*it2);
        dkrr.insert_edge(*it, *it2);

        ASSERT_EQ(dkr.mapper, dkrr.mapper);
        ASSERT_EQ(dkr.graph, dkrr.graph);
        ASSERT_EQ(dkr.index, dkrr.index);
        for (const auto &v : dkr.index){
//            std::cout << "Indexed vertex: " << v.first << std::endl;
            ASSERT_EQ(dkr.index.at(v.first).out, dkrr.index.at(v.first).out);
            ASSERT_EQ(dkr.index.at(v.first).in, dkrr.index.at(v.first).in);
        }
        ASSERT_EQ(dkr.weight, dkrr.weight);
    }
}