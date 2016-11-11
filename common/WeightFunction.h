#pragma once

#include "common.h"

class WeightFunction : public std::unordered_map<Edge, weight_t, EdgeHash>
{
public:
    void define(vertex_t s, vertex_t t, weight_t d);

    const weight_t &operator()(vertex_t s, vertex_t t) const;

    bool defined(vertex_t s, vertex_t t) const;

    void undefine(vertex_t s, vertex_t t);

    void update(vertex_t s, vertex_t t, weight_t d);
};