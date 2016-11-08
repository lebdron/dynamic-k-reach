#include "Graph.h"

Graph::Graph() : vector()
{

}

Graph::Graph(uint32_t n) : vector(n)
{

}

void Graph::insert(vertex_t s, vertex_t t)
{
    assert(!at(s).out.count(t));
    assert(!at(t).in.count(s));

    at(s).out.insert(t);
    at(t).in.insert(s);

    assert(at(s).out.count(t));
    assert(at(t).in.count(s));
}

void Graph::remove(vertex_t s, vertex_t t)
{
    assert(at(s).out.count(t));
    assert(at(t).in.count(s));

    at(s).out.erase(t);
    at(t).in.erase(s);

    assert(!at(s).out.count(t));
    assert(!at(t).in.count(s));
}