#pragma once

#include "GPUKReach.h"

class DynamicGPUKReach : public AbstractKReach {
public:
    DynamicGPUKReach(const GPUGraph &graph, distance_t k);

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

    void resume_bfs(vertex_t s,
                    std::vector<distance_t> &dist);

    void update_insert(vertex_t s, vertex_t t,
                       std::vector<distance_t> &dist);

    void update_insert(vertex_t v, const std::vector<vertex_t> &out,
                       const std::vector<vertex_t> &in,
                       std::vector<distance_t> &dist);

    void update_remove(vertex_t r, vertex_t s, vertex_t t,
                       std::vector<distance_t> &dist);

    void update_remove(vertex_t r, vertex_t v, const std::vector<vertex_t> &out,
                       std::vector<distance_t> &dist);

    const GPUGraph& graph_;
    const distance_t k_;
    Index index_;
    DegreeQueue quedeg_;
    DistanceQueue quedist_;

    std::vector<degree_t> degree_;
    std::vector<vertex_t> queue_;
    size_t back_, front_;    

    Index index_temp_;

    mgpu::mem_t<distance_t> d_distance_;
    mgpu::mem_t<int> d_visited_bits_;

    friend void gpu_bfs(const DynamicGPUKReach &kReach, vertex_t s, std::vector<distance_t> &dist);
    friend void gpu_resume_bfs(const DynamicGPUKReach &kReach, vertex_t s, std::vector<distance_t> &dist);
};