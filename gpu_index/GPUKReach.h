#pragma once

#include <common.h>
#include "GPUGraph.h"
#include <AbstractKReach.h>

class GPUKReach : public AbstractKReach {
public:
    GPUKReach(const GPUGraph &graph, distance_t k);

    void construct();

    bool query(vertex_t s, vertex_t t) const;

    void insert_edge(vertex_t s, vertex_t t);

    void remove_edge(vertex_t s, vertex_t t);

    void remove_vertex(vertex_t v, const std::vector<vertex_t> &out, const std::vector<vertex_t> &in);

    void insert_vertex(vertex_t v, const std::vector<vertex_t> &out, const std::vector<vertex_t> &in);

protected:

    bool indexed(vertex_t v) const;

    distance_t distance(vertex_t s, vertex_t t) const;

    void set_degree();

    void cover(vertex_t v);

    void construct_bfs(vertex_t s,
                       std::vector<distance_t> &dist);

    const GPUGraph& graph_;
    const distance_t k_;
    Index index_;
    DegreeQueue quedeg_;

    std::vector<degree_t> degree_;
    std::vector<vertex_t> queue_;
    size_t back_, front_;

    mgpu::mem_t<distance_t> d_distance_;
    mgpu::mem_t<int> d_visited_bits_;

    friend void gpu_bfs(const GPUKReach &kReach, vertex_t s, std::vector<distance_t> &dist);
};
