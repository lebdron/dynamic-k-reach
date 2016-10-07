#pragma once

#include <map>
#include <set>
#include <unordered_set>

typedef uint32_t vertex_t;
typedef uint8_t weight_t;

class dynamic_k_reach {
    typedef std::pair<vertex_t, weight_t> edge_t;
    typedef std::set<vertex_t> graph_adj_t;
    typedef std::map<vertex_t, graph_adj_t> graph_t;
    typedef std::map<vertex_t, weight_t> index_adj_t;
    typedef std::map<vertex_t, index_adj_t> index_t;
    typedef std::unordered_set<vertex_t> cover_t;

    weight_t k;
    graph_t out_neighbors, in_neighbors;
    index_t out_index, in_index;
    cover_t cover;

    void generate_cover();

    void bfs_index(vertex_t s);

    vertex_t vertex(const edge_t &e) const;

    weight_t weight(const edge_t &e) const;

    void index_insert(const vertex_t s, const vertex_t t, const weight_t weight);

    void
    index_update(const vertex_t s, const vertex_t t, const weight_t difference);

    void neighbors_insert(const vertex_t s, const vertex_t t);

    void intersect_aff(const vertex_t s, const vertex_t t, const index_t &index,
                           cover_t &affected, const weight_t difference) const;

    bool intersect_adj(const graph_adj_t &graph_adj,
                       const index_adj_t &index_adj,
                       const weight_t weight_adj) const;

public:
    void construct_index(std::string filename, weight_t k);

    void insert_edge(vertex_t s, vertex_t t);

    void remove_edge(vertex_t s, vertex_t t);

    void insert_vertex(vertex_t v);

    void remove_vertex(vertex_t v);

    bool query_reachability(vertex_t s, vertex_t t) const;
};
