#pragma once

#include "IteratorBase.h"

class UnionIterator : public IteratorBase
{
public:

    UnionIterator(AdjacentIterator a, AdjacentIterator a_end, AdjacentIterator b, AdjacentIterator b_end);

    UnionIterator &operator++();

    UnionIterator operator++(int);

    reference operator*() const;
};