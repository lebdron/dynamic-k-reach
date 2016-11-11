#include <IntersectionIterator.h>
#include "KReach.h"

using namespace std;

void KReach::clear()
{
    k = 0;
    graph.clear();
    index.clear();
    weight.clear();
    mapper.clear();
}

void KReach::generate_index()
{
    assert(!index.empty());

    multimap<double_t, vertex_t> order;
    for (const auto &v : index) {
        order.insert(make_pair(double_t(graph[v.first].out.size()) / graph[v.first].in.size(), v.first));
    }
    for (const auto &v : order) {
        bfs(v.second);
    }
}

void KReach::construct_index(vector<Edge> edges, weight_t k)
{
    clear();

    this->k = k;
    {
        using degree_t = uint32_t;
        unordered_map<vertex_t, degree_t> degree;
        for (const auto &e : edges) {
            degree[e.first]++;
            degree[e.second]++;
        }
        multimap<degree_t, vertex_t, greater<degree_t>> order;
        for (const auto &v : degree) {
            order.insert(make_pair(v.second, v.first));
        }
        for (const auto &v : order) {
            mapper.insert(v.second);
        }
        graph.resize(degree.size());
        for (const auto &e : edges) {
            graph.insert(mapper.query(e.first), mapper.query(e.second));
        }
    }
    generate_cover();
    generate_index();
}

void KReach::generate_cover()
{
    for (vertex_t s = 0; s < graph.size(); ++s) {
        if (index.count(s)) {
            continue;
        }
        for (const auto &t : graph.at(s)) {
            if (index.count(t)) {
                continue;
            }
            index[s];
            index[t];
            break;
        }
    }
}

void KReach::bfs(vertex_t s)
{
    assert(index.at(s).empty());

    unordered_set<vertex_t> visited;
    queue<vertex_t> frontier;

    visited.insert(s);
    frontier.push(s);
    index.insert(s, s);
    weight.define(s, s, 0);
    for (weight_t level = 0; !frontier.empty() && level < k; ++level) {
        size_t num_frontiers = frontier.size();
        for (size_t i = 0; i < num_frontiers; ++i) {
            vertex_t u = frontier.front();
            frontier.pop();
            for (const auto &v : graph.at(u).out) {
                if (visited.count(v)) {
                    continue;
                }
                visited.insert(v);
                if (index.count(v)) {
                    /*if (!index.at(v).out.empty()){
                        for (const auto &t : index.at(v).out){
                            if (weight(v, t) + level + 1 > k){
                                continue;
                            }
                            if (!weight.defined(s, t)){
                                index.insert(s, t);
                                weight.define(s, t, weight(v, t) + level + 1);
                            }
                            else if (weight(s, t) > weight(v, t) + level + 1){
                                weight.update(s, t, weight(v, t) + level + 1);
                            }
                        }
                        continue;
                    }
                    if (weight.defined(s, v) && weight(s, v) <= level + 1){
                        continue;
                    }
                    if (!weight.defined(s, v)) {*/
                        index.insert(s, v);
                        weight.define(s, v, level + 1);
                    /*}
                    else {
                        weight.update(s, v, level + 1);
                    }*/
                }
                if (graph.at(v).out.empty()) {
                    continue;
                }
                frontier.push(v);
            }
        }
    }
}

bool KReach::query(vertex_t s, vertex_t t) const
{
    if (!mapper.present(s) || !mapper.present(t)) {
        return false;
    }

    if (s == t){
        return true;
    }

    s = mapper.query(s);
    t = mapper.query(t);

    if (index.count(s) && index.count(t)) {
        return weight.defined(s, t);
    }
    else if (index.count(s)) {
        const auto &i = index.at(s).out, &g = graph.at(t).in;
        IntersectionIterator begin(i.begin(), i.end(), g.begin(), g.end()),
                end(i.end(), i.end(), g.end(), g.end());
        for (auto it = begin; it != end; ++it) {
            if (weight(s, *it) <= k - 1){
                return true;
            }
        }
    }
    else if (index.count(t)) {
        const auto &g = graph.at(s).out, &i = index.at(t).in;
        IntersectionIterator begin(i.begin(), i.end(), g.begin(), g.end()),
                end(i.end(), i.end(), g.end(), g.end());
        for (auto it = begin; it != end; ++it) {
            if (weight(*it, t) <= k - 1){
                return true;
            }
        }
    }
    else {
        for (const auto &w : graph.at(s).out){
            const auto &i = index.at(w).out, &g = graph.at(t).in;
            IntersectionIterator begin(i.begin(), i.end(), g.begin(), g.end()),
                    end(i.end(), i.end(), g.end(), g.end());
            for (auto it = begin; it != end; ++it) {
                if (weight(w, *it) <= k - 2){
                    return true;
                }
            }
        }
    }
    return false;
}
