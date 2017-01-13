#pragma once

#include "common.h"

class AdjacencyList {
    using base = std::set<Vertex>;
    base set_;
public:
    using iterator = base::iterator;
    using const_iterator = base::const_iterator;
    using size_type = std::size_t;
    iterator insert(Vertex v);
    iterator remove(Vertex v);
    const_iterator begin() const;
    const_iterator end() const;
    size_type size() const;
    void clear();
    bool operator==(const AdjacencyList &l) const;
    bool empty() const;
    bool contains(Vertex v) const;
};
