#include "dynamic_k_reach_v2.h"

#include <algorithm>
#include <iostream>

using namespace std;

void dynamic_k_reach_v2::construct_index(
        const vector<pair<vertex_t, vertex_t>> &edges, weight_t k)
{
    this->k = k;
    size_t num_vertices;
    {
        unordered_map<vertex_t, uint32_t> degree;
        for (const auto &e : edges) {
            degree[e.first]++;
            degree[e.second]++;
        }
        num_vertices = degree.size();
        vector<pair<uint32_t, vertex_t>> order;
        order.reserve(num_vertices);
        for (const auto &p : degree) {
            order.push_back(make_pair(p.second, p.first));
        }
        sort(order.begin(), order.end(), greater<pair<uint32_t, vertex_t>>());
        for (vertex_t id = 0; id < num_vertices; ++id) {
            mapping[order[id].second] = id;
        }
    }

    out_neighbors.resize(num_vertices);
    in_neighbors.resize(num_vertices);
    out_index.resize(num_vertices);
    in_index.resize(num_vertices);
    for (const auto &e : edges) {
        neighbors_insert(mapping[e.first], mapping[e.second]);
    }

    cover.resize(num_vertices);

    tmp_cover_vertices.reserve(num_vertices);

    generate_cover();

    {
        vector<pair<double_t, vertex_t>> order;
        order.reserve(num_vertices);
        for (const auto &v : tmp_cover_vertices) {
            order.push_back(make_pair((double_t) out_neighbors[v].size()
                                      / in_neighbors[v].size(), v));
        }
        sort(order.begin(), order.end());

        tmp_visited.resize(num_vertices);
        tmp_frontier.resize(num_vertices);

        for (const auto &i : order) {
            bfs(i.second);
            /*for (size_t i = 0; i < tmp_back; ++i) {
                tmp_visited[tmp_frontier[i]] = 0;
            }*/
            fill(tmp_visited.begin(), tmp_visited.end(), 0);
        }

        tmp_visited.clear();
        tmp_frontier.clear();
    }

//    tmp_cover_vertices.clear();
}

void dynamic_k_reach_v2::generate_cover()
{
    auto &cover_vertices = tmp_cover_vertices;
    const vertex_t num_vertices = out_neighbors.size();
    for (vertex_t s = 0; s < num_vertices; ++s) {
        if (cover[s]) {
            continue;
        }
        const auto &in_nei = in_neighbors[s];
        const auto &out_nei = out_neighbors[s];
        auto in_it = in_nei.begin();
        auto out_it = out_nei.begin();
        vertex_t t;
        while (in_it != in_nei.end() && out_it != out_nei.end()) {
            if (*in_it == *out_it) {
                t = *in_it;
                ++in_it;
                ++out_it;
            }
            else if (*in_it < *out_it) {
                t = *in_it;
                ++in_it;
            }
            else {
                t = *out_it;
                ++out_it;
            }
            if (!cover[t]) {
                goto next;
            }
        }
        while (in_it != in_nei.end()) {
            t = *in_it;
            ++in_it;
            if (!cover[t]) {
                goto next;
            }
        }
        while (out_it != out_nei.end()) {
            t = *out_it;
            ++out_it;
            if (!cover[t]) {
                goto next;
            }
        }
        continue;
        next:
        cover[s] = cover[t] = 1;
        cover_vertices.push_back(s);
        cover_vertices.push_back(t);
    }
}

void dynamic_k_reach_v2::neighbors_insert(vertex_t s, vertex_t t)
{
    auto &out_nei = out_neighbors[s];
    auto it = lower_bound(out_nei.begin(), out_nei.end(), t);
    if (it != out_nei.end() && *it == t) {
        return;
    }
    out_nei.insert(it, t);

    auto &in_nei = in_neighbors[t];
    it = lower_bound(in_nei.begin(), in_nei.end(), s);
    in_nei.insert(it, s);
}

void dynamic_k_reach_v2::neighbors_remove(vertex_t s, vertex_t t)
{
    auto &out_nei = out_neighbors[s];
    auto it = lower_bound(out_nei.begin(), out_nei.end(), t);
    if (it == out_nei.end() || it != out_nei.end() && *it != t) {
        return;
    }
    out_nei.erase(it);

    auto &in_nei = in_neighbors[t];
    it = lower_bound(in_nei.begin(), in_nei.end(), s);
    in_nei.erase(it);
}

dynamic_k_reach_v2::neighbors_adj_t::iterator dynamic_k_reach_v2::neighbors_find(neighbors_adj_t &nei, vertex_t v)
{
    auto it = lower_bound(nei.begin(), nei.end(), v);
    return (it == nei.end() || it != nei.end() && *it != v) ? nei.end() : it;
}

pair<dynamic_k_reach_v2::index_adj_t::iterator, dynamic_k_reach_v2::index_adj_t::iterator> dynamic_k_reach_v2::index_insert(vertex_t s, vertex_t t, weight_t w)
{
    auto &out_ind = out_index[s],  &in_ind = in_index[t];
    auto out_it = lower_bound(out_ind.begin(), out_ind.end(), t), in_it = lower_bound(in_ind.begin(), in_ind.end(), s);
    if (out_it != out_ind.end() && out_it->vertex == t && out_it->weight < w) {
        return make_pair(out_it, in_it);
    }

    if (out_it != out_ind.end() && out_it->vertex == t) {
        out_it->weight = w;
        in_it->weight = w;
    }
    else {
        out_it = out_ind.insert(out_it, index_entry_t(t, w));
        in_it = in_ind.insert(in_it, index_entry_t(s, w));
    }
    return make_pair(out_it, in_it);
}

pair<dynamic_k_reach_v2::index_adj_t::iterator, dynamic_k_reach_v2::index_adj_t::iterator> dynamic_k_reach_v2::index_remove(vertex_t s, vertex_t t)
{
    auto &out_ind = out_index[s], &in_ind = in_index[t];
    auto out_it = index_find(out_ind, t);
    if (out_it == out_ind.end()) {
        return make_pair(out_ind.end(), in_ind.end());
    }
    out_it = out_ind.erase(out_it);

    auto in_it = index_find(in_ind, s);
    in_it = in_ind.erase(in_it);
    return make_pair(out_it, in_it);
}


void dynamic_k_reach_v2::index_invalidate(vertex_t s, vertex_t t)
{
    auto &out_ind = out_index[s];
    auto it = lower_bound(out_ind.begin(), out_ind.end(), t);
    if (it == out_ind.end() || it != out_ind.end() && it->vertex != t) {
        return;
    }
    it->weight = MAX_WEIGHT / 2;

    auto &in_ind = in_index[t];
    it = lower_bound(in_ind.begin(), in_ind.end(), s);
    it->weight = MAX_WEIGHT / 2;
}

dynamic_k_reach_v2::index_adj_t::iterator dynamic_k_reach_v2::index_find(index_adj_t &ind, vertex_t v)
{
    auto it = lower_bound(ind.begin(), ind.end(), v);
    return (it == ind.end() || it != ind.end() && it->vertex != v) ? ind.end() : it;
}

void dynamic_k_reach_v2::bfs(vertex_t s) // use parallel for with n vectors.
{
    auto &frontier = tmp_frontier;
    auto &visited = tmp_visited;

    visited[s] = 1;
    size_t front = 0, &back = tmp_back = 0;
    frontier[back++] = s;
    index_insert(s, s, 0);
    for (weight_t level = 0; level < k && front < back; ++level) {
        const size_t current_back = back;
        for (; front < current_back; ++front) {
            vertex_t u = frontier[front];
            for (const auto &v : out_neighbors[u]) {
                if (visited[v]) {
                    continue;
                }
                visited[v] = 1;
                if (cover[v]) {
                    auto &out_ind = out_index[s];
                    auto it = index_find(out_ind, v);
                    if (it != out_ind.end() && it->weight <= level + 1){ // path already present
                        continue;
                    }
                    index_insert(s, v, level + 1);
                    if (out_index[v].size()) { // vertex already indexed
                        for (const auto &i : out_index[v]) {
                            if (i.vertex != s && i.vertex != v && i.weight + level + 1 <= k) {
                                index_insert(s, i.vertex, i.weight + level + 1);
                            }
                        }
                        continue;
                    }
                }
                if (!out_neighbors[v].size()){ // zero out-degree pruning
                    continue;
                }
                frontier[back++] = v;
            }
        }
    }
}

bool dynamic_k_reach_v2::intersect_query(const neighbors_adj_t &nei,
                                         const index_adj_t &ind,
                                         const weight_t w) const
{
    auto nei_it = nei.begin();
    auto ind_it = ind.begin();
    bool result = false;
    while (nei_it != nei.end() && ind_it != ind.end() && !result) {
        if (*nei_it == ind_it->vertex) {
            result |= ind_it->weight <= w;
            ++nei_it;
            ++ind_it;
        }
        else if (*nei_it < ind_it->vertex) {
            ++nei_it;
        }
        else {
            ++ind_it;
        }
    }
    return result;
}

bool dynamic_k_reach_v2::query_reachability(vertex_t s, vertex_t t) const
{
    if (!mapping.count(s) || !mapping.count(t)) {
        return false;
    }
    s = mapping.at(s);
    t = mapping.at(t);
    if (cover[s] && cover[t]) {
        auto &out_ind = out_index[s];
        auto it = lower_bound(out_ind.begin(), out_ind.end(), t);
        return it != out_ind.end() && it->vertex == t;
    }
    else if (cover[s]) {
        return intersect_query(in_neighbors[t], out_index[s], k - 1);
    }
    else if (cover[t]) {
        return intersect_query(out_neighbors[s], in_index[t], k - 1);
    }
    else {
        for (const auto &i: out_neighbors[s]) {
            if (intersect_query(in_neighbors[t], out_index[i], k - 2)) {
                return true;
            }
        }
        return false;
    }
}

void dynamic_k_reach_v2::insert_edge_update(vertex_t s, vertex_t t, weight_t d)
{
    for (const auto &i : in_index[s]){
        for (const auto &j : out_index[t]){
            if (i.weight + j.weight + d <= k) {
                index_insert(i.vertex, j.vertex, i.weight + j.weight + d);
            }
        }
    }
}

void dynamic_k_reach_v2::insert_edge(vertex_t s, vertex_t t)
{
    if (!mapping.count(s) || !mapping.count(t) || s == t) {
        return;
    }
    s = mapping.at(s);
    t = mapping.at(t);
    if (neighbors_find(out_neighbors[s], t) != out_neighbors[s].end()){
        return;
    }
    if (!cover[s] && !cover[t]){
        vertex_t new_covered_vertex;
        if (out_neighbors[s].size() + in_neighbors[s].size() > out_neighbors[t].size() + in_neighbors[t].size()){
            new_covered_vertex = s;
        }
        else {
            new_covered_vertex = t;
        }
        cover[new_covered_vertex] = 1;
        index_insert(new_covered_vertex, new_covered_vertex, 0);
        for (const auto &w : out_neighbors[new_covered_vertex]){
            for (const auto &q : out_index[w]){
                if (q.vertex != new_covered_vertex && q.weight + 1 <= k){
                    index_insert(new_covered_vertex, q.vertex, q.weight + 1);
                }
            }
        }
        for (const auto &w: in_neighbors[new_covered_vertex]){
            for (const auto &p : in_index[w]){
                if (p.vertex != new_covered_vertex && p.weight + 1 <= k){
                    index_insert(p.vertex, new_covered_vertex, p.weight + 1);
                }
            }
        }
    }
    neighbors_insert(s, t);
    if (cover[s] && cover[t]){
        insert_edge_update(s, t, 1);
    }
    else if (cover[s]){
        for (const auto &i : out_neighbors[t]){
            insert_edge_update(s, i, 2);
        }
    }
    else { // cover[t]
        for (const auto &i : in_neighbors[s]){
            insert_edge_update(i, t, 2);
        }
    }
}


void dynamic_k_reach_v2::insert_edge_reindex(vertex_t s, vertex_t t)
{
    if (!mapping.count(s) || !mapping.count(t) || s == t) {
        return;
    }
    s = mapping.at(s);
    t = mapping.at(t);
    if (neighbors_find(out_neighbors[s], t) != out_neighbors[s].end()){
        return;
    }
    neighbors_insert(s, t);
    for (const auto &i : tmp_cover_vertices){
        out_index[i].clear();
        in_index[i].clear();
    }
    if (!cover[s] && !cover[t]){
        if (out_neighbors[s].size() + in_neighbors[s].size() > out_neighbors[t].size() + in_neighbors[t].size()){
            cover[s] = 1;
            tmp_cover_vertices.push_back(s);
        }
        else {
            cover[t] = 1;
            tmp_cover_vertices.push_back(t);
        }
    }

    size_t num_vertices = out_neighbors.size();
    vector<pair<double_t, vertex_t>> order;
    order.reserve(num_vertices);
    for (const auto &v : tmp_cover_vertices) {
        order.push_back(make_pair((double_t) out_neighbors[v].size()
                                  / in_neighbors[v].size(), v));
    }
    sort(order.begin(), order.end());

    tmp_visited.resize(num_vertices);
    tmp_frontier.resize(num_vertices);

    for (const auto &i : order) {
        bfs(i.second);
        fill(tmp_visited.begin(), tmp_visited.end(), 0);
    }

    tmp_visited.clear();
    tmp_frontier.clear();
}

bool
dynamic_k_reach_v2::check_pair(vertex_t p, vertex_t q, weight_t d_ps, weight_t d_tq,
                               weight_t d) const
{
    auto &out_ind = out_index[p];
    auto it = lower_bound(out_ind.begin(), out_ind.end(), q);
    if (it == out_ind.end() || it != out_ind.end() && it->vertex != q){
        return false;
    }
    return it->weight == d_ps + d_tq + d;
}

void dynamic_k_reach_v2::remove_edge(vertex_t s, vertex_t t)
{
    if (!mapping.count(s) || !mapping.count(t) || s == t) {
        return;
    }
    s = mapping.at(s);
    t = mapping.at(t);

    if (neighbors_find(out_neighbors[s], t) == out_neighbors[s].end()){
        return;
    }
    neighbors_remove(s, t);

    if (cover[s] && cover[t]){
        identify_affected(s, t, 1);
    }
    else if (cover[s]){
        for (const auto &w : out_neighbors[t]){
            auto &out_nei = out_neighbors[s], &in_nei = in_neighbors[w];
            auto n_it = neighbors_find(out_nei, w);
            auto &out_ind = out_index[s];
            auto i_it = index_find(out_ind, w);
            if (n_it != out_nei.end()
                || i_it != out_ind.end() && reachable_neighbors(s, w) == i_it->weight){
                continue;
            }
            identify_affected(s, w, 2);
        }
    }
    else { // cover[t]
        for (const auto &w : in_neighbors[s]){
            auto &out_nei = out_neighbors[w], &in_nei = in_neighbors[t];
            auto n_it = neighbors_find(out_nei, t);
            auto &out_ind = out_index[w];
            auto i_it = index_find(out_ind, t);
            if (n_it != out_nei.end()
                || i_it != out_ind.end() && reachable_neighbors(w, t) == i_it->weight){
                continue;
            }
            identify_affected(w, t, 2);
        }
    }
    update_affected();

    tmp_affected.clear();
}

void dynamic_k_reach_v2::remove_edge_reindex(vertex_t s, vertex_t t)
{
    if (!mapping.count(s) || !mapping.count(t) || s == t) {
        return;
    }
    s = mapping.at(s);
    t = mapping.at(t);
    if (neighbors_find(out_neighbors[s], t) == out_neighbors[s].end()){
        return;
    }
    neighbors_remove(s, t);
    for (const auto &i : tmp_cover_vertices){
        out_index[i].clear();
        in_index[i].clear();
    }
    size_t num_vertices = out_neighbors.size();
    vector<pair<double_t, vertex_t>> order;
    order.reserve(num_vertices);
    for (const auto &v : tmp_cover_vertices) {
        order.push_back(make_pair((double_t) out_neighbors[v].size()
                                  / in_neighbors[v].size(), v));
    }
    sort(order.begin(), order.end());

    tmp_visited.resize(num_vertices);
    tmp_frontier.resize(num_vertices);

    for (const auto &i : order) {
        bfs(i.second);
        fill(tmp_visited.begin(), tmp_visited.end(), 0);
    }

    tmp_visited.clear();
    tmp_frontier.clear();
}


void dynamic_k_reach_v2::identify_affected(vertex_t s, vertex_t t, weight_t d)
{
    auto &affected = tmp_affected;
    for (const auto &p : in_index[s]){
        for (const auto &q : out_index[t]){
            if (check_pair(p.vertex, q.vertex, p.weight, q.weight, d)){
                if (p.weight + q.weight + d <= 2){
                    weight_t updated_distance = reachable_neighbors(p.vertex, q.vertex);
                    if (updated_distance){
                        index_insert(p.vertex, q.vertex, updated_distance);
                        continue;
                    }
                }
                affected[make_pair(p.vertex, q.vertex)] = p.weight + q.weight + d;
                index_invalidate(p.vertex, q.vertex);
            }
        }
    }
}

void dynamic_k_reach_v2::update_affected()
{
    auto &affected = tmp_affected;
    while (affected.size()){
        auto it = affected.begin();
        vertex_t s = it->first.first, t = it->first.second;
        index_find(out_index[s], t)->weight++;
        index_find(in_index[t], s)->weight++;
        fix_affected(s, t);
    }
}

pair<dynamic_k_reach_v2::index_adj_t::iterator, dynamic_k_reach_v2::index_adj_t::iterator>
dynamic_k_reach_v2::fix_affected(vertex_t s, vertex_t t)
{
//    weight_t updated_distance = 0;
//    if (tmp_affected[make_pair(s, t)] <= 2){
//        updated_distance = reachable_neighbors(s, t);
//    }
//    if (!updated_distance){
    weight_t updated_distance = reachable_index(s, t);
//    }
    tmp_affected.erase(make_pair(s, t));
    if (updated_distance){
        return index_insert(s, t, updated_distance);
    }
    else {
        return index_remove(s, t);
    }
}

weight_t dynamic_k_reach_v2::reachable_index(vertex_t s, vertex_t t)
{
    auto &s_ind = out_index[s], &t_ind = in_index[t];
    auto s_it = s_ind.begin();
    auto t_it = t_ind.begin();
    weight_t result = MAX_WEIGHT;
    while (s_it != s_ind.end() && t_it != t_ind.end()){
        if (s_it->vertex == t_it->vertex && s_it->weight && t_it->weight){
            if (s_it->weight == MAX_WEIGHT / 2){
                s_it->weight++;
                index_find(in_index[s_it->vertex], s)->weight++;
                s_it = fix_affected(s, s_it->vertex).first;
                if (s_it->vertex != t_it->vertex){
                    continue;
                }
            }
            if (t_it->weight == MAX_WEIGHT / 2){
                t_it->weight++;
                index_find(out_index[t_it->vertex], t)->weight++;
                t_it = fix_affected(t_it->vertex, t).second;
                if (s_it->vertex != t_it->vertex){
                    continue;
                }
            }
            result = min(result, weight_t(s_it->weight + t_it->weight));
            ++s_it;
            ++t_it;
        }
        else if (s_it->vertex < t_it->vertex){
            ++s_it;
        }
        else {
            ++t_it;
        }
    }
    return (result == MAX_WEIGHT || result > k) ? 0 : result;
}

weight_t dynamic_k_reach_v2::reachable_neighbors(vertex_t s, vertex_t t)
{
    auto &s_nei = out_neighbors[s], &t_nei = in_neighbors[t];
    auto s_it = s_nei.begin();
    auto t_it = t_nei.begin();
    while (s_it != s_nei.end() && t_it != t_nei.end()){
        if (*s_it == *t_it){
            return 2 <= k ? 2 : 0;
        }
        else if (*s_it < *t_it){
            ++s_it;
        }
        else {
            ++t_it;
        }
    }
    return 0;
}

const vector<dynamic_k_reach_v2::index_adj_t> &dynamic_k_reach_v2::getOut_index() const
{
    return out_index;
}

const vector<dynamic_k_reach_v2::index_adj_t> &dynamic_k_reach_v2::getIn_index() const
{
    return in_index;
}