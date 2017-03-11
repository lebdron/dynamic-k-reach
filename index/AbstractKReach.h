#pragma once

#include "common.h"

class AbstractKReach {
public:
    virtual void construct() = 0;

    virtual bool query(vertex_t s, vertex_t t) const = 0;

    virtual void insert_edge(vertex_t s, vertex_t t) = 0;

    virtual void remove_edge(vertex_t s, vertex_t t) = 0;

    virtual void remove_vertex(vertex_t v, const std::vector<vertex_t> &out, const std::vector<vertex_t> &in) = 0;

    virtual void insert_vertex(vertex_t v, const std::vector<vertex_t> &out, const std::vector<vertex_t> &in) = 0;
};