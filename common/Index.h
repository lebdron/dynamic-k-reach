#pragma once

#include "AdjacencyList.h"

class Index : public std::unordered_map<vertex_t, AdjacencyList>
{
public:
    void insert(vertex_t s, vertex_t t);

    void remove(vertex_t s, vertex_t t);
};