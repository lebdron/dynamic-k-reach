#include "IntersectionIterator.h"

void IntersectionIterator::next()
{
    while (a != a_end && b != b_end && *a != *b){
        if (*a < *b){
            ++a;
        }
        else {
            ++b;
        }
    }
    if (a == a_end || b == b_end){
        a = a_end;
        b = b_end;
    }
}

IntersectionIterator::IntersectionIterator(AdjacentIterator a, AdjacentIterator a_end,
                                           AdjacentIterator b, AdjacentIterator b_end)
        : IteratorBase(a, a_end, b, b_end)
{
    next();
}

IntersectionIterator &IntersectionIterator::operator++()
{
    ++a;
    ++b;
    next();
    return *this;
}

IntersectionIterator IntersectionIterator::operator++(int)
{
    IntersectionIterator tmp(*this);
    ++(*this);
    return tmp;
}

const vertex_t &IntersectionIterator::operator*() const
{
    return *a;
}
