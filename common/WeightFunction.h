#pragma once

#include "common.h"

struct EdgeHasher
{
    std::size_t operator()(const Edge &e) const
    {
        return (std::hash<vertex_t>{}(e.first) ^ std::hash<vertex_t>{}(e.second)) + (e.first == e.second ? e.first : 0);
    }
};

struct EdgeComparator
{
    bool operator()(const Edge &e1, const Edge &e2) const
    {
        return (e1.first == e2.first && e1.second == e2.second) || (e1.first == e2.second && e1.second == e2.first);
    }
};

class WeightFunction : public std::unordered_map<Edge, weight_t, EdgeHasher, EdgeComparator>
{
public:
    weight_t &operator()(vertex_t s, vertex_t t);

    bool defined(vertex_t s, vertex_t t) const;
};