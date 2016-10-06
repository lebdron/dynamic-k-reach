#pragma once

#include <map>
#include <set>

typedef uint32_t vertex_t;

class dynamic_k_reach {
    typedef uint8_t weight_t;
    typedef std::set<vertex_t> graph_adj_t;
    typedef std::map<vertex_t, graph_adj_t> graph_t;
    typedef std::map<vertex_t, weight_t> index_adj_t;
    typedef std::map<vertex_t, index_adj_t> index_t;
    typedef std::set<vertex_t> cover_t;

    weight_t k;
    graph_t out_neighbors, in_neighbors;
    index_t out_index, in_index;
    cover_t cover;

    void generate_cover();

    void bfs_index(vertex_t s);

    bool intersect_adj(const graph_adj_t &graph_adj,
                       const index_adj_t &index_adj,
                       const weight_t weight) const;

public:
    void construct_index(std::string filename, weight_t k);

    void insert_edge(vertex_t s, vertex_t t);

    void remove_edge(vertex_t s, vertex_t t);

    void insert_vertex(vertex_t v);

    void remove_vertex(vertex_t v);

    bool query_reachability(vertex_t s, vertex_t t) const;
};
