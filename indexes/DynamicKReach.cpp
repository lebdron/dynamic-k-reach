#include <IntersectionIterator.h>
#include "DynamicKReach.h"

using namespace std;

void DynamicKReach::insert_update(vertex_t s, vertex_t t, weight_t d)
{
    for_each(index[s].in.begin(), index[s].in.end(), [s, t, d, this](const auto &p) {
        if (weight(p, s) == k - d + 1) {
            return;
        }
        for_each(index[t].out.begin(), index[t].out.end(), [s, t, p, d, this](const auto &q) {
            if (weight(t, q) == k - d + 1) {
                return;
            }
            if (weight.defined(p, q) && weight(p, s) + weight(t, q) + d < weight(p, q)) {
                weight(p, q) = weight(p, s) + weight(t, q) + d;
            }
            else if (!weight.defined(p, q) && weight(p, s) + weight(t, q) + d <= k) {
                index.insert(p, q);
                weight(p, q) = weight(p, s) + weight(t, q) + d;
            }
        });
    });
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
        weight(v, v) = 0;
        for_each(graph[v].out.begin(), graph[v].out.end(), [v, this](const auto &w) {
            for_each(index[w].out.begin(), index[w].out.end(), [v, w, this](const auto &q) {
                if (!weight.defined(v, q) && weight(w, q) + 1 <= k) {
                    index.insert(v, q);
                    weight(v, q) = weight(w, q) + 1;
                }
            });
        });
        for_each(graph[v].in.begin(), graph[v].in.end(), [v, this](const auto &w) {
            for_each(index[w].in.begin(), index[w].in.end(), [v, w, this](const auto &p) {
                if (!weight.defined(p, v) && weight(p, w) + 1 <= k) {
                    index.insert(p, v);
                    weight(p, v) = weight(p, w) + 1;
                }
            });
        });
    }

    if (index.count(s) && index.count(t)) {
        insert_update(s, t, 1);
    }
    else if (index.count(s)) {
        for_each(graph[t].out.begin(), graph[t].out.end(), [s, this](const auto &w) {
            this->insert_update(s, w, 2);
        });
    }
    else {
        for_each(graph[s].in.begin(), graph[s].in.end(), [t, this](const auto &w) {
            this->insert_update(w, t, 2);
        });
    }
}

void DynamicKReach::remove_edge(vertex_t s, vertex_t t)
{
    DynamicKReachBase::remove_edge(s, t);

    s = mapper.query(s);
    t = mapper.query(t);

    unordered_set<Edge, EdgeHash> identified;

    if (index.count(s) && index.count(t)) {
        remove_identify(s, t, 1, identified);
    }
    else if (index.count(s)) {
        for (const auto &w : graph[t].out) {
            // prune if s -1> w, or s -2> w not through t
            /*if (graph[s].out.count(w)
                || IntersectionIterator(graph[s].out.begin(), graph[s].out.end(),
                                        graph[w].in.begin(), graph[w].in.end())
                   != IntersectionIterator(graph[s].out.end(), graph[s].out.end(),
                                           graph[w].in.end(), graph[w].in.end())) {
                return;
            }*/
            remove_identify(s, w, 2, identified);
        }
    }
    else {
        for (const auto &w : graph[s].in) {
            // prune if w -1> t, or w -2> t not through s
            /*if (graph[w].out.count(t)
                || IntersectionIterator(graph[w].out.begin(), graph[w].out.end(),
                                        graph[t].in.begin(), graph[t].in.end())
                   != IntersectionIterator(graph[w].out.end(), graph[w].out.end(),
                                           graph[t].in.end(), graph[t].in.end())) {
                return;
            }*/
            remove_identify(w, t, 2, identified);
        }
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
}

void DynamicKReach::remove_vertex(vertex_t v)
{
    DynamicKReachBase::remove_vertex(v);
}

void DynamicKReach::remove_identify(vertex_t s, vertex_t t, weight_t d, std::unordered_set<Edge, EdgeHash> &identified)
{
    for (const auto &p : index[s].in){
        if (weight(p, s) == k - d + 1) {
            continue;
        }
        for (const auto &q : index[t].out){
            if (weight(t, q) == k - d + 1) {
                continue;
            }
            if (weight.defined(p, q) && weight(p, q) == weight(p, s) + weight(t, q) + d){
                /*if (weight(p, s) + weight(t, q) + d <= 2
                    && IntersectionIterator(graph[p].out.begin(), graph[p].out.end(),
                                            graph[q].in.begin(), graph[q].in.end())
                       != IntersectionIterator(graph[p].out.end(), graph[p].out.end(),
                                               graph[q].in.end(), graph[q].in.end())) {
                    weight(p, q) = 2;
                    continue;
                }*/
                identified.insert(Edge(p, q));
                weight.undefine(p, q);
            }
        }
    }

    for (const auto &e : identified){
        assert(!weight.defined(e.first, e.second));
    }
}

void DynamicKReach::remove_update(vertex_t s, vertex_t t, std::unordered_set<Edge, EdgeHash> &identified)
{
    identified.erase(Edge(s, t));
    if (IntersectionIterator(graph[s].out.begin(), graph[s].out.end(), graph[t].in.begin(), graph[t].in.end())
        != IntersectionIterator(graph[s].out.end(), graph[s].out.end(), graph[t].in.end(), graph[t].in.end())){
        weight(s, t) = 2;

        assert(index[s].out.count(t) && weight.defined(s, t));

        return;
    }
    IntersectionIterator begin(index[s].out.begin(), index[s].out.end(), index[t].in.begin(), index[t].in.end()),
            end(index[s].out.end(), index[s].out.end(), index[t].in.end(), index[t].in.end());
    weight_t result_weight = k + 1;
    for (auto it = begin; it != end; ++it){
        if (identified.count(Edge(s, *it))){
            remove_update(s, *it, identified);
        }
        if (identified.count(Edge(*it, t))){
            remove_update(*it, t, identified);
        }
        if (!weight.defined(s, *it) || !weight.defined(*it, t)){
            continue;
        }
        if (weight(s, *it) + weight(*it, t) < result_weight){
            result_weight = weight(s, *it) + weight(*it, t);
        }
    }
    if (result_weight <= k){
        weight(s, t) = result_weight;
    }
    else {
        index.remove(s, t);
    }

    assert((index[s].out.count(t) && weight.defined(s, t)) || (!index[s].out.count(t) && !weight.defined(s, t)));
}
