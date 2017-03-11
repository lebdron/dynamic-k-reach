#pragma once

#include "common.h"

class Graph {
public:
    Graph();
    void from_kreach(const std::string &filename);
    void from_edges(const std::string &filename);
    void from_graph(const std::string &filename);
    void from_gra(const std::string &filename);
    const std::vector<vertex_t>& successors(vertex_t v) const;
    const std::vector<vertex_t>& predecessors(vertex_t v) const;
    const std::vector<degree_t>& degree() const;
    degree_t num_vertices() const;
    const std::vector<std::vector<vertex_t>>& graph() const;
    const std::vector<std::vector<vertex_t>>& reverse_graph() const;

    void compute_degree();
    void insert_edge(vertex_t s, vertex_t t);
    void remove_edge(vertex_t s, vertex_t t);
    void remove_vertex(vertex_t v);
    void insert_vertex(vertex_t v, const std::vector<vertex_t> &out, const std::vector<vertex_t> &in);

private:
    std::vector<std::vector<vertex_t>> succ_, pred_;
    std::vector<degree_t> deg_;
    degree_t num_vertices_;
};
