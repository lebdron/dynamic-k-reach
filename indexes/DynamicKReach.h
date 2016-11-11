#pragma once

#include "DynamicKReachBase.h"

class DynamicKReach : public DynamicKReachBase
{
    void insert_update(vertex_t s, vertex_t t, weight_t d);

    void remove_identify(vertex_t s, vertex_t t, weight_t d, std::unordered_set<Edge, EdgeHash> &identified);

    void remove_update(vertex_t s, vertex_t t, std::unordered_set<Edge, EdgeHash> &identified);

public:
    void insert_edge(vertex_t s, vertex_t t);

    void remove_edge(vertex_t s, vertex_t t);

    void remove_vertex(vertex_t v);
};

