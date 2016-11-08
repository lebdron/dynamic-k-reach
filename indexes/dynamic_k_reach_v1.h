#pragma once

#include <map>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include <limits>
#include <vector>

typedef uint32_t vertex_t;
typedef std::pair<vertex_t, vertex_t> edge_t;
typedef uint8_t weight_t;

class dynamic_k_reach_v1 {
    typedef std::pair<vertex_t, weight_t> index_entry_t;
    typedef std::set<vertex_t> graph_adj_t;
    typedef std::vector<graph_adj_t> graph_t;
    typedef std::map<vertex_t, weight_t> index_adj_t;
    typedef std::vector<index_adj_t> index_t;

    const weight_t MAX_WEIGHT = std::numeric_limits<weight_t>::max();

    weight_t k;
    graph_t out_neighbors, in_neighbors;
    index_t out_index, in_index;
    std::vector<uint8_t> cover_mask;

    std::unordered_map<vertex_t, vertex_t> input_mapping, cover_mapping;

    bool map_input_vertex(vertex_t &v) const;

    bool map_cover_vertex(vertex_t &v) const;

    void generate_cover();

    void bfs_index(vertex_t s);

    vertex_t vertex(const index_entry_t &e) const;

    weight_t weight(const index_entry_t &e) const;

    void index_insert(vertex_t s, vertex_t t, const weight_t weight);

    void index_insert_update(vertex_t s, vertex_t t,
                             const weight_t weight_n);

    void index_remove(vertex_t s, vertex_t t);

    void
    insert_edge_update(vertex_t s, vertex_t t,
                       const weight_t difference);

    void neighbors_insert(const vertex_t s, const vertex_t t);

    void neighbors_remove(const vertex_t s, const vertex_t t);

    weight_t intersect_remove(const index_adj_t &s_adj, const index_adj_t &t_adj) const;

    weight_t intersect_remove(const graph_adj_t &s_adj, const graph_adj_t &t_adj) const;

    bool intersect_query(const graph_adj_t &graph_adj,
                         const index_adj_t &index_adj,
                         const weight_t weight_adj) const;

    bool check_pair(const vertex_t p, const vertex_t q,
                        const vertex_t s, const vertex_t t,
                        const weight_t difference) const;

    index_adj_t& get_out_index(vertex_t v);

    index_adj_t& get_in_index(vertex_t v);

public:

    void construct_index(const std::vector<edge_t> &edges, weight_t k);

    void insert_edge(vertex_t s, vertex_t t);

    void remove_edge(vertex_t s, vertex_t t);

    void insert_vertex(vertex_t v);

    void remove_vertex(vertex_t v);

    bool query_reachability(vertex_t s, vertex_t t) const;
};
