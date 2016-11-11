#include "DynamicKReachBase.h"

void DynamicKReachBase::insert_edge(vertex_t s, vertex_t t)
{
    if (s == t){
        return;
    }

    if (!mapper.present(s) && mapper.insert(s) >= graph.size()){
        graph.resize(mapper.size());
    }
    if (!mapper.present(t) && mapper.insert(t) >= graph.size()){
        graph.resize(mapper.size());
    }

    assert(mapper.present(s) && mapper.present(t));

    s = mapper.query(s);
    t = mapper.query(t);

    assert(s <= graph.size() && t <= graph.size());

    if (graph[s].out.count(t)){
        return;
    }

    graph.insert(s, t);
}

void DynamicKReachBase::remove_edge(vertex_t s, vertex_t t)
{
    if (s == t || !mapper.present(s) || !mapper.present(t)){
        return;
    }

    s = mapper.query(s);
    t = mapper.query(t);

    if (!graph[s].out.count(t)){
        return;
    }

    graph.remove(s, t);
}

void DynamicKReachBase::remove_vertex(vertex_t v)
{
    if (!mapper.present(v)){
        return;
    }

    v = mapper.query(v);

    for (const auto &p : graph.at(v).in){
        graph.at(p).out.erase(v);
    }
    for (const auto &q : graph.at(v).out){
        graph.at(q).in.erase(v);
    }
}

DynamicKReachBase::~DynamicKReachBase()
{

}
