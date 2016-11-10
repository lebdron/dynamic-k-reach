#pragma once

#include "common.h"

class Mapper
{
    std::unordered_map<vertex_t, vertex_t> mapping;
    std::unordered_set<vertex_t> free;

public:
    vertex_t insert(vertex_t v);

    bool present(vertex_t v) const;

    vertex_t query(vertex_t v) const;

    void remove(vertex_t v);

    void clear();

    size_t size() const;

    bool operator==(const Mapper &mapper) const;
};