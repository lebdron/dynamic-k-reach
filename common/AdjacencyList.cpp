#include "AdjacencyList.h"

AdjacencyList::iterator AdjacencyList::insert(Vertex v) {
    return set_.insert(v).first;
}

AdjacencyList::iterator AdjacencyList::remove(Vertex v) {
    auto it = set_.find(v);
    if (it == set_.end()){
        return it;
    }
    return set_.erase(it);
}

AdjacencyList::size_type AdjacencyList::size() const {
    return set_.size();
}

void AdjacencyList::clear() {
    set_.clear();
}

AdjacencyList::const_iterator AdjacencyList::begin() const {
    return set_.begin();
}

AdjacencyList::const_iterator AdjacencyList::end() const {
    return set_.end();
}

bool AdjacencyList::operator==(const AdjacencyList &l) const {
    return set_ == l.set_;
}

bool AdjacencyList::empty() const {
    return set_.empty();
}

bool AdjacencyList::contains(Vertex v) const {
    return set_.find(v) != set_.end();
}
