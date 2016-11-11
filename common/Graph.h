#pragma once

#include "AdjacencyList.h"

class Graph : public std::vector<AdjacencyList>
{
public:
    Graph();

    Graph(uint32_t n);

    void insert(vertex_t s, vertex_t t);

    void remove(vertex_t s, vertex_t t);

    bool has(vertex_t s, vertex_t t) const;
};