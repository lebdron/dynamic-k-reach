#pragma once

#include "DynamicKReachBase.h"

class DynamicKReachReindex : public DynamicKReachBase
{
    void clear_index();

public:
    void insert_edge(vertex_t s, vertex_t t);

    void remove_edge(vertex_t s, vertex_t t);

    void remove_vertex(vertex_t v);
};
