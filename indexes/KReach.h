#pragma once

#include <common.h>
#include <Graph.h>
#include <WeightFunction.h>
#include <Index.h>
#include <Mapper.h>

class KReach
{
    weight_t k;
    Graph graph;
    Index index;
    WeightFunction weight;
    Mapper mapper;

    void generate_cover();

    void bfs(vertex_t s);

    void clear();

public:
    void construct_index(std::vector<Edge> edges, weight_t k);

    bool query(vertex_t s, vertex_t t) const;
};
