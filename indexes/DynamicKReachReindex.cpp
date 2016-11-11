#include "DynamicKReachReindex.h"

using namespace std;

void DynamicKReachReindex::insert_edge(vertex_t s, vertex_t t)
{
    DynamicKReachBase::insert_edge(s, t);
    s = mapper.query(s);
    t = mapper.query(t);
    if (!index.count(s) && !index.count(t)){
        vertex_t v = (graph[s].degree() > graph[t].degree()) ? s : t;
        index[v];
    }
    clear_index();
    generate_index();
}

void DynamicKReachReindex::remove_edge(vertex_t s, vertex_t t)
{
    DynamicKReachBase::remove_edge(s, t);
    clear_index();
    generate_index();
}

void DynamicKReachReindex::remove_vertex(vertex_t v)
{
    DynamicKReachBase::remove_vertex(v);

    vertex_t old_v = v;
    v = mapper.query(v);
    mapper.remove(old_v);

    if (index.count(v)){
        for (const auto &p : index.at(v).in){
            index.at(p).out.erase(v);
        }
        for (const auto &q : index.at(v).out){
            index.at(q).in.erase(v);
        }
        index.erase(v);
    }

    graph.at(v).clear();

    clear_index();
    generate_index();
}

void DynamicKReachReindex::clear_index()
{
    for_each(index.begin(), index.end(), [](auto &v){
        v.second.clear();
    });
    weight.clear();
}
