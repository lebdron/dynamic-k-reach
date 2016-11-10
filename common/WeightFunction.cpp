#include "WeightFunction.h"

weight_t &WeightFunction::operator()(vertex_t s, vertex_t t)
{
    return operator[](Edge(s, t));
}

const weight_t &WeightFunction::operator()(vertex_t s, vertex_t t) const
{
    assert(defined(s, t));

    return at(Edge(s, t));
}

bool WeightFunction::defined(vertex_t s, vertex_t t) const
{
    return std::unordered_map<Edge, weight_t, EdgeHash>::count(Edge(s, t)) != 0;
}

void WeightFunction::undefine(vertex_t s, vertex_t t)
{
    assert(defined(s, t));

    erase(Edge(s, t));

    assert(!defined(s, t));
}
