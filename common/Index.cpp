#include "Index.h"

void Index::insert(vertex_t s, vertex_t t)
{
    assert(!at(s).out.count(t));
    assert(!at(t).in.count(s));

    at(s).out.insert(t);
    at(t).in.insert(s);

    assert(at(s).out.count(t));
    assert(at(t).in.count(s));
}

void Index::remove(vertex_t s, vertex_t t)
{
    assert(at(s).out.count(t));
    assert(at(t).in.count(s));

    at(s).out.erase(t);
    at(t).in.erase(s);

    assert(!at(s).out.count(t));
    assert(!at(t).in.count(s));
}

bool Index::has(vertex_t s, vertex_t t) const
{
    return at(s).out.count(t) != 0;
}
