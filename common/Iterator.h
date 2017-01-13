#pragma once

#include <iterator>
#include "common.h"
#include "AdjacencyList.h"

using VertexIterator = std::iterator<std::input_iterator_tag,
        Vertex, std::ptrdiff_t, const Vertex*, const Vertex&>;
using AdjacentIterator = AdjacencyList::const_iterator;

class Union : public VertexIterator{
    AdjacentIterator cur1, end1, cur2, end2;
public:
    explicit Union(AdjacentIterator cur1, AdjacentIterator end1,
                   AdjacentIterator cur2, AdjacentIterator end2);
    Union& operator++();
    Union operator++(int);
    bool operator==(Union it) const;
    bool operator!=(Union it) const;
    reference operator*() const;
};

class InclusiveIntersection : public VertexIterator{
    AdjacentIterator cur1, end1, cur2, end2;
    void next();
public:
    explicit InclusiveIntersection(AdjacentIterator cur1, AdjacentIterator end1,
                                   AdjacentIterator cur2, AdjacentIterator end2);
    InclusiveIntersection& operator++();
    InclusiveIntersection operator++(int);
    bool operator==(InclusiveIntersection it) const;
    bool operator!=(InclusiveIntersection it) const;
    reference operator*() const;
};

class ExclusiveIntersection : public VertexIterator{
    AdjacentIterator cur1, end1, cur2, end2;
    Vertex v1, v2;
    void next();
public:
    explicit ExclusiveIntersection(Vertex v1, AdjacentIterator cur1, AdjacentIterator end1,
            Vertex v2, AdjacentIterator cur2, AdjacentIterator end2);
    ExclusiveIntersection& operator++();
    ExclusiveIntersection operator++(int);
    bool operator==(ExclusiveIntersection it) const;
    bool operator!=(ExclusiveIntersection it) const;
    reference operator*() const;
    AdjacentIterator first() const;
    AdjacentIterator second() const;
};