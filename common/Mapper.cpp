#include "Mapper.h"

vertex_t Mapper::insert(vertex_t v)
{
    assert(!mapping.count(v));

    vertex_t id;
    if (free.empty()){
        id = mapping.size();
    }
    else {
        auto it = free.begin();
        id = *it;
        free.erase(it);
    }
    mapping[v] = id;

    assert(mapping.count(v));
    assert(!free.count(id));

    return id;
}

bool Mapper::present(vertex_t v) const
{
    return mapping.count(v) != 0;
}

vertex_t Mapper::query(vertex_t v) const
{
    assert(mapping.count(v));

    return mapping.at(v);
}

void Mapper::remove(vertex_t v)
{
    assert(mapping.count(v));

    auto it = mapping.find(v);
    vertex_t id = it->second;
    free.insert(id);
    mapping.erase(it);

    assert(!mapping.count(v));
    assert(free.count(id));
}

void Mapper::clear()
{
    mapping.clear();
    free.clear();
}

size_t Mapper::size() const {
    return mapping.size() + free.size();
}

bool Mapper::operator==(const Mapper &mapper) const {
    return mapping == mapper.mapping && free == mapper.free;
}

