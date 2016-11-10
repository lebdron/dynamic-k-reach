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
    multimap<double_t, vertex_t> order;
    for_each(index.begin(), index.end(), [&order, this](const auto &v) {
        order.insert(make_pair(double_t(graph[v.first].out.size()) / graph[v.first].in.size(), v.first));
    });
    for_each(order.begin(), order.end(), [this](const auto &v) {
        this->bfs(v.second);
    });

    for (const auto &u : index){
        for (const auto &v : u.second.out){
            assert(weight.defined(u.first, v));
        }
        for (const auto &v : u.second.in){
            assert(weight.defined(v, u.first));
        }
    }
}

void KReach::construct_index(vector<Edge> edges, weight_t k)
{
    clear();

    this->k = k;
    {
        using degree_t = uint32_t;
        unordered_map<vertex_t, degree_t> degree;
        for_each(edges.begin(), edges.end(), [&degree](const auto &e) {
            degree[e.first]++;
            degree[e.second]++;
        });
        multimap<degree_t, vertex_t, greater<degree_t>> order;
        for_each(degree.begin(), degree.end(), [&order](const auto &v) {
            order.insert(make_pair(v.second, v.first));
        });
        for_each(order.begin(), order.end(), [this](const auto &v) {
            mapper.insert(v.second);
        });
        graph.resize(degree.size());
        for_each(edges.begin(), edges.end(), [this](const auto &e) {
            graph.insert(mapper.query(e.first), mapper.query(e.second));
        });
    }
    generate_cover();
    generate_index();
}

void KReach::generate_cover()
{
    assert(index.empty());

    for (vertex_t s = 0; s < graph.size(); ++s) {
        if (index.count(s)) {
            continue;
        }
        find_if(graph.at(s).begin(), graph.at(s).end(), [s, this](const auto &t) {
            if (index.count(t)) {
                return false;
            }
            index[s];
            index[t];
            return true;
        });
    }
}

void KReach::bfs(vertex_t s)
{
    assert(index[s].out.empty());

    unordered_set<vertex_t> visited;
    queue<vertex_t> frontier;

    visited.insert(s);
    frontier.push(s);
    index.insert(s, s);
    weight(s, s) = 0;
    for (weight_t level = 0; !frontier.empty() && level < k; ++level) {
        size_t num_frontiers = frontier.size();
        for (size_t i = 0; i < num_frontiers; ++i) {
            vertex_t u = frontier.front();
            frontier.pop();
//            for_each(graph[u].out.begin(), graph[u].out.end(), [s, &visited, &frontier, level, this](const auto &v) {
            for (const auto &v : graph[u].out) {
                if (visited.count(v)) {
                    return;
                }
                visited.insert(v);
                if (index.count(v)) {
                    /*if (weight.defined(s, v) && weight(s, v) <= level + 1) {
                        return;
                    }
                    if (!index[v].out.empty()) {
                        for_each(index[v].out.begin(), index[v].out.end(), [s, level, v, this](const auto &t) {
                            if (!weight.defined(s, t) && weight(v, t) + level + 1 <= k) {
                                index.insert(s, t);
                                weight(s, t) = weight(v, t) + level + 1;
                            }
                            else if (weight.defined(s, t) && weight(s, t) > weight(v, t) + level + 1) {
                                weight(s, t) = weight(v, t) + level + 1;
                            }
                        });
                        return;
                    }
                    if (!weight.defined(s, v)) {*/
                    index.insert(s, v);
                    weight(s, v) = level + 1;
                    /*}
                    else {
                        weight(s, v) = level + 1;
                    }
                }
                if (graph[v].out.empty()) {
                    return;*/
                }
                frontier.push(v);
//            });
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
        return index.at(s).out.count(t) != 0;
    }
    else if (index.count(s)) {
        const auto &is = index.at(s).out, &gt = graph.at(t).in;
        IntersectionIterator begin(is.begin(), is.end(), gt.begin(), gt.end()),
                end(is.end(), is.end(), gt.end(), gt.end());
        return find_if(begin, end, [s, this](const vertex_t &v) {
            return weight(s, v) <= k - 1;
        }) != end;
    }
    else if (index.count(t)) {
        const auto &gs = graph.at(s).out, &it = index.at(t).in;
        IntersectionIterator begin(it.begin(), it.end(), gs.begin(), gs.end()),
                end(it.end(), it.end(), gs.end(), gs.end());
        return find_if(begin, end, [t, this](const vertex_t &v) {
            return weight(v, t) <= k - 1;
        }) != end;
    }
    else {
        const auto &gs = graph.at(s).out;
        return find_if(gs.begin(), gs.end(), [t, this](const vertex_t &i) {
            const auto &ii = index.at(i).out, &gt = graph.at(t).in;
            IntersectionIterator begin(ii.begin(), ii.end(), gt.begin(), gt.end()),
                    end(ii.end(), ii.end(), gt.end(), gt.end());
            return find_if(begin, end, [i, this](const vertex_t &v) {
                return weight(i, v) <= k - 2;
            }) != end;
        }) != gs.end();
    }
}
