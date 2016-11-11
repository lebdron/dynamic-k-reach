#include <IntersectionIterator.h>
#include "DynamicKReach.h"

using namespace std;

void DynamicKReach::insert_update(vertex_t s, vertex_t t, weight_t d)
{
    for (const auto &p: index[s].in) {
        if (weight(p, s) == k){
            continue;
        }
        for (const auto &q : index[t].out) {
            if (weight(t, q) == k){
                continue;
            }
            if (weight.defined(p, q) && weight(p, s) + weight(t, q) + d < weight(p, q)) {
                weight.undefine(p, q);
                weight.define(p, q, weight(p, s) + weight(t, q) + d);
            }
            else if (!weight.defined(p, q) && weight(p, s) + weight(t, q) + d <= k) {
                index.insert(p, q);
                weight.define(p, q, weight(p, s) + weight(t, q) + d);
            }
        }
    }
}

void DynamicKReach::insert_edge(vertex_t s, vertex_t t)
{
    DynamicKReachBase::insert_edge(s, t);
    s = mapper.query(s);
    t = mapper.query(t);
    if (!index.count(s) && !index.count(t)) {
        vertex_t v = (graph[s].degree() > graph[t].degree()) ? s : t;
        index[v];
        index.insert(v, v);
        weight.define(v, v, 0);
        for (const auto &w : graph[v].out) {
            for (const auto &q : index[w].out) {
                if (!weight.defined(v, q) && weight(w, q) + 1 <= k) {
                    index.insert(v, q);
                    weight.define(v, q, weight(w, q) + 1);
                }
            }
        }
        for (const auto &w : graph[v].in) {
            for (const auto &p : index[w].in) {
                if (!weight.defined(p, v) && weight(p, w) + 1 <= k) {
                    index.insert(p, v);
                    weight.define(p, v, weight(p, w) + 1);
                }
            }
        }
    }

    if (index.count(s) && index.count(t)) {
        insert_update(s, t, 1);
    }
    else if (index.count(s)) {
        for (const auto &w : graph[t].out) {
            insert_update(s, w, 2);
        }
    }
    else {
        for (const auto &w : graph[s].in) {
            insert_update(w, t, 2);
        }
    }
}

void DynamicKReach::remove_edge(vertex_t s, vertex_t t)
{
    DynamicKReachBase::remove_edge(s, t);

    for (const auto &u : index){
        for (const auto &v : u.second.out){
            assert(weight.defined(u.first, v));
        }
        for (const auto &v : u.second.in){
            assert(weight.defined(v, u.first));
        }
    }
    for (const auto &p : weight){
        assert(index.at(p.first.first).out.count(p.first.second));
    }

    s = mapper.query(s);
    t = mapper.query(t);

    unordered_set<Edge, EdgeHash> identified;

    if (index.count(s) && index.count(t)) {
        remove_identify(s, t, 1, identified);
    }
    else if (index.count(s)) {
        for (const auto &w : graph[t].out) {
            // prune if s -1> w, or s -2> w not through t
            if (graph.has(s, w) || IntersectionIterator(graph.at(s).out.begin(), graph.at(s).out.end(), graph.at(w)
                    .in.begin(), graph.at(w).in.end()) != IntersectionIterator(graph.at(s).out.end(), graph.at(s).out
                    .end(), graph.at(w).in.end(), graph.at(w).in.end())){
                continue;
            }
            remove_identify(s, w, 2, identified);
        }
    }
    else {
        for (const auto &w : graph[s].in) {
            // prune if w -1> t, or w -2> t not through s
            if (graph.has(w, t) || IntersectionIterator(graph.at(w).out.begin(), graph.at(w).out.end(), graph.at(t)
                    .in.begin(), graph.at(t).in.end()) != IntersectionIterator(graph.at(w).out.end(), graph.at(w).out
                    .end(), graph.at(t).in.end(), graph.at(t).in.end())){
                continue;
            }
            remove_identify(w, t, 2, identified);
        }
    }

    for (const auto &e : identified){
        assert(!weight.defined(e.first, e.second));
    }

    while (!identified.empty()){
        Edge e = *identified.begin();
        remove_update(e.first, e.second, identified);
    }

    for (const auto &u : index){
        for (const auto &v : u.second.out){
            assert(weight.defined(u.first, v));
        }
        for (const auto &v : u.second.in){
            assert(weight.defined(v, u.first));
        }
    }
    for (const auto &p : weight){
        assert(index.at(p.first.first).out.count(p.first.second));
    }
}

void DynamicKReach::remove_vertex(vertex_t v)
{
    DynamicKReachBase::remove_vertex(v);

    for (const auto &u : index){
        for (const auto &s : u.second.out){
            assert(weight.defined(u.first, s));
        }
        for (const auto &s : u.second.in){
            assert(weight.defined(s, u.first));
        }
    }
    for (const auto &p : weight){
        assert(index.at(p.first.first).out.count(p.first.second));
    }

    vertex_t old_v = v;
    v = mapper.query(v);
    mapper.remove(old_v);

    unordered_set<Edge, EdgeHash> identified;

    if (index.count(v)){
        index.remove(v, v);
        weight.undefine(v, v);
        remove_identify(v, v, 0, identified);
        for (const auto &p : index.at(v).in){
            index.at(p).out.erase(v);
            weight.undefine(p, v);
        }
        for (const auto &q : index.at(v).out){
            index.at(q).in.erase(v);
            weight.undefine(v, q);
        }
        index.erase(v);
    }
    else {
        for (const auto &p : graph.at(v).in){
            for (const auto &q : graph.at(v).out){
                if (graph.has(p, q) || IntersectionIterator(graph.at(p).out.begin(), graph.at(p).out.end(), graph.at(q)
                        .in.begin(), graph.at(q).in.end()) != IntersectionIterator(graph.at(p).out.end(), graph.at(p).out
                        .end(), graph.at(q).in.end(), graph.at(q).in.end())){
                    continue;
                }
                remove_identify(p, q, 2, identified);
            }
        }
    }

    graph.at(v).clear();

    while (!identified.empty()){
        Edge e = *identified.begin();
        remove_update(e.first, e.second, identified);
    }

    for (const auto &u : index){
        for (const auto &s : u.second.out){
            assert(weight.defined(u.first, s));
        }
        for (const auto &s : u.second.in){
            assert(weight.defined(s, u.first));
        }
    }
    for (const auto &p : weight){
        assert(index.at(p.first.first).out.count(p.first.second));
    }
}

void DynamicKReach::remove_identify(vertex_t s, vertex_t t, weight_t d, std::unordered_set<Edge, EdgeHash> &identified)
{
    for (const auto &p : index.at(s).in){
        if (!weight.defined(p, s) || weight(p, s) == k){
            continue;
        }
        for (const auto &q : index.at(t).out){
            if (!weight.defined(t, q) || weight(t, q) == k){
                continue;
            }
            if (weight.defined(p, q) && weight(p, q) == weight(p, s) + weight(t, q) + d){
                weight.undefine(p, q);
                if (IntersectionIterator(graph[p].out.begin(), graph[p].out.end(), graph[q].in.begin(), graph[q].in
                        .end())
                    != IntersectionIterator(graph[p].out.end(), graph[p].out.end(), graph[q].in.end(), graph[q].in
                        .end())){
                    weight.define(p, q, 2);
                    continue;
                }
                identified.insert(Edge(p, q));
            }
        }
    }
}

void DynamicKReach::remove_update(vertex_t s, vertex_t t, std::unordered_set<Edge, EdgeHash> &identified)
{
    identified.erase(Edge(s, t));

    const auto &ids = index[s].out, &idt = index[t].in;
    IntersectionIterator begin(ids.begin(), ids.end(), idt.begin(), idt.end()),
            end(ids.end(), ids.end(), idt.end(), idt.end());
    weight_t result_weight = k + 1;
    for (auto it = begin; it != end; ++it){
        if (identified.count(Edge(s, *it))){
            remove_update(s, *it, identified);
        }
        if (identified.count(Edge(*it, t))){
            remove_update(*it, t, identified);
        }
        if (!ids.count(*it) || !idt.count(*it)){
            it = IntersectionIterator(ids.begin(), ids.end(), idt.begin(), idt.end());
            continue;
        }
        if (!weight.defined(s, *it) || !weight.defined(*it, t)){
            continue;
        }
        if (weight(s, *it) + weight(*it, t) < result_weight){
            result_weight = weight(s, *it) + weight(*it, t);
        }
    }
    if (result_weight <= k){
        weight.define(s, t, result_weight);
    }
    else {
        index.remove(s, t);
    }

    assert((ids.count(t) && weight.defined(s, t)) || (!ids.count(t) && !weight.defined(s, t)));
}
