#include "KReach.h"

using namespace std;

KReach::KReach(const Graph &graph, distance_t k)
        : graph_(graph), k_(k), degree_(graph.num_vertices()), queue_(graph.num_vertices()){
    vector<pair<degree_t, vertex_t>> quedeg_temp_;
    quedeg_temp_.reserve(graph.num_vertices());
    DegreeQueue(less<pair<degree_t, vertex_t>>(), move(quedeg_temp_)).swap(quedeg_);
}

void KReach::construct() {
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

void KReach::cover(vertex_t v) {
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

void KReach::set_degree() {
    copy(graph_.degree().begin(), graph_.degree().end(), degree_.begin());
}

void KReach::construct_bfs(vertex_t s, std::vector<distance_t> &dist) {
    back_ = 0;
    front_ = 0;
    dist.at(s) = 0;
    queue_.at(back_++) = s;
    while (back_ != front_) {
        auto cur = queue_.at(front_++);
        if (dist.at(cur) >= k_) {
            continue;
        }
        for (const auto &nxt : graph_.successors(cur)) {
            if (dist.at(nxt) != INF8) {
                continue;
            }
            dist.at(nxt) = dist.at(cur) + 1;
            if (graph_.successors(nxt).size() > 0) {
                queue_.at(back_++) = nxt;
            }
        }
    }
}

distance_t KReach::distance(vertex_t s, vertex_t t) const {
    return index_.at(s).at(t);
}

bool KReach::indexed(vertex_t v) const {
    return index_.find(v) != index_.end();
}

bool KReach::query(vertex_t s, vertex_t t) const {
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

void KReach::insert_edge(vertex_t s, vertex_t t) {
    (void) s;
    (void) t;
    construct();
}

void KReach::remove_edge(vertex_t s, vertex_t t) {
    (void) s;
    (void) t;
    construct();
}

void KReach::remove_vertex(vertex_t v, const std::vector<vertex_t> &out, const std::vector<vertex_t> &in) {
    (void) v;
    (void) out;
    (void) in;
    construct();
}

void KReach::insert_vertex(vertex_t v, const std::vector<vertex_t> &out, const std::vector<vertex_t> &in) {
    (void) v;
    (void) out;
    (void) in;
    construct();
}


