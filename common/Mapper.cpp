#include "Mapper.h"

using std::make_pair;

Vertex Mapper::operator[](Vertex v) {
    Vertex id;
    if (set_.empty()){
        id = (Vertex) map_.size();
    }
    else{
        auto it = set_.begin();
        id = *it;
        set_.erase(it);
    }
    map_.insert(make_pair(v, id));
    return id;
}

Vertex Mapper::operator()(Vertex v) const {
    return map_.at(v);
}

void Mapper::remove(Vertex v) {
    auto it = map_.find(v);
    if (it == map_.end()){
        return;
    }
    Vertex id = it->second;
    set_.insert(id);
    map_.erase(it);
}

bool Mapper::empty() const {
    return map_.empty();
}

Mapper::size_type Mapper::size() const {
    return map_.size();
}

bool Mapper::present(Vertex v) const {
    return map_.find(v) != map_.end();
}

void Mapper::clear() {
    map_.clear();
    set_.clear();
}

bool Mapper::operator==(const Mapper &m) const {
    return map_ == m.map_ && set_ == m.set_;
}
