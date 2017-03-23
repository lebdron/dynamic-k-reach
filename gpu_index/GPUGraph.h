#pragma once

#include <common.h>
#include "context.h"

class GPUGraph {
public:
    GPUGraph();
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

    const std::vector<degree_t>& vertices() const;
    const std::vector<vertex_t>& edges() const;
    const degree_t* gpu_vertices() const;
    const vertex_t* gpu_edges() const;

    const std::vector<degree_t>& rvertices() const;
    const std::vector<vertex_t>& redges() const;
    const degree_t* gpu_rvertices() const;
    const vertex_t* gpu_redges() const;

    void compute_degree();
    void insert_edge(vertex_t s, vertex_t t);
    void remove_edge(vertex_t s, vertex_t t);
    void remove_vertex(vertex_t v);
    void insert_vertex(vertex_t v, const std::vector<vertex_t> &out, const std::vector<vertex_t> &in);

private:
    std::vector<std::vector<vertex_t>> succ_, pred_;
    std::vector<degree_t> deg_;
    degree_t num_vertices_;
    uint32_t num_edges_;

    std::vector<degree_t> h_vertices_;
    std::vector<vertex_t> h_edges_;
    mgpu::mem_t<degree_t> d_vertices_;
    mgpu::mem_t<vertex_t> d_edges_;

    std::vector<degree_t> h_rvertices_;
    std::vector<vertex_t> h_redges_;
    mgpu::mem_t<degree_t> d_rvertices_;
    mgpu::mem_t<vertex_t> d_redges_;
};
