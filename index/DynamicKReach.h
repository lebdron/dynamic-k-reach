#pragma once

#include "KReach.h"

class DynamicKReach : public AbstractKReach {
public:
    DynamicKReach(const Graph &graph, distance_t k);

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

    void update_remove(vertex_t s, vertex_t t,
                       std::vector<distance_t> &dist);

    void update_remove(vertex_t v, const std::vector<vertex_t> &out,
                       std::vector<distance_t> &dist);

    bool has_parent(vertex_t s,
                    const std::vector<distance_t> &dist);

    void collect_changes(vertex_t s,
                         std::vector<distance_t> &dist);

    void collect_changes(const std::vector<vertex_t> &sv,
                         std::vector<distance_t> &dist);

    void fix_changes(std::vector<distance_t> &dist);

    const Graph& graph_;
    const distance_t k_;
    Index index_;
    DegreeQueue quedeg_;
    DistanceQueue quedist_;

    std::vector<degree_t> degree_;
    std::vector<vertex_t> queue_;
    size_t back_, front_;
    std::vector<vertex_t> updated_;

    Index index_temp_;
};