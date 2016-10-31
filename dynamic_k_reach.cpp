#include <fstream>
#include <boost/circular_buffer.hpp>

#include "dynamic_k_reach.h"

using namespace std;

void dynamic_k_reach::construct_index(const vector<edge_t> &edges, weight_t k)
{
    this->k = k;
    vertex_t id = 0;
    {
        unordered_map<vertex_t, uint32_t> degree;
        for (const auto &e: edges) {
            degree[e.first]++;
            degree[e.second]++;
        }

        set<pair<uint32_t, vertex_t>, greater<pair<uint32_t, vertex_t>>> ordering;
        for (const auto &v: degree) {
            ordering.insert(make_pair(v.second, v.first));
        }
        for (const auto &v: ordering) {
            input_mapping[v.second] = id++;
        }
    }

    out_neighbors.resize(id);
    in_neighbors.resize(id);
    for (const auto &e: edges){
        neighbors_insert(input_mapping[e.first], input_mapping[e.second]);
    }

    cover_mask.resize(id);
    generate_cover();

    {
        set<pair<double_t, vertex_t>> ordering;
        id = 0;
        for (size_t v = 0; v < cover_mask.size(); ++v){
            if (v) {
                ordering.insert(make_pair((double) out_neighbors.at(v).size() /
                                          in_neighbors.at(v).size(), v));
                cover_mapping[v] = id++;
            }
        }
        out_index.resize(id);
        in_index.resize(id);
        vector<vertex_t> order;
        order.reserve(ordering.size());
        for (const auto &p: ordering){
            order.push_back(p.second);
        }

        for (const auto &i: order){
            bfs_index(i);
        }
    }
}

inline vertex_t dynamic_k_reach::vertex(const index_entry_t &e) const
{
    return e.first;
}

inline weight_t dynamic_k_reach::weight(const index_entry_t &e) const
{
    return e.second;
}

inline void dynamic_k_reach::neighbors_insert(const vertex_t s, const vertex_t t)
{
    out_neighbors[s].insert(t);
    in_neighbors[t].insert(s);
}

inline void dynamic_k_reach::neighbors_remove(const vertex_t s, const vertex_t t)
{
    out_neighbors[s].erase(t);
    in_neighbors[t].erase(s);
}

void dynamic_k_reach::generate_cover()
{
    vertex_t s = 0;
    for (const auto &neighbors: out_neighbors){
        if (!cover_mask[s]){
            for (const auto &t: neighbors){
                if (!cover_mask[t]){
                    cover_mask[s] = cover_mask[t] = 1;
                    break;
                }
            }
        }
        ++s;
    }
}

/*
 * Internal index modifications
 */

inline bool dynamic_k_reach::map_cover_vertex(vertex_t &v) const
{
    if (cover_mask[v]){
        v = cover_mapping.at(v);
        return true;
    }
    return false;
}


dynamic_k_reach::index_adj_t &dynamic_k_reach::get_out_index(vertex_t v)
{
    map_cover_vertex(v);
    return out_index.at(v);
}

dynamic_k_reach::index_adj_t &dynamic_k_reach::get_in_index(vertex_t v)
{
    map_cover_vertex(v);
    return in_index.at(v);
}

inline void dynamic_k_reach::index_insert(vertex_t s, vertex_t t,
                                          const weight_t weight)
{
    if (!map_cover_vertex(s) || !map_cover_vertex(t)){
        return;
    }
    if (weight <= k) {
        out_index[s][t] = in_index[t][s] = weight;
    }
}

inline void dynamic_k_reach::index_insert_update(vertex_t s, vertex_t t,
                                                 const weight_t weight_n)
{
    if (!map_cover_vertex(s) || !map_cover_vertex(t)){
        return;
    }
    auto it = out_index[s].find(t);
    if (it == out_index[s].end() || it != out_index[s].end() && weight_n < weight(*it)){
        if (weight_n <= k) {
            out_index[s][t] = in_index[t][s] = weight_n;
        }
    }
}

inline void dynamic_k_reach::index_remove(vertex_t s, vertex_t t)
{
    if (!map_cover_vertex(s) || !map_cover_vertex(t)){
        return;
    }
    out_index[s].erase(t);
    in_index[t].erase(s);
}

/*
 * External index modifications
 */

void dynamic_k_reach::insert_edge_update(vertex_t s, vertex_t t,
                                         const weight_t difference)
{
    for (const auto &i: get_in_index(s)) {
        for (const auto &j: get_out_index(t)) {
            index_insert_update(vertex(i), vertex(j),
                                weight(i) + weight(j) + difference);
        }
    }
}

inline bool dynamic_k_reach::map_input_vertex(vertex_t &v) const
{
    if (input_mapping.count(v)){
        v = input_mapping.at(v);
        return true;
    }
    return false;
}

void dynamic_k_reach::insert_edge(vertex_t s, vertex_t t)
{
    if (s >= out_neighbors.size() || t >= out_neighbors.size()){
        return;
    }
    neighbors_insert(s, t);
    if (!cover_mask[s] && !cover_mask[t]) { // Add vertex with higher degree
        if (!in_neighbors[s].size() || !out_neighbors[s].size()) {
            cover_mask[t] = 1;
        }
        else {
            cover_mask[s] = 1;
        }
    }
    if (cover_mask[s] && cover_mask[t]) {
        insert_edge_update(s, t, 1);
    }
    else if (cover_mask[s]) {
        for (const auto &i: out_neighbors[t]) {
            insert_edge_update(s, i, 2);
        }
    }
    else { // cover.count(t)
        for (const auto &i: in_neighbors[s]) {
            insert_edge_update(i, t, 2);
        }
    }
}

weight_t
dynamic_k_reach::intersect_remove(const index_adj_t &s_adj,
                                  const index_adj_t &t_adj) const
{
    auto s_it = s_adj.begin();
    auto t_it = t_adj.begin();
    weight_t result = MAX_WEIGHT;
    while (s_it != s_adj.end() && t_it != t_adj.end()){
        if (vertex(*s_it) == vertex(*t_it) && weight(*s_it) && weight(*t_it)){
            result = min(result, weight_t(weight(*s_it) + weight(*t_it)));
            ++s_it;
            ++t_it;
        }
        else if (vertex(*s_it) < vertex(*t_it)) {
            ++s_it;
        }
        else {
            ++t_it;
        }
    }
    return (result == MAX_WEIGHT || result > k) ? 0 : result;
}


weight_t
dynamic_k_reach::intersect_remove(const dynamic_k_reach::graph_adj_t &s_adj,
                                  const dynamic_k_reach::graph_adj_t &t_adj) const
{
    auto s_it = s_adj.begin();
    auto t_it = t_adj.begin();
    while (s_it != s_adj.end() && t_it != t_adj.end()){
        if (*s_it == *t_it){
            return 2 <= k ? 2 : 0;
        }
        else if (*s_it < *t_it) {
            ++s_it;
        }
        else {
            ++t_it;
        }
    }
    return 0;
}

inline bool dynamic_k_reach::check_pair(const vertex_t p, const vertex_t q,
                                 const vertex_t s, const vertex_t t,
                                 const weight_t difference) const
{
    return out_index.at(p).count(q)
           && out_index.at(p).at(q)
              == out_index.at(p).at(s) + out_index.at(t).at(q) + difference;
}

void dynamic_k_reach::remove_edge(vertex_t s, vertex_t t)
{
    if (s >= out_neighbors.size() || t >= out_neighbors.size()){
        return;
    }
    if (!out_neighbors[s].count(t)){
        return;
    }
    neighbors_remove(s, t);

    weight_t updated_distance;
    set<pair<vertex_t, vertex_t>> affected;

    if (cover_mask[s] && cover_mask[t]){
        // Identify affected
        index_remove(s, t);
        for (const auto &p: in_index.at(s)){
            for (const auto &q: out_index.at(t)){
                if (check_pair(vertex(p), vertex(q), s, t, 1)){
                    affected.insert(make_pair(vertex(p), vertex(q)));
                    index_remove(vertex(p), vertex(q));
                }
            }
        }
        // Update index
        updated_distance = intersect_remove(out_neighbors.at(s), in_neighbors.at(t));
        if (!updated_distance){
            updated_distance = intersect_remove(out_index.at(s), in_index.at(t));
        }
        if (updated_distance) {
            index_insert(s, t, updated_distance);
        }
    }
    else if (cover_mask[s]){
        for (const auto &w: out_neighbors.at(t)){
            if (out_neighbors.at(s).count(w)
                || intersect_remove(out_neighbors.at(s),
                                    in_neighbors.at(w)) == out_index[s][w]){
                continue;
            }
            // Identify affected
            for (const auto &p: in_index.at(s)){
                for (const auto &q: out_index.at(w)){
                    if (check_pair(vertex(p), vertex(q), s, w, 2)){
                        affected.insert(make_pair(vertex(p), vertex(q)));
                        index_remove(vertex(p), vertex(q));
                    }
                }
            }
        }
    }
    else { // cover.count(t)
        for (const auto &w: in_neighbors.at(s)){
            if (out_neighbors.at(w).count(t)
                || intersect_remove(out_neighbors.at(w),
                                    in_neighbors.at(t)) == out_index[w][t]){
                continue;
            }
            // Identify affected
            for (const auto &p: in_index.at(w)){
                for (const auto &q: out_index.at(t)){
                    if (check_pair(vertex(p), vertex(q), w, t, 2)){
                        affected.insert(make_pair(vertex(p), vertex(q)));
                        index_remove(vertex(p), vertex(q));
                    }
                }
            }
        }
    }
    // Update index
    for (const auto &a: affected){
        vertex_t p = a.first, q = a.second;
        updated_distance = intersect_remove(out_index.at(p), in_index.at(q));
        if (updated_distance){
            index_insert(p, q, updated_distance);
        }
    }
}

inline void dynamic_k_reach::insert_vertex(vertex_t v)
{
    input_mapping[v] = out_neighbors.size();
    out_neighbors.resize(out_neighbors.size() + 1);
    in_neighbors.resize(in_neighbors.size() + 1);
    cover_mask.resize(cover_mask.size() + 1);
}

void dynamic_k_reach::remove_vertex(vertex_t v)
{
    if (!map_input_vertex(v)){
        return;
    }
    for (const auto &i: in_neighbors[v]){
        out_neighbors[i].erase(v);
    }
    for (const auto &i: out_neighbors[v]){
        in_neighbors[i].erase(v);
    }

    weight_t updated_distance;
    set<pair<vertex_t, vertex_t>> affected;

    if (cover_mask[v]){
        index_remove(v, v);
        // Identify affected
        for (const auto &p: in_index.at(v)){
            for (const auto &q: out_index.at(v)){
                if (check_pair(vertex(p), vertex(q), v, v, 0)){
                    affected.insert(make_pair(vertex(p), vertex(q)));
                    index_remove(vertex(p), vertex(q));
                }
            }
        }
        for (const auto &i: in_index[v]){
            out_index[vertex(i)].erase(v);
        }
        for (const auto &i: out_index[v]){
            in_index[vertex(i)].erase(v);
        }
        out_index.erase(v);
        in_index.erase(v);
        cover_mask[v] = 0;
    }
    else {
        for (const auto &s: in_neighbors.at(v)){
            if (!cover_mask[s]){
                continue;
            }
            for (const auto &t: out_neighbors.at(v)){
                if (!cover_mask[t]){
                    continue;
                }
                if (out_neighbors.at(s).count(t)
                    || intersect_remove(out_neighbors.at(s),
                                        in_neighbors.at(t)) == out_index[s][t]){
                    continue;
                }
                // Identify affected
                for (const auto &p: in_index.at(s)){
                    for (const auto &q: out_index.at(t)){
                        if (check_pair(vertex(p), vertex(q), s, t, 2)){
                            affected.insert(make_pair(vertex(p), vertex(q)));
                            index_remove(vertex(p), vertex(q));
                        }
                    }
                }
            }
        }
    }

    out_neighbors[v].clear();
    in_neighbors[v].clear();

    // Update index
    for (const auto &a: affected){
        vertex_t p = a.first, q = a.second;
        updated_distance = intersect_remove(out_index.at(p), in_index.at(q));
        if (updated_distance){
            index_insert(p, q, updated_distance);
        }
    }
}

bool dynamic_k_reach::intersect_query(const graph_adj_t &graph_adj,
                                      const index_adj_t &index_adj,
                                      const weight_t weight_adj) const
{
    auto graph_it = graph_adj.begin();
    auto index_it = index_adj.begin();
    bool result = false;
    while (graph_it != graph_adj.end() && index_it != index_adj.end() &&
           !result) {
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
    if (!map_input_vertex(s) || !map_input_vertex(t)){
        return false;
    }
    if (cover_mask[s] && cover_mask[t]) { // case 1
        return out_index.at(s).find(t) != out_index.at(s).end();
    }
    else if (cover_mask[s]) { // case 2
        return intersect_query(in_neighbors.at(t), out_index.at(s), k - 1);
    }
    else if (cover_mask[t]) { // case 3
        return intersect_query(out_neighbors.at(s), in_index.at(t), k - 1);
    }
    else { // case 4
        for (const auto &i: out_neighbors.at(s)) {
            if (intersect_query(in_neighbors.at(t), out_index.at(i), k - 2)) {
                return true;
            }
        }
        return false;
    }
}

void dynamic_k_reach::bfs_index(vertex_t s)
{
    boost::circular_buffer<vertex_t> frontier(out_neighbors.size());
    vector<uint8_t> visited(out_neighbors.size());
    frontier.push_back(s);
    visited[s] = 1;
    vertex_t current_count = 1, next_count = 0;
    for (weight_t level = 0; level <= k && !frontier.empty(); ) {
        vertex_t u = frontier.front();
        frontier.pop_front();
        --current_count;
        if (cover_mask[u]){
            index_insert(s, u, level);
            if (out_index.count(u) && u != s) {
                for (const auto &i: out_index[u]) {
                    if (vertex(i) != s && vertex(i) != u) {
                        index_insert_update(s, vertex(i),
                                            weight(i) + level);
                    }
                }
                goto prune;
            }
        }
        for (const auto &v: out_neighbors.at(u)){
            if (!visited[v]){
                visited[v] = 1;
                frontier.push_back(v);
                ++next_count;
            }
        }
        prune:
        if (!current_count){
            swap(current_count, next_count);
            ++level;
        }
    }
}