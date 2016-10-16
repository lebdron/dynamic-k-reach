#pragma once

#include <map>
#include <set>
#include <unordered_set>
#include <limits>

typedef uint32_t vertex_t;
typedef uint8_t weight_t;

class dynamic_k_reach {
    typedef std::pair<vertex_t, weight_t> edge_t;
    typedef std::set<vertex_t> graph_adj_t;
    typedef std::map<vertex_t, graph_adj_t> graph_t;
    typedef std::map<vertex_t, weight_t> index_adj_t;
    typedef std::map<vertex_t, index_adj_t> index_t;
    typedef std::unordered_set<vertex_t> cover_t;

    const weight_t MAX_WEIGHT = std::numeric_limits<weight_t>::max();

    weight_t k;
    graph_t out_neighbors, in_neighbors;
    index_t out_index, in_index;
    cover_t cover;

    void generate_cover();

    void bfs_index(vertex_t s);

    vertex_t vertex(const edge_t &e) const;

    weight_t weight(const edge_t &e) const;

    void index_insert(const vertex_t s, const vertex_t t, const weight_t weight);

    void index_insert_update(const vertex_t s, const vertex_t t,
                             const weight_t weight_n);

    void index_remove(const vertex_t s, const vertex_t t);

    void
    insert_edge_update(const vertex_t s, const vertex_t t,
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

public:
    void construct_index(std::string filename, weight_t k);

    void insert_edge(vertex_t s, vertex_t t);

    void remove_edge(vertex_t s, vertex_t t);

    void insert_vertex(vertex_t v);

    void remove_vertex(vertex_t v);

    bool query_reachability(vertex_t s, vertex_t t) const;
};
