#pragma once

#include "KReach.h"

class DynamicKReach : public KReach {
protected:
    std::unordered_set<Edge, EdgeHash> identified, processing;
    void insert_update(Vertex s, Vertex t, Weight d);
    void remove_identify(Vertex s, Vertex t, Weight d);
    void remove_update();
public:
    DynamicKReach() = default;
    DynamicKReach(const DynamicKReach& i);
    DynamicKReach(const KReach& i);
    DynamicKReach& operator=(DynamicKReach i);
    DynamicKReach& operator=(KReach i);
    void insert_edge(Vertex s, Vertex t);
    void remove_edge(Vertex s, Vertex t);
    void remove_vertex(Vertex v);
    void remove_vertex_edges(Vertex v);
};