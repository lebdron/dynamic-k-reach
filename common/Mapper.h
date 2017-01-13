#pragma once

#include <unordered_map>
#include <unordered_set>
#include "common.h"

class Mapper{
    std::unordered_map<Vertex, Vertex> map_;
    std::unordered_set<Vertex> set_;
public:
    using size_type = std::size_t;
    Vertex operator[](Vertex v);
    Vertex operator()(Vertex v) const;
    void remove(Vertex v);
    bool empty() const;
    size_type size() const;
    bool present(Vertex v) const;
    bool operator==(const Mapper &m) const;
    void clear();
};