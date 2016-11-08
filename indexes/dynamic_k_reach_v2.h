#pragma once

#include <vector>
#include <utility>
#include <limits>
#include <unordered_map>
#include <unordered_set>

#include <boost/functional/hash.hpp>

#include "../common/common.h"

class dynamic_k_reach_v2 {

public:
    struct index_entry_t {
        vertex_t vertex;
        weight_t weight;

        index_entry_t(vertex_t v, weight_t w) : vertex(v), weight(w){}

        bool operator<(const vertex_t v) const
        {
            return vertex < v;
        }

        bool operator==(const index_entry_t &other) const
        {
            return vertex == other.vertex && weight == other.weight;
        }
    };
    typedef std::vector<vertex_t> neighbors_adj_t;
    typedef std::vector<index_entry_t> index_adj_t;

    const weight_t MAX_WEIGHT = std::numeric_limits<weight_t>::max();

    weight_t k;
    std::vector<neighbors_adj_t> out_neighbors, in_neighbors;
    std::vector<index_adj_t> out_index, in_index;

    const std::vector<index_adj_t> &getOut_index() const;

    const std::vector<index_adj_t> &getIn_index() const;

    std::vector<uint8_t> cover;

    std::unordered_map<vertex_t, vertex_t> mapping;
    std::vector<std::size_t> free_neighbors;

    std::vector<uint8_t> tmp_visited;
    std::vector<vertex_t> tmp_frontier, tmp_cover_vertices;
    size_t tmp_back;
    std::unordered_map<std::pair<vertex_t, vertex_t>, weight_t,
            boost::hash<std::pair<vertex_t, vertex_t>>> tmp_affected;

    void generate_cover();

    void neighbors_insert(vertex_t s, vertex_t t);

    void neighbors_remove(vertex_t s, vertex_t t);

    neighbors_adj_t::iterator neighbors_find(neighbors_adj_t &nei, vertex_t v);

    std::pair<index_adj_t::iterator, index_adj_t::iterator> index_insert(vertex_t s, vertex_t t, weight_t w);

    std::pair<index_adj_t::iterator, index_adj_t::iterator> index_remove(vertex_t s, vertex_t t);

    void index_invalidate(vertex_t s, vertex_t t);

    index_adj_t::iterator index_find(index_adj_t &ind, vertex_t v);

    void bfs(vertex_t s);

    bool intersect_query(const neighbors_adj_t &nei, const index_adj_t &ind, const weight_t w) const;

    void insert_edge_update(vertex_t s, vertex_t t, weight_t d);

    bool check_pair(vertex_t p, vertex_t q, weight_t d_ps, weight_t d_tq,
                    weight_t d) const;

    void identify_affected(vertex_t s, vertex_t t, weight_t d);

    void update_affected();

    std::pair<index_adj_t::iterator, index_adj_t::iterator> fix_affected(vertex_t s, vertex_t t);

    weight_t reachable_index(vertex_t s, vertex_t t);

    weight_t reachable_neighbors(vertex_t s, vertex_t t);

    void construct_index(const std::vector<std::pair<vertex_t, vertex_t>> &edges, weight_t k);

    bool query_reachability(vertex_t s, vertex_t t) const;

    void insert_edge(vertex_t s, vertex_t t);

    void remove_edge(vertex_t s, vertex_t t);

    void insert_edge_reindex(vertex_t s, vertex_t t);

    void remove_edge_reindex(vertex_t s, vertex_t t);

    void remove_vertex(vertex_t v);

    void remove_vertex_reindex(vertex_t v);

    void remove_vertex_edges(vertex_t v);
};