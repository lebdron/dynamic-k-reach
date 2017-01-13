#include "WeightFunction.h"

Weight &WeightFunction::operator()(Vertex s, Vertex t) {
    return map_[Edge(s, t)];
}

WeightFunction::const_iterator WeightFunction::find(Vertex s, Vertex t) const{
    return map_.find(Edge(s, t));
}

WeightFunction::iterator WeightFunction::undefine(WeightFunction::iterator it) {
    return map_.erase(it);
}

bool WeightFunction::defined(WeightFunction::iterator it) const {
    return it != map_.end();
}

void WeightFunction::clear() {
    map_.clear();
}

bool WeightFunction::defined(WeightFunction::const_iterator it) const {
    return it != map_.end();
}

const Weight &WeightFunction::operator()(Vertex s, Vertex t) const {
    return map_.at(Edge(s, t));
}

WeightFunction::iterator WeightFunction::find(Vertex s, Vertex t) {
    return map_.find(Edge(s, t));
}

bool WeightFunction::operator==(const WeightFunction &w) const {
    return map_ == w.map_;
}
