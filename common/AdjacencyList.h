#pragma once

#include "common.h"
#include "UnionIterator.h"

class AdjacencyList
{
public:
    typedef UnionIterator Iterator;
    typedef UnionIterator ConstIterator;

    Adjacent out, in;

    Iterator begin();

    Iterator end();

    ConstIterator begin() const;

    ConstIterator end() const;
};
