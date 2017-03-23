#include "DynamicScalableGPUKReach.h"

__device__ char atomicMinCharS(char* address, char val) {
    unsigned int *base_address = (unsigned int *)((size_t)address & ~3);
    unsigned int selectors[] = {0x3214, 0x3240, 0x3410, 0x4210};
    unsigned int sel = selectors[(size_t)address & 3];
    unsigned int old, assumed, min_, new_;

    old = *base_address;
    do {
        assumed = old;
        min_ = min(val, (char)__byte_perm(old, 0, ((size_t)address & 3) | 0x4440));
        new_ = __byte_perm(old, min_, sel);
        if (new_ == old){
            break;
        }
        old = atomicCAS(base_address, assumed, new_);
    } while (assumed != old);

    return old;
}

using namespace std;

void gpu_bfs(const DynamicScalableGPUKReach::DynamicPartialIndex &kReach, vertex_t s, const std::vector<degree_t> &h_vertices, const std::vector<vertex_t> &h_edges,
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

void gpu_set_d1(const DynamicScalableGPUKReach::DynamicPartialIndex &kReach) {
    auto d1_bits_data = kReach.d_d1_bits_.data();
    auto d1_data = kReach.d_d1_.data();
    mgpu::htod(d1_data, kReach.queue_.data(), kReach.parent_.D1_.succ_.size());
    cudaMemset(d1_bits_data, 0, kReach.d_d1_bits_.size() * sizeof(int));
    mgpu::transform([=]MGPU_DEVICE(int index) {
        vertex_t vertex = d1_data[index];
        atomicOr(d1_bits_data + vertex / 32, 1 << (31 & vertex));
    }, kReach.parent_.D1_.succ_.size(), mgpu_context);
}

void gpu_resume_bfs(const DynamicScalableGPUKReach::DynamicPartialIndex &kReach, vertex_t s, const std::vector<degree_t> &h_vertices, const std::vector<vertex_t> &h_edges,
    const degree_t* vertices, const vertex_t* edges, std::vector<distance_t> &dist) {
    distance_t* distance_data = kReach.d_distance_.data();
    int* visited_bits_data = kReach.d_visited_bits_.data();
    auto d1_bits_data = kReach.d_d1_bits_.data();
    auto isD2 = kReach.isD2_;

    mgpu::htod(distance_data, dist.data(), dist.size());
    cudaMemset(visited_bits_data, 0, kReach.d_visited_bits_.size() * sizeof(int));

    mgpu::transform([=]MGPU_DEVICE(int index) {
        if (distance_data[index] < 0x3f) {
            atomicOr(visited_bits_data + index / 32, 1 << (31 & index));
        }
    }, dist.size(), mgpu_context);

  workload_t wl;
  wl.count = h_vertices.at(s + 1) - h_vertices.at(s);
  wl.num_segments = 1;
  vector<int> edge_indices_host = { (int) h_vertices.at(s) };
  wl.segments = mgpu::fill<int>(0, 1, mgpu_context);
  wl.edge_indices = mgpu::to_mem(edge_indices_host, mgpu_context);
  for (int cur_level = dist.at(s); cur_level < kReach.k_ && wl.num_segments; ++cur_level) {
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
        else if (cur_level + 1 < atomicMinCharS((char*)distance_data + neighbor, cur_level + 1)){
            if (!isD2 || !(mask & d1_bits_data[neighbor / 32])){
                count = vertices[neighbor + 1] - vertices[neighbor];
            }
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


void DynamicScalableGPUKReach::DynamicPartialIndex::resume_bfs(vertex_t s, const std::vector<degree_t> &h_vertices, const std::vector<vertex_t> &h_edges,
        const degree_t* vertices, const vertex_t* edges, std::vector<distance_t> &dist) {
    gpu_resume_bfs(*this, s, h_vertices, h_edges, vertices, edges, dist);
}

void DynamicScalableGPUKReach::DynamicPartialIndex::update_insert(vertex_t s, vertex_t t,
        const std::vector<degree_t> &h_vertices, const std::vector<vertex_t> &h_edges,
        const degree_t* vertices, const vertex_t* edges, std::vector<distance_t> &dist) {
    if (dist.at(s) >= k_ || dist.at(t) <= dist.at(s) + 1) {
        return;
    }
    dist.at(t) = dist.at(s) + 1;
    resume_bfs(t, h_vertices, h_edges, vertices, edges, dist);
}

void DynamicScalableGPUKReach::DynamicPartialIndex::insert_edge(vertex_t s, vertex_t t) {
    succ_temp_.clear();
    swap(succ_, succ_temp_);
    pred_temp_.clear();
    swap(pred_, pred_temp_);
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
        if (!isD2_) { // is D1
            if (succ_temp_.find(cur) != succ_temp_.end()) { // was indexed
                succ_[cur] = move(succ_temp_.at(cur));
                update_insert(s, t, graph_.vertices(), graph_.edges(), graph_.gpu_vertices(), graph_.gpu_edges(), succ_.at(cur));
                pred_[cur] = move(pred_temp_.at(cur));
                update_insert(t, s, graph_.rvertices(), graph_.redges(), graph_.gpu_rvertices(), graph_.gpu_redges(), pred_.at(cur));
            }
            else if (parent_.D2_.indexed(cur)) { // indexed in D2
                succ_[cur] = move(parent_.D2_.succ_.at(cur));
                for (const auto &i : succ_temp_) {
                    if (succ_.at(cur).at(i.first) != INF8) {
                        resume_bfs(i.first, graph_.vertices(), graph_.edges(), graph_.gpu_vertices(), graph_.gpu_edges(), succ_.at(cur));
                    }
                }
                update_insert(s, t, graph_.vertices(), graph_.edges(), graph_.gpu_vertices(), graph_.gpu_edges(), succ_.at(cur));
                pred_[cur] = move(parent_.D2_.pred_.at(cur));
                for (const auto &i : succ_temp_) {
                    if (pred_.at(cur).at(i.first) != INF8) {
                        resume_bfs(i.first, graph_.rvertices(), graph_.redges(), graph_.gpu_rvertices(), graph_.gpu_redges(), pred_.at(cur));
                    }
                }
                update_insert(t, s, graph_.rvertices(), graph_.redges(), graph_.gpu_rvertices(), graph_.gpu_redges(), pred_.at(cur));
            }
            else { // not indexed before
                succ_[cur] = vector<distance_t>(graph_.num_vertices(), INF8);
                construct_bfs(cur, graph_.vertices(), graph_.edges(), graph_.gpu_vertices(), graph_.gpu_edges(), succ_.at(cur));
                pred_[cur] = vector<distance_t>(graph_.num_vertices(), INF8);
                construct_bfs(cur, graph_.rvertices(), graph_.redges(), graph_.gpu_rvertices(), graph_.gpu_redges(), pred_.at(cur));
            }
        }
        else { // is D2
            if (succ_temp_.find(cur) != succ_temp_.end()) { // was indexed
                succ_[cur] = move(succ_temp_.at(cur));
                for (const auto &i : parent_.D1_.succ_temp_) {
                    if (succ_.at(cur).at(i.first) != INF8) {
                        resume_bfs(i.first, graph_.vertices(), graph_.edges(), graph_.gpu_vertices(), graph_.gpu_edges(), succ_.at(cur));
                    }
                }
                update_insert(s, t, graph_.vertices(), graph_.edges(), graph_.gpu_vertices(), graph_.gpu_edges(), succ_.at(cur));
                pred_[cur] = move(pred_temp_.at(cur));
                for (const auto &i : parent_.D1_.succ_temp_) {
                    if (pred_.at(cur).at(i.first) != INF8) {
                        resume_bfs(i.first, graph_.rvertices(), graph_.redges(), graph_.gpu_rvertices(), graph_.gpu_redges(), pred_.at(cur));
                    }
                }
                update_insert(t, s, graph_.rvertices(), graph_.redges(), graph_.gpu_rvertices(), graph_.gpu_redges(), pred_.at(cur));
            }
            else if (parent_.D1_.succ_temp_.find(cur) != parent_.D1_.succ_temp_.end()) { // was indexed in D1
                succ_[cur] = move(parent_.D1_.succ_temp_.at(cur));
                update_insert(s, t, graph_.vertices(), graph_.edges(), graph_.gpu_vertices(), graph_.gpu_edges(), succ_.at(cur));
                pred_[cur] = move(parent_.D1_.pred_temp_.at(cur));
                update_insert(t, s, graph_.rvertices(), graph_.redges(), graph_.gpu_rvertices(), graph_.gpu_redges(), pred_.at(cur));
            }
            else { // not indexed before
                succ_[cur] = vector<distance_t>(graph_.num_vertices(), INF8);
                construct_bfs(cur, graph_.vertices(), graph_.edges(), graph_.gpu_vertices(), graph_.gpu_edges(), succ_.at(cur));
                pred_[cur] = vector<distance_t>(graph_.num_vertices(), INF8);
                construct_bfs(cur, graph_.rvertices(), graph_.redges(), graph_.gpu_rvertices(), graph_.gpu_redges(), pred_.at(cur));
            }
        }
        succ_temp_.erase(cur);
        pred_temp_.erase(cur);
        update_cover(cur);
    }
    while (!quedeg_.empty()) {
        quedeg_.pop();
    }
}

void DynamicScalableGPUKReach::DynamicPartialIndex::remove_edge(vertex_t s, vertex_t t) {
    succ_temp_.clear();
    swap(succ_, succ_temp_);
    pred_temp_.clear();
    swap(pred_, pred_temp_);
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
        if (!isD2_) { // is D1
            if (succ_temp_.find(cur) != succ_temp_.end()) { // was indexed
                succ_[cur] = move(succ_temp_.at(cur));
                update_remove(cur, s, t, graph_.vertices(), graph_.edges(), graph_.gpu_vertices(), graph_.gpu_edges(), succ_.at(cur));
                pred_[cur] = move(pred_temp_.at(cur));
                update_remove(cur, t, s, graph_.rvertices(), graph_.redges(), graph_.gpu_rvertices(), graph_.gpu_redges(), pred_.at(cur));
            }
            else if (parent_.D2_.indexed(cur)) { // indexed in D2
                succ_[cur] = move(parent_.D2_.succ_.at(cur));
                for (const auto &i : succ_temp_) {
                    if (succ_.at(cur).at(i.first) != INF8) {
                        resume_bfs(i.first, graph_.vertices(), graph_.edges(), graph_.gpu_vertices(), graph_.gpu_edges(), succ_.at(cur));
                    }
                }
                update_remove(cur, s, t, graph_.vertices(), graph_.edges(), graph_.gpu_vertices(), graph_.gpu_edges(), succ_.at(cur));
                pred_[cur] = move(parent_.D2_.pred_.at(cur));
                for (const auto &i : succ_temp_) {
                    if (pred_.at(cur).at(i.first) != INF8) {
                        resume_bfs(i.first, graph_.rvertices(), graph_.redges(), graph_.gpu_rvertices(), graph_.gpu_redges(), pred_.at(cur));
                    }
                }
                update_remove(cur, t, s, graph_.rvertices(), graph_.redges(), graph_.gpu_rvertices(), graph_.gpu_redges(), pred_.at(cur));
            }
            else { // not indexed before
                succ_[cur] = vector<distance_t>(graph_.num_vertices(), INF8);
                construct_bfs(cur, graph_.vertices(), graph_.edges(), graph_.gpu_vertices(), graph_.gpu_edges(), succ_.at(cur));
                pred_[cur] = vector<distance_t>(graph_.num_vertices(), INF8);
                construct_bfs(cur, graph_.rvertices(), graph_.redges(), graph_.gpu_rvertices(), graph_.gpu_redges(), pred_.at(cur));
            }
        }
        else { // is D2
            if (succ_temp_.find(cur) != succ_temp_.end()) { // was indexed
                succ_[cur] = move(succ_temp_.at(cur));
                for (const auto &i : parent_.D1_.succ_temp_) {
                    if (succ_.at(cur).at(i.first) != INF8) {
                        resume_bfs(i.first, graph_.vertices(), graph_.edges(), graph_.gpu_vertices(), graph_.gpu_edges(), succ_.at(cur));
                    }
                }
                update_remove(cur, s, t, graph_.vertices(), graph_.edges(), graph_.gpu_vertices(), graph_.gpu_edges(), succ_.at(cur));
                pred_[cur] = move(pred_temp_.at(cur));
                for (const auto &i : parent_.D1_.succ_temp_) {
                    if (pred_.at(cur).at(i.first) != INF8) {
                        resume_bfs(i.first, graph_.rvertices(), graph_.redges(), graph_.gpu_rvertices(), graph_.gpu_redges(), pred_.at(cur));
                    }
                }
                update_remove(cur, t, s, graph_.rvertices(), graph_.redges(), graph_.gpu_rvertices(), graph_.gpu_redges(), pred_.at(cur));
            }
            else if (parent_.D1_.succ_temp_.find(cur) != parent_.D1_.succ_temp_.end()) { // was indexed in D1
                succ_[cur] = move(parent_.D1_.succ_temp_.at(cur));
                update_remove(cur, s, t, graph_.vertices(), graph_.edges(), graph_.gpu_vertices(), graph_.gpu_edges(), succ_.at(cur));
                pred_[cur] = move(parent_.D1_.pred_temp_.at(cur));
                update_remove(cur, t, s, graph_.rvertices(), graph_.redges(), graph_.gpu_rvertices(), graph_.gpu_redges(), pred_.at(cur));
            }
            else { // not indexed before
                succ_[cur] = vector<distance_t>(graph_.num_vertices(), INF8);
                construct_bfs(cur, graph_.vertices(), graph_.edges(), graph_.gpu_vertices(), graph_.gpu_edges(), succ_.at(cur));
                pred_[cur] = vector<distance_t>(graph_.num_vertices(), INF8);
                construct_bfs(cur, graph_.rvertices(), graph_.redges(), graph_.gpu_rvertices(), graph_.gpu_redges(), pred_.at(cur));
            }
        }
        succ_temp_.erase(cur);
        pred_temp_.erase(cur);
        update_cover(cur);
    }
    while (!quedeg_.empty()) {
        quedeg_.pop();
    }
}

void DynamicScalableGPUKReach::DynamicPartialIndex::remove_vertex(vertex_t v, const std::vector<vertex_t> &out,
                                                               const std::vector<vertex_t> &in) {
    succ_temp_.clear();
    swap(succ_, succ_temp_);
    pred_temp_.clear();
    swap(pred_, pred_temp_);
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
        if (!isD2_) { // is D1
            if (succ_temp_.find(cur) != succ_temp_.end()) { // was indexed
                succ_[cur] = move(succ_temp_.at(cur));
                update_remove(cur, v, out, graph_.vertices(), graph_.edges(), graph_.gpu_vertices(), graph_.gpu_edges(), succ_.at(cur));
                pred_[cur] = move(pred_temp_.at(cur));
                update_remove(cur, v, in, graph_.rvertices(), graph_.redges(), graph_.gpu_rvertices(), graph_.gpu_redges(), pred_.at(cur));
            }
            else if (parent_.D2_.indexed(cur)) { // indexed in D2
                succ_[cur] = move(parent_.D2_.succ_.at(cur));
                for (const auto &i : succ_temp_) {
                    if (succ_.at(cur).at(i.first) != INF8) {
                        resume_bfs(i.first, graph_.vertices(), graph_.edges(), graph_.gpu_vertices(), graph_.gpu_edges(), succ_.at(cur));
                    }
                }
                update_remove(cur, v, out, graph_.vertices(), graph_.edges(), graph_.gpu_vertices(), graph_.gpu_edges(), succ_.at(cur));
                pred_[cur] = move(parent_.D2_.pred_.at(cur));
                for (const auto &i : succ_temp_) {
                    if (pred_.at(cur).at(i.first) != INF8) {
                        resume_bfs(i.first, graph_.rvertices(), graph_.redges(), graph_.gpu_rvertices(), graph_.gpu_redges(), pred_.at(cur));
                    }
                }
                update_remove(cur, v, in, graph_.rvertices(), graph_.redges(), graph_.gpu_rvertices(), graph_.gpu_redges(), pred_.at(cur));
            }
            else { // not indexed before
                succ_[cur] = vector<distance_t>(graph_.num_vertices(), INF8);
                construct_bfs(cur, graph_.vertices(), graph_.edges(), graph_.gpu_vertices(), graph_.gpu_edges(), succ_.at(cur));
                pred_[cur] = vector<distance_t>(graph_.num_vertices(), INF8);
                construct_bfs(cur, graph_.rvertices(), graph_.redges(), graph_.gpu_rvertices(), graph_.gpu_redges(), pred_.at(cur));
            }
        }
        else { // is D2
            if (succ_temp_.find(cur) != succ_temp_.end()) { // was indexed
                succ_[cur] = move(succ_temp_.at(cur));
                for (const auto &i : parent_.D1_.succ_temp_) {
                    if (succ_.at(cur).at(i.first) != INF8) {
                        resume_bfs(i.first, graph_.vertices(), graph_.edges(), graph_.gpu_vertices(), graph_.gpu_edges(), succ_.at(cur));
                    }
                }
                update_remove(cur, v, out, graph_.vertices(), graph_.edges(), graph_.gpu_vertices(), graph_.gpu_edges(), succ_.at(cur));
                pred_[cur] = move(pred_temp_.at(cur));
                for (const auto &i : parent_.D1_.succ_temp_) {
                    if (pred_.at(cur).at(i.first) != INF8) {
                        resume_bfs(i.first, graph_.rvertices(), graph_.redges(), graph_.gpu_rvertices(), graph_.gpu_redges(), pred_.at(cur));
                    }
                }
                update_remove(cur, v, in, graph_.rvertices(), graph_.redges(), graph_.gpu_rvertices(), graph_.gpu_redges(), pred_.at(cur));
            }
            else if (parent_.D1_.succ_temp_.find(cur) != parent_.D1_.succ_temp_.end()) { // was indexed in D1
                succ_[cur] = move(parent_.D1_.succ_temp_.at(cur));
                update_remove(cur, v, out, graph_.vertices(), graph_.edges(), graph_.gpu_vertices(), graph_.gpu_edges(), succ_.at(cur));
                pred_[cur] = move(parent_.D1_.pred_temp_.at(cur));
                update_remove(cur, v, in, graph_.rvertices(), graph_.redges(), graph_.gpu_rvertices(), graph_.gpu_redges(), pred_.at(cur));
            }
            else { // not indexed before
                succ_[cur] = vector<distance_t>(graph_.num_vertices(), INF8);
                construct_bfs(cur, graph_.vertices(), graph_.edges(), graph_.gpu_vertices(), graph_.gpu_edges(), succ_.at(cur));
                pred_[cur] = vector<distance_t>(graph_.num_vertices(), INF8);
                construct_bfs(cur, graph_.rvertices(), graph_.redges(), graph_.gpu_rvertices(), graph_.gpu_redges(), pred_.at(cur));
            }
        }
        succ_temp_.erase(cur);
        pred_temp_.erase(cur);
        update_cover(cur);
    }
    while (!quedeg_.empty()) {
        quedeg_.pop();
    }
}

DynamicScalableGPUKReach::DynamicPartialIndex::DynamicPartialIndex(DynamicScalableGPUKReach &parent, const GPUGraph &graph,
                                                                distance_t k, uint32_t budget, bool isD2)
        : parent_(parent), graph_(graph), k_(k), budget_(budget), isD2_(isD2),
          quedeg_(parent.quedeg_), degree_(parent.degree_),
          queue_(parent.queue_), quedist_(parent.quedist_), updated_(parent.updated_), 
          d_distance_(parent.d_distance_), d_visited_bits_(parent.d_visited_bits_), d_d1_bits_(parent.d_d1_bits_), d_d1_(parent.d_d1_) {}


void DynamicScalableGPUKReach::DynamicPartialIndex::update_remove(vertex_t r, vertex_t s, vertex_t t,
            const std::vector<degree_t> &h_vertices, const std::vector<vertex_t> &h_edges,
            const degree_t* vertices, const vertex_t* edges, std::vector<distance_t> &dist) {
    if (dist.at(s) >= k_ || dist.at(t) > k_ || dist.at(s) + 1 != dist.at(t)) {
        return;
    }
    gpu_bfs(*this, r, h_vertices, h_edges, vertices, edges, dist);
}

void DynamicScalableGPUKReach::DynamicPartialIndex::update_remove(vertex_t r, vertex_t v, const std::vector<vertex_t> &out,
            const std::vector<degree_t> &h_vertices, const std::vector<vertex_t> &h_edges,
            const degree_t* vertices, const vertex_t* edges, std::vector<distance_t> &dist) {
    if (dist.at(v) > k_) {
        return;
    }
    gpu_bfs(*this, r, h_vertices, h_edges, vertices, edges, dist);
}

void DynamicScalableGPUKReach::DynamicPartialIndex::construct() {
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

bool DynamicScalableGPUKReach::DynamicPartialIndex::indexed(vertex_t v) const {
    return succ_.find(v) != succ_.end();
}

distance_t DynamicScalableGPUKReach::DynamicPartialIndex::distance(vertex_t s, vertex_t t) const {
    return indexed(s) ? succ_.at(s).at(t) : pred_.at(t).at(s);
}

bool DynamicScalableGPUKReach::DynamicPartialIndex::single_intermediate(vertex_t s, vertex_t t) const {
    for (const auto &i : succ_) {
        if (pred_.at(i.first).at(s) + succ_.at(i.first).at(t) <= k_) {
            return true;
        }
    }
    return false;
}

bool DynamicScalableGPUKReach::DynamicPartialIndex::double_intermediate(vertex_t s, vertex_t t) const {
    for (const auto &i : succ_) {
        for (const auto &j : succ_) {
            if (pred_.at(i.first).at(s) + succ_.at(i.first).at(j.first) + succ_.at(j.first).at(t) <= k_) {
                return true;
            }
        }
    }
    return false;
}

void DynamicScalableGPUKReach::DynamicPartialIndex::set_degree() {
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

void DynamicScalableGPUKReach::DynamicPartialIndex::cover(vertex_t v) {
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

void DynamicScalableGPUKReach::DynamicPartialIndex::update_cover(vertex_t v) {
    for (vertex_t j = 0; j < graph_.num_vertices(); ++j) {
        if ((succ_.at(v).at(j) != INF8 || pred_.at(v).at(j) != INF8) &&
            degree_.at(j) > 0) {
            cover(j);
        }
    }
}

void
DynamicScalableGPUKReach::DynamicPartialIndex::construct_bfs(vertex_t s, const std::vector<degree_t> &h_vertices, const std::vector<vertex_t> &h_edges,
        const degree_t* vertices, const vertex_t* edges, std::vector<distance_t> &dist) {
    gpu_bfs(*this, s, h_vertices, h_edges, vertices, edges, dist);
}

void DynamicScalableGPUKReach::DynamicPartialIndex::insert_vertex(vertex_t v, const std::vector<vertex_t> &out,
                                                               const std::vector<vertex_t> &in) {
    succ_temp_.clear();
    swap(succ_, succ_temp_);
    pred_temp_.clear();
    swap(pred_, pred_temp_);
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
        if (!isD2_) { // is D1
            if (succ_temp_.find(cur) != succ_temp_.end()) { // was indexed
                succ_[cur] = move(succ_temp_.at(cur));
                update_insert(v, out, in, graph_.vertices(), graph_.edges(), graph_.gpu_vertices(), graph_.gpu_edges(), succ_.at(cur));
                pred_[cur] = move(pred_temp_.at(cur));
                update_insert(v, in, out, graph_.rvertices(), graph_.redges(), graph_.gpu_rvertices(), graph_.gpu_redges(), pred_.at(cur));
            }
            else if (parent_.D2_.indexed(cur)) { // indexed in D2
                succ_[cur] = move(parent_.D2_.succ_.at(cur));
                for (const auto &i : succ_temp_) {
                    if (succ_.at(cur).at(i.first) != INF8) {
                        resume_bfs(i.first, graph_.vertices(), graph_.edges(), graph_.gpu_vertices(), graph_.gpu_edges(), succ_.at(cur));
                    }
                }
                update_insert(v, out, in, graph_.vertices(), graph_.edges(), graph_.gpu_vertices(), graph_.gpu_edges(), succ_.at(cur));
                pred_[cur] = move(parent_.D2_.pred_.at(cur));
                for (const auto &i : succ_temp_) {
                    if (pred_.at(cur).at(i.first) != INF8) {
                        resume_bfs(i.first, graph_.rvertices(), graph_.redges(), graph_.gpu_rvertices(), graph_.gpu_redges(), pred_.at(cur));
                    }
                }
                update_insert(v, in, out, graph_.rvertices(), graph_.redges(), graph_.gpu_rvertices(), graph_.gpu_redges(), pred_.at(cur));
            }
            else { // not indexed before
                succ_[cur] = vector<distance_t>(graph_.num_vertices(), INF8);
                construct_bfs(cur, graph_.vertices(), graph_.edges(), graph_.gpu_vertices(), graph_.gpu_edges(), succ_.at(cur));
                pred_[cur] = vector<distance_t>(graph_.num_vertices(), INF8);
                construct_bfs(cur, graph_.rvertices(), graph_.redges(), graph_.gpu_rvertices(), graph_.gpu_redges(), pred_.at(cur));
            }
        }
        else { // is D2
            if (succ_temp_.find(cur) != succ_temp_.end()) { // was indexed
                succ_[cur] = move(succ_temp_.at(cur));
                for (const auto &i : parent_.D1_.succ_temp_) {
                    if (succ_.at(cur).at(i.first) != INF8) {
                        resume_bfs(i.first, graph_.vertices(), graph_.edges(), graph_.gpu_vertices(), graph_.gpu_edges(), succ_.at(cur));
                    }
                }
                update_insert(v, out, in, graph_.vertices(), graph_.edges(), graph_.gpu_vertices(), graph_.gpu_edges(), succ_.at(cur));
                pred_[cur] = move(pred_temp_.at(cur));
                for (const auto &i : parent_.D1_.succ_temp_) {
                    if (pred_.at(cur).at(i.first) != INF8) {
                        resume_bfs(i.first, graph_.rvertices(), graph_.redges(), graph_.gpu_rvertices(), graph_.gpu_redges(), pred_.at(cur));
                    }
                }
                update_insert(v, in, out, graph_.rvertices(), graph_.redges(), graph_.gpu_rvertices(), graph_.gpu_redges(), pred_.at(cur));
            }
            else if (parent_.D1_.succ_temp_.find(cur) != parent_.D1_.succ_temp_.end()) { // was indexed in D1
                succ_[cur] = move(parent_.D1_.succ_temp_.at(cur));
                update_insert(v, out, in, graph_.vertices(), graph_.edges(), graph_.gpu_vertices(), graph_.gpu_edges(), succ_.at(cur));
                pred_[cur] = move(parent_.D1_.pred_temp_.at(cur));
                update_insert(v, in, out, graph_.rvertices(), graph_.redges(), graph_.gpu_rvertices(), graph_.gpu_redges(), pred_.at(cur));
            }
            else { // not indexed before
                succ_[cur] = vector<distance_t>(graph_.num_vertices(), INF8);
                construct_bfs(cur, graph_.vertices(), graph_.edges(), graph_.gpu_vertices(), graph_.gpu_edges(), succ_.at(cur));
                pred_[cur] = vector<distance_t>(graph_.num_vertices(), INF8);
                construct_bfs(cur, graph_.rvertices(), graph_.redges(), graph_.gpu_rvertices(), graph_.gpu_redges(), pred_.at(cur));
            }
        }
        succ_temp_.erase(cur);
        pred_temp_.erase(cur);
        update_cover(cur);
    }
    while (!quedeg_.empty()) {
        quedeg_.pop();
    }
}

void DynamicScalableGPUKReach::DynamicPartialIndex::update_insert(vertex_t v, const std::vector<vertex_t> &out,
            const std::vector<vertex_t> &in,
            const std::vector<degree_t> &h_vertices, const std::vector<vertex_t> &h_edges,
            const degree_t* vertices, const vertex_t* edges, std::vector<distance_t> &dist) {
    auto d = dist.at(v);
    for (const auto &s : in){
        if (dist.at(s) < k_ && dist.at(s) + 1 < dist.at(v)){
            dist.at(v) = dist.at(s) + 1;
        }
    }

    if (dist.at(v) >= k_){
        return;
    }

    if (dist.at(v) < d){
        resume_bfs(v, h_vertices, h_edges, vertices, edges, dist);
    }
    else {
        for (const auto &t : out) {
            if (dist.at(v) + 1 < dist.at(t)){
                dist.at(t) = dist.at(v) + 1;
                resume_bfs(t, h_vertices, h_edges, vertices, edges, dist);
            }
        }
    }
}


void DynamicScalableGPUKReach::insert_edge(vertex_t s, vertex_t t) {
    D1_.insert_edge(s, t);
    D2_.insert_edge(s, t);
}

void DynamicScalableGPUKReach::remove_edge(vertex_t s, vertex_t t) {
    D1_.remove_edge(s, t);
    D2_.remove_edge(s, t);
}

void
DynamicScalableGPUKReach::remove_vertex(vertex_t v, const std::vector<vertex_t> &out, const std::vector<vertex_t> &in) {
    D1_.remove_vertex(v, out, in);
    D2_.remove_vertex(v, out, in);
}

DynamicScalableGPUKReach::DynamicScalableGPUKReach(const GPUGraph &graph, distance_t k, uint32_t b1, uint32_t b2)
        : graph_(graph), k_(k), D1_(*this, graph, k, b1), D2_(*this, graph, k, b2, true),
          degree_(graph.num_vertices()), distance_(graph.num_vertices(), INF8), queue_(graph.num_vertices()),
          d_distance_(graph.num_vertices(), mgpu_context), d_visited_bits_(mgpu::div_up(graph.num_vertices(), 32), mgpu_context), 
          d_d1_bits_(mgpu::div_up(graph.num_vertices(), 32), mgpu_context), d_d1_(graph.num_vertices(), mgpu_context) {
    std::vector<std::pair<degree_t, vertex_t>> quedeg_temp_;
    quedeg_temp_.reserve(graph.num_vertices());
    DegreeQueue(less<pair<degree_t, vertex_t>>(), move(quedeg_temp_)).swap(quedeg_);
    updated_.reserve(graph.num_vertices());
    std::vector<std::pair<distance_t, vertex_t>> quedist_temp_;
    quedist_temp_.reserve(graph.num_vertices());
    DistanceQueue(greater<pair<distance_t, vertex_t>>(), move(quedist_temp_)).swap(quedist_);
}

void DynamicScalableGPUKReach::construct() {
    D1_.construct();
    D2_.construct();
}

bool DynamicScalableGPUKReach::query(vertex_t s, vertex_t t) const {
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

bool DynamicScalableGPUKReach::bfs(vertex_t s, vertex_t t) const {
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

void
DynamicScalableGPUKReach::insert_vertex(vertex_t v, const std::vector<vertex_t> &out, const std::vector<vertex_t> &in) {
    D1_.insert_vertex(v, out, in);
    D2_.insert_vertex(v, out, in);
}
