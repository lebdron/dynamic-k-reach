#include "ScalableGPUKReach.h"

using namespace std;

void gpu_bfs(const ScalableGPUKReach::PartialIndex &kReach, vertex_t s, const std::vector<degree_t> &h_vertices, const std::vector<vertex_t> &h_edges,
    const degree_t* vertices, const vertex_t* edges, std::vector<distance_t> &dist) {
    distance_t* distance_data = kReach.d_distance_.data();
    int* visited_bits_data = kReach.d_visited_bits_.data();
    auto d1_bits_data = kReach.d_d1_bits_.data();
    auto isD2 = kReach.isD2_;

    cudaMemset(distance_data, 0x3f, kReach.d_distance_.size() * sizeof(distance_t));
    cudaMemset(visited_bits_data, 0, kReach.d_visited_bits_.size() * sizeof(int));

    mgpu::transform([=]MGPU_DEVICE(int index) {
    visited_bits_data[s / 32] = 1 << (31 & s);
    distance_data[s] = 0;
    }, 1, mgpu_context);

  workload_t wl;
  wl.count = h_vertices.at(s + 1) - h_vertices.at(s);
  wl.num_segments = 1;
  vector<int> edge_indices_host = { (int) h_vertices.at(s) };
  wl.segments = mgpu::fill<int>(0, 1, mgpu_context);
  wl.edge_indices = mgpu::to_mem(edge_indices_host, mgpu_context);
  for (int cur_level = 0; cur_level < kReach.k_ && wl.num_segments; ++cur_level) {
      // Create a dynamic work-creation engine.
    auto engine = mgpu::expt::lbs_workcreate(wl.count, wl.segments.data(), 
        wl.num_segments, mgpu_context);

    // The upsweep attempts atomicOr. If it succeeds, return the number of 
    // edges for that vertex.
    auto wl2_count = engine.upsweep(
        [=]MGPU_DEVICE(int index, int seg, int rank, mgpu::tuple<int> desc) {
        int count = 0;
        int neighbor = edges[mgpu::get<0>(desc) + rank];
        int mask = 1<< (31 & neighbor);
        if (0 == (mask & atomicOr(visited_bits_data + neighbor / 32, mask))) {
            if (!isD2 || !(mask & d1_bits_data[neighbor / 32])){
                count = vertices[neighbor + 1] - vertices[neighbor];
            }
            distance_data[neighbor] = cur_level + 1;
        }
        return count;
        }, mgpu::make_tuple(wl.edge_indices.data())
    );

    // The downsweep streams out the new edge pointers.
    mgpu::mem_t<int> edge_indices(wl2_count.num_segments, mgpu_context);
    mgpu::mem_t<int> segments = engine.downsweep(
        [=]MGPU_DEVICE(int dest_seg, int index, int seg, int rank, 
        mgpu::tuple<int> desc, int* out_edge_indices) {
        // Return the same count as before and store output segment-specific
        // data using dest_seg.
        int neighbor = edges[mgpu::get<0>(desc) + rank];
        int begin = vertices[neighbor];
        int end = vertices[neighbor + 1];

        // Store the pointer into the edges array for the new work segment.
        out_edge_indices[dest_seg] = begin;

        return end - begin;
        }, mgpu::make_tuple(wl.edge_indices.data()), edge_indices.data()
    );

    // Update the workload.
    wl.count = wl2_count.count;
    wl.num_segments = wl2_count.num_segments;
    wl.segments = std::move(segments);
    wl.edge_indices = std::move(edge_indices);
  }
  mgpu::dtoh(dist, distance_data, dist.size());
}

void gpu_set_d1(const ScalableGPUKReach::PartialIndex &kReach) {
    auto d1_bits_data = kReach.d_d1_bits_.data();
    auto d1_data = kReach.d_d1_.data();
    mgpu::htod(d1_data, kReach.queue_.data(), kReach.parent_.D1_.succ_.size());
    cudaMemset(d1_bits_data, 0, kReach.d_d1_bits_.size() * sizeof(int));
    mgpu::transform([=]MGPU_DEVICE(int index) {
        vertex_t vertex = d1_data[index];
        atomicOr(d1_bits_data + vertex / 32, 1 << (31 & vertex));
    }, kReach.parent_.D1_.succ_.size(), mgpu_context);
}

ScalableGPUKReach::PartialIndex::PartialIndex(ScalableGPUKReach &parent,
                                           const GPUGraph &graph, distance_t k, uint32_t budget, bool isD2)
        : parent_(parent), graph_(graph), k_(k), budget_(budget), isD2_(isD2),
          quedeg_(parent.quedeg_), degree_(parent.degree_), queue_(parent.queue_), 
          d_distance_(parent.d_distance_), d_visited_bits_(parent.d_visited_bits_), d_d1_bits_(parent.d_d1_bits_), d_d1_(parent.d_d1_) {}

void ScalableGPUKReach::PartialIndex::construct() {
    succ_.clear();
    pred_.clear();
    set_degree();
    for (degree_t i = 0; i < graph_.num_vertices(); ++i) {
        quedeg_.emplace(degree_.at(i), i);
    }
    while (!quedeg_.empty() && succ_.size() != budget_) {
        auto deg = quedeg_.top().first;
        auto cur = quedeg_.top().second;
        quedeg_.pop();
        if (deg <= 0 || degree_.at(cur) <= 0) {
            continue;
        }
        if (deg != degree_.at(cur)) {
            quedeg_.emplace(degree_.at(cur), cur);
            continue;
        }
        succ_[cur] = vector<distance_t>(graph_.num_vertices(), INF8);
        construct_bfs(cur, graph_.vertices(), graph_.edges(), graph_.gpu_vertices(), graph_.gpu_edges(), succ_.at(cur));
        pred_[cur] = vector<distance_t>(graph_.num_vertices(), INF8);
        construct_bfs(cur, graph_.rvertices(), graph_.redges(), graph_.gpu_rvertices(), graph_.gpu_redges(), pred_.at(cur));
        update_cover(cur);
    }
    while (!quedeg_.empty()) {
        quedeg_.pop();
    }
}

void ScalableGPUKReach::PartialIndex::set_degree() {
    copy(graph_.degree().begin(), graph_.degree().end(), degree_.begin());
    if (isD2_) {
        size_t j = 0;
        for (const auto &i : parent_.D1_.succ_) {
            cover(i.first);
            queue_.at(j++) = i.first;
        }
        gpu_set_d1(*this);
    }
}

void ScalableGPUKReach::PartialIndex::cover(vertex_t v) {
    degree_.at(v) = 0;
    for (const auto &i : graph_.successors(v)) {
        if (degree_.at(i) > 0) {
            --degree_.at(i);
        }
    }
    for (const auto &i : graph_.predecessors(v)) {
        if (degree_.at(i) > 0) {
            --degree_.at(i);
        }
    }
}

void ScalableGPUKReach::PartialIndex::update_cover(vertex_t v) {
    for (vertex_t j = 0; j < graph_.num_vertices(); ++j) {
        if ((succ_.at(v).at(j) != INF8 || pred_.at(v).at(j) != INF8) &&
            degree_.at(j) > 0) {
            cover(j);
        }
    }
}

void ScalableGPUKReach::PartialIndex::construct_bfs(vertex_t s, const std::vector<degree_t> &h_vertices, const std::vector<vertex_t> &h_edges,
        const degree_t* vertices, const vertex_t* edges, std::vector<distance_t> &dist) {
   gpu_bfs(*this, s, h_vertices, h_edges, vertices, edges, dist);
}

bool ScalableGPUKReach::PartialIndex::indexed(vertex_t v) const {
    return succ_.find(v) != succ_.end();
}

distance_t ScalableGPUKReach::PartialIndex::distance(vertex_t s, vertex_t t) const {
    return indexed(s) ? succ_.at(s).at(t) : pred_.at(t).at(s);
}

bool ScalableGPUKReach::PartialIndex::single_intermediate(vertex_t s, vertex_t t) const {
    for (const auto &i : succ_) {
        if (pred_.at(i.first).at(s) + succ_.at(i.first).at(t) <= k_) {
            return true;
        }
    }
    return false;
}

bool ScalableGPUKReach::PartialIndex::double_intermediate(vertex_t s, vertex_t t) const {
    for (const auto &i : succ_) {
        for (const auto &j : succ_) {
            if (pred_.at(i.first).at(s) + succ_.at(i.first).at(j.first) + succ_.at(j.first).at(t) <= k_) {
                return true;
            }
        }
    }
    return false;
}

void ScalableGPUKReach::PartialIndex::insert_edge(vertex_t s, vertex_t t) {
    (void) s;
    (void) t;
    construct();
}

void ScalableGPUKReach::PartialIndex::remove_edge(vertex_t s, vertex_t t) {
    (void) s;
    (void) t;
    construct();
}

void ScalableGPUKReach::PartialIndex::remove_vertex(vertex_t v, const std::vector<vertex_t> &out,
                                                 const std::vector<vertex_t> &in) {
    (void) v;
    (void) out;
    (void) in;
    construct();
}

void ScalableGPUKReach::PartialIndex::insert_vertex(vertex_t v, const std::vector<vertex_t> &out,
                                                 const std::vector<vertex_t> &in) {
    (void) v;
    (void) out;
    (void) in;
    construct();
}


ScalableGPUKReach::ScalableGPUKReach(const GPUGraph &graph, distance_t k, uint32_t b1, uint32_t b2)
        : graph_(graph), k_(k), D1_(*this, graph, k, b1), D2_(*this, graph, k, b2, true),
          degree_(graph.num_vertices()), distance_(graph.num_vertices(), INF8), queue_(graph.num_vertices()), 
          d_distance_(graph.num_vertices(), mgpu_context), d_visited_bits_(mgpu::div_up(graph.num_vertices(), 32), mgpu_context), 
          d_d1_bits_(mgpu::div_up(graph.num_vertices(), 32), mgpu_context), d_d1_(graph.num_vertices(), mgpu_context) {
    vector<pair<degree_t, vertex_t>> quedeg_temp_;
    quedeg_temp_.reserve(graph.num_vertices());
    DegreeQueue(less<pair<degree_t, vertex_t>>(), move(quedeg_temp_)).swap(quedeg_);
}

void ScalableGPUKReach::construct() {
    D1_.construct();
    D2_.construct();
}

void ScalableGPUKReach::insert_edge(vertex_t s, vertex_t t) {
    (void) s;
    (void) t;
    construct();
}

void ScalableGPUKReach::remove_edge(vertex_t s, vertex_t t) {
    (void) s;
    (void) t;
    construct();
}

void ScalableGPUKReach::remove_vertex(vertex_t v, const std::vector<vertex_t> &out, const std::vector<vertex_t> &in) {
    (void) v;
    (void) out;
    (void) in;
    construct();
}

bool ScalableGPUKReach::query(vertex_t s, vertex_t t) const {
    if (s == t && s < graph_.num_vertices()) {
        return true;
    }
    if (D1_.indexed(s) && D1_.indexed(t)) {
        return D1_.distance(s, t) <= k_;
    }
    else if (D1_.indexed(s) || D1_.indexed(t)) {
        return D1_.distance(s, t) <= k_ || D1_.single_intermediate(s, t);
    }
    else if (D2_.indexed(s) && D2_.indexed(t)) {
        return D2_.distance(s, t) <= k_ || D1_.double_intermediate(s, t);
    }
    else if (D2_.indexed(s) || D2_.indexed(t)) {
        return D2_.distance(s, t) <= k_ || D2_.single_intermediate(s, t) || D1_.double_intermediate(s, t);
    }
    else {
        return D1_.double_intermediate(s, t) || D2_.double_intermediate(s, t) || bfs(s, t);
    }
}

bool ScalableGPUKReach::bfs(vertex_t s, vertex_t t) const {
    back_ = 0;
    front_ = 0;
    distance_.at(s) = 0;
    queue_.at(back_++) = s;
    while (back_ != front_ && distance_.at(t) == INF8) {
        vertex_t cur = queue_.at(front_++);
        if (distance_.at(cur) >= k_) {
            continue;
        }
        for (const auto &nxt : graph_.successors(cur)) {
            if (distance_.at(nxt) == INF8 && !D1_.indexed(nxt) && !D2_.indexed(nxt)) {
                distance_.at(nxt) = distance_.at(cur) + 1;
                queue_.at(back_++) = nxt;
            }
        }
    }
    bool result = distance_.at(t) <= k_;
    for (size_t i = 0; i < back_; ++i) {
        distance_.at(queue_.at(i)) = INF8;
    }
    return result;
}

void ScalableGPUKReach::insert_vertex(vertex_t v, const std::vector<vertex_t> &out, const std::vector<vertex_t> &in) {
    (void) v;
    (void) out;
    (void) in;
    construct();
}


