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

void dynamic_k_reach::index_insert_update(const vertex_t s, const vertex_t t,
                                          const weight_t weight_n)
{
    auto it = out_index[s].find(t);
    if (it == out_index[s].end()
        || it != out_index[s].end() && weight_n < weight(*it)){
        index_insert(s, t, weight_n);
    }
}

void dynamic_k_reach::index_remove(const vertex_t s, const vertex_t t)
{
    out_index[s].erase(t);
    in_index[t].erase(s);
}

void dynamic_k_reach::neighbors_insert(const vertex_t s, const vertex_t t)
{
    out_neighbors[s].insert(t);
    in_neighbors[t].insert(s);
}

void dynamic_k_reach::neighbors_remove(const vertex_t s, const vertex_t t)
{
    out_neighbors[s].erase(t);
    in_neighbors[t].erase(s);
}

void dynamic_k_reach::generate_cover()
{
    for (const auto &u: out_neighbors){
        if (!cover.count(u.first)){
            for (const auto &v: u.second){
                if (!cover.count(v)){
                    cover.insert({u.first, v});
                    break;
                }
            }
        }
    }
}

void dynamic_k_reach::construct_index(string filename,
                                      weight_t k)
{
    this->k = k;
    ifstream fin(filename);

    vertex_t s, t;
    while (fin >> s >> t) {
        insert_vertex(s);
        insert_vertex(t);
        neighbors_insert(s, t);
    }

    fin.close();
    generate_cover();
    for (const auto &v: cover) {
        bfs_index(v);
    }
}

void dynamic_k_reach::insert_edge_update(const vertex_t s, const vertex_t t,
                                         const weight_t difference)
{
    for (const auto &i: in_index[s]) {
        for (const auto &j: out_index[t]) {
            index_insert_update(vertex(i), vertex(j),
                         weight(i) + weight(j) + difference);
        }
    }
}

void dynamic_k_reach::insert_edge(vertex_t s, vertex_t t)
{
    neighbors_insert(s, t);
    if (!cover.count(s) && !cover.count(t)) { // Add vertex with higher degree
        if (!in_neighbors[s].size() || !out_neighbors[s].size()) {
            cover.insert(t);
        }
        else {
            cover.insert(s);
        }
    }
    if (cover.count(s) && cover.count(t)) {
        insert_edge_update(s, t, 1);
    }
    else if (cover.count(s)) {
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

bool dynamic_k_reach::check_pair(const vertex_t p, const vertex_t q,
                                 const vertex_t s, const vertex_t t,
                                 const weight_t difference) const
{
    return out_index.at(p).count(q)
           && out_index.at(p).at(q)
              == out_index.at(p).at(s) + out_index.at(t).at(q) + difference;
}

void dynamic_k_reach::remove_edge(vertex_t s, vertex_t t)
{
    neighbors_remove(s, t);

    weight_t updated_distance;
    set<pair<vertex_t, vertex_t>> affected;

    if (cover.count(s) && cover.count(t)){
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
        for (const auto &a: affected){
            vertex_t p = a.first, q = a.second;
            updated_distance = intersect_remove(out_index.at(p), in_index.at(q));
            if (updated_distance){
                index_insert(p, q, updated_distance);
            }
        }
    }
    else if (cover.count(s)){
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
        // Update index
        for (const auto &a: affected){
            vertex_t p = a.first, q = a.second;
            updated_distance = intersect_remove(out_index.at(p), in_index.at(q));
            if (updated_distance){
                index_insert(p, q, updated_distance);
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
        // Update index
        for (const auto &a: affected){
            vertex_t p = a.first, q = a.second;
            updated_distance = intersect_remove(out_index.at(p), in_index.at(q));
            if (updated_distance){
                index_insert(p, q, updated_distance);
            }
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
    for (const auto &i: in_neighbors[v]){
        out_neighbors[i].erase(v);
    }
    for (const auto &i: out_neighbors[v]){
        in_neighbors[i].erase(v);
    }

    weight_t updated_distance;
    set<pair<vertex_t, vertex_t>> affected;

    if (cover.count(v)){
        for (const auto &i: in_index[v]){
            out_index[vertex(i)].erase(v);
        }
        for (const auto &i: out_index[v]){
            in_index[vertex(i)].erase(v);
        }
        out_index.erase(v);
        in_index.erase(v);
        cover.erase(v);
    }

    for (const auto &s: in_neighbors.at(v)){
        if (!cover.count(s)){
            continue;
        }
        for (const auto &t: out_neighbors.at(v)){
            if (!cover.count(t)){
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

    out_neighbors.erase(v);
    in_neighbors.erase(v);

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
    if (cover.count(s) && cover.count(t)) { // case 1
        return out_index.at(s).find(t) != out_index.at(s).end();
    }
    else if (cover.count(s)) { // case 2
        return intersect_query(in_neighbors.at(t), out_index.at(s), k - 1);
    }
    else if (cover.count(t)) { // case 3
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
    queue<vertex_t> frontier;
    set<vertex_t> visited;
    frontier.push(s);
    visited.insert(s);
    weight_t cur_level = 1, next_level = 0;
    for (weight_t level = 0; level < k && !frontier.empty();) {
        vertex_t u = frontier.front();
        frontier.pop();
        if (cover.count(u)) {
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
        for (const auto &v: out_neighbors.at(u)) {
            if (!visited.count(v)) {
                visited.insert(v);
                frontier.push(v);
                ++next_level;
            }
        }
        prune:
        if (!--cur_level) {
            swap(cur_level, next_level);
            ++level;
        }
    }
}