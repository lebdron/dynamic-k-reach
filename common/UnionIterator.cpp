#include "UnionIterator.h"

UnionIterator::UnionIterator(AdjacentIterator a, AdjacentIterator a_end,
                             AdjacentIterator b, AdjacentIterator b_end)
        : IteratorBase(a, a_end, b, b_end)
{

}

UnionIterator &UnionIterator::operator++()
{
    if (a != a_end && b != b_end){
        if (*a == *b){
            ++a;
            ++b;
        }
        else if (*a < *b){
            ++a;
        }
        else {
            ++b;
        }
    }
    else if (a != a_end){
        ++a;
    }
    else if (b != b_end) {
        ++b;
    }
    return *this;
}

UnionIterator UnionIterator::operator++(int)
{
    UnionIterator tmp(*this);
    ++(*this);
    return tmp;
}

const vertex_t &UnionIterator::operator*() const
{

    if (a != a_end && b != b_end){
        return *a <= *b ? *a : *b;
    }
    else if (a != a_end){
        return *a;
    }
    else {
        return *b;
    }
}
