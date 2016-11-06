#include <queue>
#include <unordered_set>
#include <cassert>
#include "dynamic_k_reach_v3.h"

using namespace std;

void dynamic_k_reach_v3::construct_index(vector<edge_t> edges, weight_t k)
{
    this->k = k;
    {
        unordered_map<vertex_t, uint32_t> degree;
        for_each(edges.begin(), edges.end(), [&degree](const auto &e) {
            degree[e.first]++;
            degree[e.second]++;
        });
        multimap<uint32_t, vertex_t, greater<uint32_t>> order;
        for_each(degree.begin(), degree.end(), [&order](const auto &v) {
            order.insert(make_pair(v.second, v.first));
        });
        for_each(order.begin(), order.end(), [this, i = 0](const auto &v) mutable {
            mapping[v.second] = i++;
        });
        graph.resize(degree.size());
        for_each(edges.begin(), edges.end(), [this](const auto &e) {
            graph.insert(mapping[e.first], mapping[e.second]);
        });
    }
    generate_cover();
    {
        multimap<double_t, vertex_t> order;
        for_each(index.begin(), index.end(), [&order, this](const auto &v) {
            order.insert(make_pair(double_t(graph[v.first].out.size()) / graph[v.first].in.size(), v.first));
        });
        for_each(order.begin(), order.end(), [this](const auto &v) {
            this->bfs(v.second);
        });
    }
}

void dynamic_k_reach_v3::generate_cover()
{
    assert(index.empty());

    for (size_t s = 0; s < graph.size(); ++s){
        if (index.count(s)) {
            return;
        }
        find_if(graph.at(s).begin(), graph.at(s).end(), [s, this](auto t) {
            if (index.count(t)) {
                return false;
            }
            index[s];
            index[t];
            return true;
        });
    }
}

void dynamic_k_reach_v3::bfs(vertex_t s)
{
    assert(index[s].out.empty());

    unordered_set<vertex_t> visited;
    queue<vertex_t> frontier;

    visited.insert(s);
    frontier.push(s);
    index.insert(s, s);
    weight(s, s) = 0;
    for (weight_t level = 0; !frontier.empty() && level < k ; ++level) {
        size_t num_frontiers = frontier.size();
        for (size_t i = 0; i < num_frontiers; ++i) {
            vertex_t u = frontier.front();
            frontier.pop();
            for_each(graph[u].out.begin(), graph[u].out.end(), [s, &visited, &frontier, level, this](const auto &v) {
                if (visited.count(v)) {
                    return;
                }
                visited.insert(v);
                if (index.count(v)) {
                    if (weight.count(s, v) && weight(s, v) <= level + 1) {
                        return;
                    }
                    if (!index[v].out.empty()) {
                        for_each(index[v].out.begin(), index[v].out.end(), [s, level, v, this](const auto &t) {
                            if (!weight.count(s, t) && weight(v, t) + level + 1 <= k) {
                                index.insert(s, t);
                                weight(s, t) = weight(v, t) + level + 1;
                            }
                            else if (weight.count(s, t) && weight(s, t) > weight(v, t) + level + 1 <= k) {
                                weight(s, t) = weight(v, t) + level + 1;
                            }
                            // TODO check if else is required
                        });
                        return;
                    }
                    if (!weight.count(s, v)) {
                        index.insert(s, v);
                        weight(s, v) = level + 1;
                    }
                    else {
                        weight(s, v) = level + 1;
                    }
                }
                if (graph[v].out.empty()) {
                    return;
                }
                frontier.push(v);
            });
        }
    }
}

void dynamic_k_reach_v3::edge_insert(vertex_t s, vertex_t t)
{
    if (!mapping.count(s) || !mapping.count(t) || s == t) {
        return;
    }
    s = mapping.at(s);
    t = mapping.at(t);
    if (!graph[s].out.count(t)){
        return;
    }

    if (!index.count(s) && !index.count(t)){
        vertex_t vertex = graph[s].size() > graph[t].size() ? s : t;
        index[vertex];
        weight(vertex, vertex) = 0;
    }
}

void dynamic_k_reach_v3::neighbors_t::iterator::set_value()
{
    if (out != outer.out.end() && in != outer.in.end()) {
        value = *out <= *in ? *out : *in;
    }
    else if (out != outer.out.end()) {
        value = *out;
    }
    else if (in != outer.in.end()) {
        value = *in;
    }
}

dynamic_k_reach_v3::neighbors_t::iterator::iterator(dynamic_k_reach_v3::neighbors_t &outer,
                                                    adjacent_t::iterator out,
                                                    adjacent_t::iterator in) : outer(outer), out(out), in(in)
{
    set_value();
}

dynamic_k_reach_v3::neighbors_t::iterator &dynamic_k_reach_v3::neighbors_t::iterator::operator++()
{
    if (out != outer.out.end() && in != outer.in.end()) {
        if (*out == *in) {
            ++out;
            ++in;
        }
        else if (*out < *in) {
            ++out;
        }
        else {
            ++in;
        }
    }
    else if (out != outer.out.end()) {
        ++out;
    }
    else if (in != outer.in.end()) {
        ++in;
    }

    set_value();
    return *this;
}

dynamic_k_reach_v3::neighbors_t::iterator dynamic_k_reach_v3::neighbors_t::iterator::operator++(int)
{
    iterator val = *this;
    ++(*this);
    return val;
}

bool dynamic_k_reach_v3::neighbors_t::iterator::operator==(dynamic_k_reach_v3::neighbors_t::iterator it) const
{
    return out == it.out && in == it.in;
}

bool dynamic_k_reach_v3::neighbors_t::iterator::operator!=(dynamic_k_reach_v3::neighbors_t::iterator it) const
{
    return !(*this == it);
}

unsigned int dynamic_k_reach_v3::neighbors_t::iterator::operator*() const
{
    return value;
}

dynamic_k_reach_v3::neighbors_t::iterator dynamic_k_reach_v3::neighbors_t::begin()
{
    return iterator(*this, out.begin(), in.begin());
}

dynamic_k_reach_v3::neighbors_t::iterator dynamic_k_reach_v3::neighbors_t::end()
{
    return iterator(*this, out.end(), in.end());
}

size_t dynamic_k_reach_v3::neighbors_t::size() const
{
    return out.size() + in.size();
}

std::size_t dynamic_k_reach_v3::edge_hasher::operator()(const edge_t &e) const
{
    return hash<vertex_t>{}(e.first) ^ hash<vertex_t>{}(e.second) + e.first == e.second ? e.first : 0;
}

bool dynamic_k_reach_v3::edge_comparator::operator()(const edge_t &e1, const edge_t &e2) const
{
    return e1.first == e2.first && e1.second == e2.second || e1.first == e2.second && e1.second == e2.first;
}

weight_t &dynamic_k_reach_v3::weight_function::operator()(vertex_t s, vertex_t t)
{
    return operator[](edge_t(s, t));
}

unsigned long dynamic_k_reach_v3::weight_function::count(vertex_t s, vertex_t t) const
{
    return weight_function_t::count(edge_t(s, t));
}

void dynamic_k_reach_v3::graph_adjacency_lists::insert(vertex_t s, vertex_t t)
{
    assert(!at(s).out.count(t));
    assert(!at(t).in.count(s));

    at(s).out.insert(t);
    at(t).in.insert(s);

    assert(at(s).out.count(t));
    assert(at(t).in.count(s));
}

void dynamic_k_reach_v3::graph_adjacency_lists::erase(vertex_t s, vertex_t t)
{
    assert(at(s).out.count(t));
    assert(at(t).in.count(s));

    at(s).out.erase(t);
    at(t).in.erase(s);

    assert(!at(s).out.count(t));
    assert(!at(t).in.count(s));
}

void dynamic_k_reach_v3::index_adjacency_lists::insert(vertex_t s, vertex_t t)
{
    assert(!at(s).out.count(t));
    assert(!at(t).in.count(s));

    at(s).out.insert(t);
    at(t).in.insert(s);

    assert(at(s).out.count(t));
    assert(at(t).in.count(s));
}

void dynamic_k_reach_v3::index_adjacency_lists::erase(vertex_t s, vertex_t t)
{
    assert(at(s).out.count(t));
    assert(at(t).in.count(s));

    at(s).out.erase(t);
    at(t).in.erase(s);

    assert(!at(s).out.count(t));
    assert(!at(t).in.count(s));
}
