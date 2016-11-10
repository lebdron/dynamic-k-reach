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
    // TODO implement
}

void DynamicKReachReindex::clear_index()
{
    for_each(index.begin(), index.end(), [](auto &v){
        v.second.clear();
    });
    weight.clear();
}
