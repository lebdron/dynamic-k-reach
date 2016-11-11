#include "WeightFunction.h"

void WeightFunction::define(vertex_t s, vertex_t t, weight_t d)
{
    assert(!defined(s, t));

    insert(make_pair(Edge(s, t), d));
}

const weight_t &WeightFunction::operator()(vertex_t s, vertex_t t) const
{
    assert(defined(s, t));

    return at(Edge(s, t));
}

bool WeightFunction::defined(vertex_t s, vertex_t t) const
{
    return count(Edge(s, t)) != 0;
}

void WeightFunction::undefine(vertex_t s, vertex_t t)
{
    assert(defined(s, t));

    erase(Edge(s, t));

    assert(!defined(s, t));
}

void WeightFunction::update(vertex_t s, vertex_t t, weight_t d)
{
    assert(defined(s, t));

    operator[](Edge(s, t)) = d;
}
