#pragma once

#include <unordered_map>
#include "common.h"

class WeightFunction {
    using base = std::unordered_map<Edge, Weight, EdgeHash>;
    base map_;
    using iterator = base::iterator;
    using const_iterator = base::const_iterator;
public:
    Weight& operator()(Vertex s, Vertex t);
    const Weight& operator()(Vertex s, Vertex t) const;
    iterator find(Vertex s, Vertex t);
    const_iterator find(Vertex s, Vertex t) const;
    iterator undefine(iterator it);
    bool defined(iterator it) const;
    bool defined(const_iterator it) const;
    void clear();
    bool operator==(const WeightFunction &w) const;
};