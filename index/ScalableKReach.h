#pragma once

#include "common.h"
#include "Graph.h"
#include "AbstractKReach.h"

class ScalableKReach : public AbstractKReach{
public:
    ScalableKReach(const Graph &graph, distance_t k, uint32_t b1, uint32_t b2);

    void construct();

    bool query(vertex_t s, vertex_t t) const;

    void insert_edge(vertex_t s, vertex_t t);

    void remove_edge(vertex_t s, vertex_t t);

    void remove_vertex(vertex_t v, const std::vector<vertex_t> &out, const std::vector<vertex_t> &in);

    void insert_vertex(vertex_t v, const std::vector<vertex_t> &out, const std::vector<vertex_t> &in);

protected:
    class PartialIndex {
    public:
        PartialIndex(ScalableKReach &parent,
                     const Graph &graph,
                     distance_t k, uint32_t budget, bool isD2 = false);

        void construct();

        bool indexed(vertex_t v) const;

        distance_t distance(vertex_t s, vertex_t t) const;

        bool single_intermediate(vertex_t s, vertex_t t) const;

        bool double_intermediate(vertex_t s, vertex_t t) const;

        void insert_edge(vertex_t s, vertex_t t);

        void remove_edge(vertex_t s, vertex_t t);

        void remove_vertex(vertex_t v, const std::vector<vertex_t> &out, const std::vector<vertex_t> &in);

        void insert_vertex(vertex_t v, const std::vector<vertex_t> &out, const std::vector<vertex_t> &in);

    protected:
        void set_degree();

        void cover(vertex_t v);

        void update_cover(vertex_t v);

        void construct_bfs(vertex_t s,
                           const std::vector<std::vector<vertex_t>> &graph,
                           std::vector<distance_t> &dist);

        ScalableKReach &parent_;
        const Graph& graph_;
        const distance_t k_;
        uint32_t budget_;
        bool isD2_;
        Index succ_, pred_;

        DegreeQueue &quedeg_;
        std::vector<degree_t> &degree_;
        std::vector<vertex_t> &queue_;
        size_t back_, front_;
    };

    bool bfs(vertex_t s, vertex_t t) const;

    const Graph& graph_;
    const distance_t k_;
    PartialIndex D1_, D2_;
    DegreeQueue quedeg_;

    std::vector<degree_t> degree_;
    mutable std::vector<distance_t> distance_;
    mutable std::vector<vertex_t> queue_;
    mutable size_t back_, front_;

};

