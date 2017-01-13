#pragma once

#include <common.h>
#include <Graph.h>
#include <WeightFunction.h>
#include <Mapper.h>

class KReach {
protected:
    Weight k;
    Graph graph;
    Index index;
    WeightFunction weight;
    Mapper mapper;

    void construct_cover();
    void construct_index();
    void bfs(Vertex s);
    void clear();
    void clear_index();

public:
    KReach() = default;
    KReach(const KReach& i);
    KReach& operator=(KReach i);
    void construct_index(std::vector<Edge> edges, Weight k);
    bool query(Vertex s, Vertex t) const;
    void insert_edge(Vertex s, Vertex t);
    void insert_vertex(Vertex v);
    void remove_edge(Vertex s, Vertex t);
    void remove_vertex(Vertex v);
    friend void TEST_equals(const KReach& i1, const KReach& i2);
};
