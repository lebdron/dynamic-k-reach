#pragma once

#include <common.h>
#include <Graph.h>
#include <WeightFunction.h>
#include <Index.h>
#include <Mapper.h>
#include <gtest/gtest_prod.h>

class KReach
{
protected:
    weight_t k;
    Graph graph;
    Index index;
    WeightFunction weight;
    Mapper mapper;

    void generate_cover();

    void generate_index();

    void bfs(vertex_t s);

    void clear();

    FRIEND_TEST(KReachTest, ReferenceIndexTest);

    FRIEND_TEST(DynamicKReachReindexTest, ReferenceInsertTest);

    FRIEND_TEST(DynamicKReachTest, InsertTest);

    FRIEND_TEST(DynamicKReachTest, RemoveTest);

    FRIEND_TEST(DynamicKReachTest, RemovalStressTest);

public:
    void construct_index(std::vector<Edge> edges, weight_t k);

    bool query(vertex_t s, vertex_t t) const;
};
