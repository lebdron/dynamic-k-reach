#include "GPUKReach.h"

using namespace std;

void gpu_bfs(const GPUKReach &kReach, vertex_t s, std::vector<distance_t> &dist) {
    distance_t* distance_data = kReach.d_distance_.data();
    int* visited_bits_data = kReach.d_visited_bits_.data();
    const degree_t* vertices = kReach.graph_.gpu_vertices();
    const vertex_t* edges = kReach.graph_.gpu_edges();

    cudaMemset(distance_data, 0x3f, kReach.d_distance_.size() * sizeof(distance_t));
    cudaMemset(visited_bits_data, 0, kReach.d_visited_bits_.size() * sizeof(int));

    mgpu::transform([=]MGPU_DEVICE(int index) {
    visited_bits_data[s / 32] = 1 << (31 & s);
    distance_data[s] = 0;
    }, 1, mgpu_context);

  workload_t wl;
  wl.count = kReach.graph_.successors(s).size();
  wl.num_segments = 1;
  vector<int> edge_indices_host = { (int) kReach.graph_.vertices().at(s) };
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
            count = vertices[neighbor + 1] - vertices[neighbor];
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

GPUKReach::GPUKReach(const GPUGraph &graph, distance_t k)
        : graph_(graph), k_(k), degree_(graph.num_vertices()), queue_(graph.num_vertices()), 
          d_distance_(graph.num_vertices(), mgpu_context), d_visited_bits_(mgpu::div_up(graph.num_vertices(), 32), mgpu_context){
    vector<pair<degree_t, vertex_t>> quedeg_temp_;
    quedeg_temp_.reserve(graph.num_vertices());
    DegreeQueue(less<pair<degree_t, vertex_t>>(), move(quedeg_temp_)).swap(quedeg_);
}

void GPUKReach::construct() {
    index_.clear();
    set_degree();
    for (vertex_t i = 0; i < graph_.num_vertices(); ++i){
        quedeg_.emplace(degree_.at(i), i);
    }
    while (!quedeg_.empty()){
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
        index_[cur] = vector<distance_t>(graph_.num_vertices(), INF8);
        construct_bfs(cur, index_.at(cur));
        cover(cur);
    }
}

void GPUKReach::cover(vertex_t v) {
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

void GPUKReach::set_degree() {
    copy(graph_.degree().begin(), graph_.degree().end(), degree_.begin());
}

void GPUKReach::construct_bfs(vertex_t s, std::vector<distance_t> &dist) {
    gpu_bfs(*this, s, dist);
}

distance_t GPUKReach::distance(vertex_t s, vertex_t t) const {
    return index_.at(s).at(t);
}

bool GPUKReach::indexed(vertex_t v) const {
    return index_.find(v) != index_.end();
}

bool GPUKReach::query(vertex_t s, vertex_t t) const {
    if (s == t && s < graph_.num_vertices()){
        return true;
    }
    if (indexed(s) && indexed(t)){
        return distance(s, t) <= k_;
    }
    else if (indexed(s) && !indexed(t)){
        for (const auto &v : graph_.predecessors(t)){
            if (distance(s, v) + 1 <= k_){
                return true;
            }
        }
    }
    else if (!indexed(s) && indexed(t)){
        for (const auto &v : graph_.successors(s)){
            if (distance(v, t) + 1 <= k_){
                return true;
            }
        }
    }
    else {
        for (const auto &u : graph_.successors(s)){
            for (const auto &v : graph_.predecessors(t)){
                if (distance(u, v) + 2 <= k_){
                    return true;
                }
            }
        }
    }
    return false;
}

void GPUKReach::insert_edge(vertex_t s, vertex_t t) {
    (void) s;
    (void) t;
    construct();
}

void GPUKReach::remove_edge(vertex_t s, vertex_t t) {
    (void) s;
    (void) t;
    construct();
}

void GPUKReach::remove_vertex(vertex_t v, const std::vector<vertex_t> &out, const std::vector<vertex_t> &in) {
    (void) v;
    (void) out;
    (void) in;
    construct();
}

void GPUKReach::insert_vertex(vertex_t v, const std::vector<vertex_t> &out, const std::vector<vertex_t> &in) {
    (void) v;
    (void) out;
    (void) in;
    construct();
}


