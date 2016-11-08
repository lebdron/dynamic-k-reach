#pragma once

#include "IteratorBase.h"

class IntersectionIterator : public IteratorBase
{
    void next();

public:

    IntersectionIterator(AdjacentIterator a, AdjacentIterator a_end, AdjacentIterator b, AdjacentIterator b_end);

    IntersectionIterator &operator++();

    IntersectionIterator operator++(int);

    reference operator*() const;
};