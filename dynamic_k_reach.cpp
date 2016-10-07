#include <fstream>
#include <algorithm>
#include <queue>
#include "dynamic_k_reach.h"

using namespace std;

vertex_t dynamic_k_reach::vertex(const edge_t &e) const
{
    return e.first;
}

weight_t dynamic_k_reach::weight(const edge_t &e) const
{
    return e.second;
}

void dynamic_k_reach::index_insert(const vertex_t s, const vertex_t t,
                                   const weight_t weight)
{
    if (weight <= k) {
        out_index[s][t] = in_index[t][s] = weight;
    }
}

void dynamic_k_reach::neighbors_insert(const vertex_t s, const vertex_t t)
{
    out_neighbors[s].insert(t);
    in_neighbors[t].insert(s);
}

void dynamic_k_reach::generate_cover()
{
    /*for (const auto &u: out_neighbors){
        if (!cover.count(u.first)){
            for (const auto &v: u.second){
                if (!cover.count(v)){
                    cover.insert({u.first, v});
                    break;
                }
            }
        }
    }*/
//    cover.insert({2, 4, 7, 9});
    cover.insert({2, 1, 6});
}

void dynamic_k_reach::construct_index(string filename,
                                      weight_t k)
{
    this->k = k;
    ifstream fin(filename);

    vertex_t s, t;
    while (fin >> s >> t) {
        neighbors_insert(s, t);
    }

    fin.close();
    generate_cover();
    /*for (const auto &v: cover) {
        bfs_index(v);
    }*/
    /*bfs_index(9);
    bfs_index(7);
    bfs_index(4);
    bfs_index(2);*/
    bfs_index(2);
    bfs_index(1);
    bfs_index(6);
}

void dynamic_k_reach::index_update(const vertex_t s, const vertex_t t,
                                   const weight_t difference)
{
    for (const auto &i: out_index[t]) {
        for (const auto &j: in_index[s]) {
            index_insert(vertex(j), vertex(i),
                         weight(j) + weight(i) + difference);
        }
    }
}

void dynamic_k_reach::insert_edge(vertex_t s, vertex_t t)
{
    neighbors_insert(s, t);
    if (!cover.count(s) && !cover.count(t)) {
        if (!in_neighbors[s].size() || !out_neighbors[s].size()) {
            cover.insert(s);
        }
        else {
            cover.insert(t);
        }
    }
    if (cover.count(s) && cover.count(t)) {
        index_update(s, t, 1);
    }
    else if (cover.count(s)) {
        for (const auto &i: out_neighbors[t]) {
            if (cover.count(i)) {
                index_update(s, i, 2);
            }
        }
    }
    else {
        for (const auto &i: in_neighbors[s]) {
            if (cover.count(i)) {
                index_update(i, t, 2);
            }
        }
    }
}

void dynamic_k_reach::remove_edge(vertex_t s, vertex_t t)
{ // Can we only traverse index?
    out_neighbors[s].erase(t);
    in_neighbors[t].erase(s);
    cover_t out_affected, in_affected;

    // check that edge is used: e.g. d(in_nei(s),t)=2
    if (cover.count(s) &&
        cover.count(t)) { // merge in_index[s] and in_index[t],
        // leaving only d(in_index[s], s) + 1 = d(in_index[t], t)
        intersect_aff(s, t, in_index, out_affected, 1);
    }
    else if (cover.count(t)) { // in_nei(s),t
        for (const auto &i: in_neighbors[s]) {
            if (cover.count(i)) {
                intersect_aff(i, t, in_index, out_affected, 2);
            }
        }
    }
    else { // s,out_nei(t)
        for (const auto &i: out_neighbors[t]) {

        }
    }
}

void dynamic_k_reach::insert_vertex(vertex_t v)
{
    in_neighbors[v];
    out_neighbors[v];
}

void dynamic_k_reach::remove_vertex(vertex_t v)
{
    const auto in_nei = in_neighbors.at(v);
    in_neighbors.erase(v);
    out_neighbors.erase(v);
    if (cover.count(v)) {
        cover.erase(v);
        out_index.erase(v);
        in_index.erase(v);
    }
    for (const auto &i: in_nei) {
        remove_edge(i, v);
    }
}

void dynamic_k_reach::intersect_aff(const vertex_t s, const vertex_t t,
                                    const index_t &index,
                                    cover_t &affected,
                                    const weight_t difference) const
{
    auto s_it = index.at(s).begin();
    auto t_it = index.at(t).begin();
    while (s_it != index.at(s).end() && t_it != index.at(t).end()) {
        if (s_it->first == t_it->first && s_it->second + difference ==
                                          t_it->second) { // TODO check if in_affected would be same as out_affected
            affected.insert(s_it->first);
        }
        else if (s_it->first < t_it->second) {
            ++s_it;
        }
        else {
            ++t_it;
        }
    }
}

bool dynamic_k_reach::intersect_adj(const graph_adj_t &graph_adj,
                                    const index_adj_t &index_adj,
                                    const weight_t weight_adj) const
{
    auto graph_it = graph_adj.begin();
    auto index_it = index_adj.begin();
    bool result = false;
    while (graph_it != graph_adj.end() && index_it != index_adj.end() && !result) {
        if (*graph_it == vertex(*index_it)) {
            result |= weight(*index_it) <= weight_adj;
            ++graph_it;
            ++index_it;
        }
        else if (*graph_it < vertex(*index_it)) {
            ++graph_it;
        }
        else {
            ++index_it;
        }
    }
    return result;
}

bool dynamic_k_reach::query_reachability(vertex_t s, vertex_t t) const
{
    if (cover.count(s) && cover.count(t)) { // case 1
        return out_index.at(s).find(t) != out_index.at(s).end();
    }
    else if (cover.count(s) && !cover.count(t)) { // case 2
        return intersect_adj(in_neighbors.at(t), out_index.at(s), k - 1);
    }
    else if (!cover.count(s) && cover.count(t)) { // case 3
        return intersect_adj(out_neighbors.at(s), in_index.at(t), k - 1);
    }
    else { // case 4
        for (const auto &i: out_neighbors.at(s)) {
            if (intersect_adj(in_neighbors.at(t), out_index.at(i), k - 2)) {
                return true;
            }
        }
        return false;
    }
}

void dynamic_k_reach::bfs_index(vertex_t s)
{
    queue<vertex_t> frontier;
    set<vertex_t> visited;
    index_insert(s, s, 0);
    frontier.push(s);
    visited.insert(s);
    weight_t cur_level = 1, next_level = 0;
    for (weight_t level = 0; level < k && !frontier.empty();) {
        vertex_t u = frontier.front();
        frontier.pop();
        for (const auto &v: out_neighbors.at(u)) {
            if (!visited.count(v)) {
                visited.insert(v);
                if (cover.count(v)) {
                    index_insert(s, v, level + 1);
                    if (out_index.count(v)) { // prune
                        for (const auto &i: out_index[v]) {
                            if (vertex(i) != s) {
                                index_insert(s, vertex(i), weight(i) + level + 1);
                            }
                        }
                        continue;
                    }
                }
                frontier.push(v);
                ++next_level;
            }
        }
        if (!--cur_level) {
            swap(cur_level, next_level);
            ++level;
        }
    }
}