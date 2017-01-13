#include "Graph.h"

Union AdjacencyListPair::begin() const {
    return Union(out.begin(), out.end(), in.begin(), in.end());
}

Union AdjacencyListPair::end() const {
    return Union(out.end(), out.end(), in.end(), in.end());
}

bool AdjacencyListPair::operator==(const AdjacencyListPair &p) const {
    return out == p.out && in == p.in;
}

AdjacencyListPair::size_type AdjacencyListPair::degree() const {
    return out.size() + in.size();
}

void AdjacencyListPair::clear() {
    out.clear();
    in.clear();
}

bool AdjacencyListPair::empty() const {
    return out.empty() && in.empty();
}

AdjacencyListPairPosition::AdjacencyListPairPosition(AdjacencyListPairPosition::const_iterator out,
                                                     AdjacencyListPairPosition::const_iterator in)
        : out(out), in(in){

}

Graph::Graph(Graph::size_type n)
        : vector_(n){

}

AdjacencyListPairPosition Graph::insert(Vertex s, Vertex t) {
    auto out = vector_.at(s).out.insert(t), in = vector_.at(t).in.insert(s);
    return AdjacencyListPairPosition(out, in);
}

AdjacencyListPairPosition Graph::remove(Vertex s, Vertex t) {
    auto &sout = vector_.at(s).out, &tin = vector_.at(t).in;
    auto out = sout.remove(t), in = tin.remove(s);
    return AdjacencyListPairPosition(out, in);
}

AdjacencyList &Graph::out(Vertex v) {
    return vector_.at(v).out;
}

const AdjacencyList &Graph::out(Vertex v) const {
    return vector_.at(v).out;
}

AdjacencyList &Graph::in(Vertex v) {
    return vector_.at(v).in;
}

const AdjacencyList &Graph::in(Vertex v) const {
    return vector_.at(v).in;
}

AdjacencyListPair &Graph::operator()(Vertex v) {
    return vector_.at(v);
}

const AdjacencyListPair &Graph::operator()(Vertex v) const {
    return vector_.at(v);
}

void Graph::clear() {
    vector_.clear();
}

void Graph::resize(Graph::size_type n) {
    vector_.resize(n);
}

Graph::size_type Graph::size() const {
    return vector_.size();
}

bool Graph::operator==(const Graph &g) const {
    return vector_ == g.vector_;
}

AdjacencyListPairPosition Index::insert(Vertex s, Vertex t) {
    auto out = map_.at(s).out.insert(t), in = map_.at(t).in.insert(s);
    return AdjacencyListPairPosition(out, in);
}

AdjacencyListPairPosition Index::remove(Vertex s, Vertex t) {
    auto &sout = map_.at(s).out, &tin = map_.at(t).in;
    auto out = sout.remove(t), in = tin.remove(s);
    return AdjacencyListPairPosition(out, in);
}

AdjacencyList &Index::out(Vertex v) {
    return map_.at(v).out;
}

const AdjacencyList &Index::out(Vertex v) const {
    return map_.at(v).out;
}

AdjacencyList &Index::in(Vertex v) {
    return map_.at(v).in;
}

const AdjacencyList &Index::in(Vertex v) const {
    return map_.at(v).in;
}

AdjacencyListPair &Index::operator()(Vertex v) {
    return map_.at(v);
}

const AdjacencyListPair &Index::operator()(Vertex v) const {
    return map_.at(v);
}

bool Index::contains(Vertex v) const {
    return map_.find(v) != map_.end();
}

Index::const_iterator Index::begin() const {
    return map_.begin();
}

Index::const_iterator Index::end() const {
    return map_.end();
}

void Index::clear() {
    map_.clear();
}

void Index::operator[](Vertex v) {
    map_[v];
}

Index::iterator Index::begin() {
    return map_.begin();
}

Index::iterator Index::end() {
    return map_.end();
}

bool Index::operator==(const Index &i) const {
    return map_ == i.map_;
}

void Index::remove(Vertex v) {
    map_.erase(v);
}

Index::size_type Index::size() const {
    return map_.size();
}
