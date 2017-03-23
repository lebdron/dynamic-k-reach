#pragma once

#include "ScalableGPUKReach.h"

class DynamicScalableGPUKReach : public AbstractKReach{
public:

    DynamicScalableGPUKReach(const GPUGraph &graph, distance_t k, uint32_t b1, uint32_t b2);

    void construct();

    bool query(vertex_t s, vertex_t t) const;

    void insert_edge(vertex_t s, vertex_t t);

    void remove_edge(vertex_t s, vertex_t t);

    void remove_vertex(vertex_t v, const std::vector<vertex_t> &out, const std::vector<vertex_t> &in);

    void insert_vertex(vertex_t v, const std::vector<vertex_t> &out, const std::vector<vertex_t> &in);

    class DynamicPartialIndex {
    public:
        DynamicPartialIndex(DynamicScalableGPUKReach &parent,
                            const GPUGraph &graph,
                            distance_t k, uint32_t budget, bool isD2 = false);

        void construct();

        bool indexed(vertex_t v) const;

        distance_t distance(vertex_t s, vertex_t t) const;

        bool single_intermediate(vertex_t s, vertex_t t) const;

        bool double_intermediate(vertex_t s, vertex_t t) const;

        void insert_edge(vertex_t s, vertex_t t);

        void remove_edge(vertex_t s, vertex_t t);

        void remove_vertex(vertex_t v,
                           const std::vector<vertex_t> &out,
                           const std::vector<vertex_t> &in);

        void insert_vertex(vertex_t v,
                           const std::vector<vertex_t> &out,
                           const std::vector<vertex_t> &in);

    protected:
        void set_degree();

        void cover(vertex_t v);

        void update_cover(vertex_t v);

        void construct_bfs(vertex_t s, const std::vector<degree_t> &h_vertices, const std::vector<vertex_t> &h_edges,
        const degree_t* vertices, const vertex_t* edges, std::vector<distance_t> &dist);

        void resume_bfs(vertex_t s, const std::vector<degree_t> &h_vertices, const std::vector<vertex_t> &h_edges,
        const degree_t* vertices, const vertex_t* edges, std::vector<distance_t> &dist);

        void update_insert(vertex_t s, vertex_t t,
            const std::vector<degree_t> &h_vertices, const std::vector<vertex_t> &h_edges,
            const degree_t* vertices, const vertex_t* edges, std::vector<distance_t> &dist);

        void update_insert(vertex_t v, const std::vector<vertex_t> &out,
            const std::vector<vertex_t> &in,
            const std::vector<degree_t> &h_vertices, const std::vector<vertex_t> &h_edges,
            const degree_t* vertices, const vertex_t* edges, std::vector<distance_t> &dist);

        void update_remove(vertex_t r, vertex_t s, vertex_t t,
            const std::vector<degree_t> &h_vertices, const std::vector<vertex_t> &h_edges,
            const degree_t* vertices, const vertex_t* edges, std::vector<distance_t> &dist);

        void update_remove(vertex_t r, vertex_t v, const std::vector<vertex_t> &out,
            const std::vector<degree_t> &h_vertices, const std::vector<vertex_t> &h_edges,
            const degree_t* vertices, const vertex_t* edges, std::vector<distance_t> &dist);

        DynamicScalableGPUKReach &parent_;
        const GPUGraph& graph_;
        const distance_t k_;
        uint32_t budget_;
        bool isD2_;

        Index succ_, pred_;
        DegreeQueue &quedeg_;
        std::vector<degree_t> &degree_;
        std::vector<vertex_t> &queue_;
        size_t back_, front_;
        DistanceQueue &quedist_;
        std::vector<vertex_t> &updated_;

        Index succ_temp_, pred_temp_;

        mgpu::mem_t<distance_t> &d_distance_;
        mgpu::mem_t<int> &d_visited_bits_;
        mgpu::mem_t<int> &d_d1_bits_;
        mgpu::mem_t<vertex_t> &d_d1_;

        friend void gpu_bfs(const DynamicPartialIndex &kReach, vertex_t s, const std::vector<degree_t> &h_vertices, const std::vector<vertex_t> &h_edges,
            const degree_t* vertices, const vertex_t* edges, std::vector<distance_t> &dist);
        friend void gpu_set_d1(const DynamicPartialIndex &kReach);
        friend void gpu_resume_bfs(const DynamicPartialIndex &kReach, vertex_t s, const std::vector<degree_t> &h_vertices, const std::vector<vertex_t> &h_edges,
            const degree_t* vertices, const vertex_t* edges, std::vector<distance_t> &dist);
    };

    bool bfs(vertex_t s, vertex_t t) const;

    const GPUGraph& graph_;
    const distance_t k_;
    DynamicPartialIndex D1_, D2_;
    DegreeQueue quedeg_;
    DistanceQueue quedist_;

    std::vector<degree_t> degree_;
    mutable std::vector<distance_t> distance_;
    mutable std::vector<vertex_t> queue_;
    mutable size_t back_, front_;
    std::vector<vertex_t> updated_;

    mgpu::mem_t<distance_t> d_distance_;
    mgpu::mem_t<int> d_visited_bits_;
    mgpu::mem_t<int> d_d1_bits_;
    mgpu::mem_t<vertex_t> d_d1_;
};
