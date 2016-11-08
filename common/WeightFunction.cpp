#include "WeightFunction.h"

weight_t &WeightFunction::operator()(vertex_t s, vertex_t t)
{
    return operator[](Edge(s, t));
}

bool WeightFunction::defined(vertex_t s, vertex_t t) const
{
    return std::unordered_map<Edge, weight_t, EdgeHasher, EdgeComparator>::count(Edge(s, t)) != 0;
}
