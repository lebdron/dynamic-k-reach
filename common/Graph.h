#pragma once

#include <unordered_map>
#include "AdjacencyList.h"
#include "Iterator.h"

struct AdjacencyListPair{
    using size_type = AdjacencyList::size_type;
    AdjacencyList out, in;
    Union begin() const;
    Union end() const;
    bool operator==(const AdjacencyListPair &p) const;
    size_type degree() const;
    void clear();
    bool empty() const;
};

struct AdjacencyListPairPosition{
    using const_iterator = AdjacencyList::const_iterator;
    const_iterator out, in;

    AdjacencyListPairPosition(const_iterator out, const_iterator in);
};

class Graph {
    std::vector<AdjacencyListPair> vector_;
public:
    using size_type = std::size_t;
    Graph() = default;
    Graph(size_type n);
    AdjacencyListPairPosition insert(Vertex s, Vertex t);
    AdjacencyListPairPosition remove(Vertex s, Vertex t);
    AdjacencyList& out(Vertex v);
    const AdjacencyList& out(Vertex v) const;
    AdjacencyList& in(Vertex v);
    const AdjacencyList& in(Vertex v) const;
    AdjacencyListPair& operator()(Vertex v);
    const AdjacencyListPair& operator()(Vertex v) const;
    void clear();
    void resize(size_type n);
    size_type size() const;
    bool operator==(const Graph &g) const;
};

class Index {
    using base = std::unordered_map<Vertex, AdjacencyListPair>;
    base map_;
public:
    using size_type = std::size_t;
    using iterator = base::iterator;
    using const_iterator = base::const_iterator;
    Index() = default;
    AdjacencyListPairPosition insert(Vertex s, Vertex t);
    void operator[](Vertex v);
    AdjacencyListPairPosition remove(Vertex s, Vertex t);
    AdjacencyList& out(Vertex v);
    const AdjacencyList& out(Vertex v) const;
    AdjacencyList& in(Vertex v);
    const AdjacencyList& in(Vertex v) const;
    AdjacencyListPair& operator()(Vertex v);
    const AdjacencyListPair& operator()(Vertex v) const;
    bool contains(Vertex v) const;
    iterator begin();
    const_iterator begin() const;
    iterator end();
    const_iterator end() const;
    void clear();
    size_type size() const;
    bool operator==(const Index &i) const;
    void remove(Vertex v);
};