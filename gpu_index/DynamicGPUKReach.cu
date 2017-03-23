#include "DynamicGPUKReach.h"

__device__ char atomicMinChar(char* address, char val) {
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

void gpu_bfs(const DynamicGPUKReach &kReach, vertex_t s, std::vector<distance_t> &dist) {
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

void gpu_resume_bfs(const DynamicGPUKReach &kReach, vertex_t s, std::vector<distance_t> &dist) {
    distance_t* distance_data = kReach.d_distance_.data();
    int* visited_bits_data = kReach.d_visited_bits_.data();
    const degree_t* vertices = kReach.graph_.gpu_vertices();
    const vertex_t* edges = kReach.graph_.gpu_edges();

    mgpu::htod(distance_data, dist.data(), dist.size());
    cudaMemset(visited_bits_data, 0, kReach.d_visited_bits_.size() * sizeof(int));

    mgpu::transform([=]MGPU_DEVICE(int index) {
        if (distance_data[index] < 0x3f) {
            atomicOr(visited_bits_data + index / 32, 1 << (31 & index));
        }
    }, dist.size(), mgpu_context);

  workload_t wl;
  wl.count = kReach.graph_.successors(s).size();
  wl.num_segments = 1;
  vector<int> edge_indices_host = { (int) kReach.graph_.vertices().at(s) };
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
            count = vertices[neighbor + 1] - vertices[neighbor];
            distance_data[neighbor] = cur_level + 1;
        }
        else if (cur_level + 1 < atomicMinChar((char*)distance_data + neighbor, cur_level + 1)){
            count = vertices[neighbor + 1] - vertices[neighbor];
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

DynamicGPUKReach::DynamicGPUKReach(const GPUGraph &graph, distance_t k)
        : graph_(graph), k_(k), degree_(graph.num_vertices()), queue_(graph.num_vertices()), 
          d_distance_(graph.num_vertices(), mgpu_context), d_visited_bits_(mgpu::div_up(graph.num_vertices(), 32), mgpu_context){
    vector<pair<degree_t, vertex_t>> quedeg_temp_;
    quedeg_temp_.reserve(graph.num_vertices());
    DegreeQueue(less<pair<degree_t, vertex_t>>(), move(quedeg_temp_)).swap(quedeg_);
    vector<pair<distance_t, vertex_t>> quedist_temp_;
    quedist_temp_.reserve(graph.num_vertices());
    DistanceQueue(greater<pair<distance_t, vertex_t>>(), move(quedist_temp_)).swap(quedist_);
}

void DynamicGPUKReach::construct() {
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

bool DynamicGPUKReach::query(vertex_t s, vertex_t t) const {
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

bool DynamicGPUKReach::indexed(vertex_t v) const {
    return index_.find(v) != index_.end();
}

distance_t DynamicGPUKReach::distance(vertex_t s, vertex_t t) const {
    return index_.at(s).at(t);
}

void DynamicGPUKReach::set_degree() {
    copy(graph_.degree().begin(), graph_.degree().end(), degree_.begin());
}

void DynamicGPUKReach::cover(vertex_t v) {
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

void DynamicGPUKReach::construct_bfs(vertex_t s, std::vector<distance_t> &dist) {
    gpu_bfs(*this, s, dist);
}

void DynamicGPUKReach::insert_edge(vertex_t s, vertex_t t) {
    index_temp_.clear();
    swap(index_, index_temp_);
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
        if (index_temp_.find(cur) != index_temp_.end()){
            index_[cur] = move(index_temp_.at(cur));
            update_insert(s, t, index_.at(cur));
        }
        else {
            index_[cur] = vector<distance_t>(graph_.num_vertices(), INF8);
            construct_bfs(cur, index_.at(cur));
        }
        index_temp_.erase(cur);
        cover(cur);
    }
}

void DynamicGPUKReach::update_insert(vertex_t s, vertex_t t, std::vector<distance_t> &dist) {
    if (dist.at(s) >= k_ || dist.at(t) <= dist.at(s) + 1) {
        return;
    }
    dist.at(t) = dist.at(s) + 1;
    resume_bfs(t, dist);
}

void DynamicGPUKReach::resume_bfs(vertex_t s, std::vector<distance_t> &dist) {  
    gpu_resume_bfs(*this, s, dist);
}

void DynamicGPUKReach::insert_vertex(vertex_t v, const std::vector<vertex_t> &out, const std::vector<vertex_t> &in) {
    index_temp_.clear();
    swap(index_, index_temp_);
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
        if (index_temp_.find(cur) != index_temp_.end()){
            index_[cur] = move(index_temp_.at(cur));
            update_insert(v, out, in, index_.at(cur));
        }
        else {
            index_[cur] = vector<distance_t>(graph_.num_vertices(), INF8);
            construct_bfs(cur, index_.at(cur));
        }
        index_temp_.erase(cur);
        cover(cur);
    }
}

void DynamicGPUKReach::update_insert(vertex_t v, const std::vector<vertex_t> &out, const std::vector<vertex_t> &in,
                                  std::vector<distance_t> &dist) {
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
        resume_bfs(v, dist);
    }
    else {
        for (const auto &t : out) {
            if (dist.at(v) + 1 < dist.at(t)){
                dist.at(t) = dist.at(v) + 1;
                resume_bfs(t, dist);
            }
        }
    }
}

void DynamicGPUKReach::remove_edge(vertex_t s, vertex_t t) {
    index_temp_.clear();
    swap(index_, index_temp_);
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
        if (index_temp_.find(cur) != index_temp_.end()){
            index_[cur] = move(index_temp_.at(cur));
            update_remove(cur, s, t, index_.at(cur));
        }
        else {
            index_[cur] = vector<distance_t>(graph_.num_vertices(), INF8);
            construct_bfs(cur, index_.at(cur));
        }
        index_temp_.erase(cur);
        cover(cur);
    }
}

void DynamicGPUKReach::update_remove(vertex_t r, vertex_t s, vertex_t t, std::vector<distance_t> &dist) {
    if (dist.at(s) >= k_ || dist.at(t) > k_ || dist.at(s) + 1 != dist.at(t)) {
        return;
    }
    gpu_bfs(*this, r, dist);
}

void DynamicGPUKReach::remove_vertex(vertex_t v, const std::vector<vertex_t> &out, const std::vector<vertex_t> &in) {
    (void) in;
    index_temp_.clear();
    swap(index_, index_temp_);
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
        if (index_temp_.find(cur) != index_temp_.end()){
            index_[cur] = move(index_temp_.at(cur));
            update_remove(cur, v, out, index_.at(cur));
        }
        else {
            index_[cur] = vector<distance_t>(graph_.num_vertices(), INF8);
            construct_bfs(cur, index_.at(cur));
        }
        index_temp_.erase(cur);
        cover(cur);
    }
}

void DynamicGPUKReach::update_remove(vertex_t r, vertex_t v, const std::vector<vertex_t> &out, std::vector<distance_t> &dist) {
    if (dist.at(v) > k_) {
        return;
    }    
    gpu_bfs(*this, r, dist);
}