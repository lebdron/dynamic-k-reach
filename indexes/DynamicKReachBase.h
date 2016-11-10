#pragma once

#include "KReach.h"

class DynamicKReachBase : public KReach
{
public:
    virtual void insert_edge(vertex_t s, vertex_t t);

    virtual void remove_edge(vertex_t s, vertex_t t);

    virtual void remove_vertex(vertex_t v);

    virtual ~DynamicKReachBase() = 0;
};
