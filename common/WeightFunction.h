#pragma once

#include "common.h"

class WeightFunction : public std::unordered_map<Edge, weight_t, EdgeHash>
{
public:
    weight_t &operator()(vertex_t s, vertex_t t);

    const weight_t &operator()(vertex_t s, vertex_t t) const;

    bool defined(vertex_t s, vertex_t t) const;

    void undefine(vertex_t s, vertex_t t);
};