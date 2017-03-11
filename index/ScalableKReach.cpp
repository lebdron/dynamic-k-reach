#include "ScalableKReach.h"

using namespace std;

ScalableKReach::PartialIndex::PartialIndex(ScalableKReach &parent,
                                           const Graph &graph, distance_t k, uint32_t budget, bool isD2)
        : parent_(parent), graph_(graph), k_(k), budget_(budget), isD2_(isD2),
          quedeg_(parent.quedeg_), degree_(parent.degree_), queue_(parent.queue_) {}

void ScalableKReach::PartialIndex::construct() {
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
        construct_bfs(cur, graph_.graph(), succ_.at(cur));
        pred_[cur] = vector<distance_t>(graph_.num_vertices(), INF8);
        construct_bfs(cur, graph_.reverse_graph(), pred_.at(cur));
        update_cover(cur);
    }
    while (!quedeg_.empty()) {
        quedeg_.pop();
    }
}

void ScalableKReach::PartialIndex::set_degree() {
    copy(graph_.degree().begin(), graph_.degree().end(), degree_.begin());
    if (isD2_) {
        for (const auto &i : parent_.D1_.succ_) {
            cover(i.first);
        }
    }
}

void ScalableKReach::PartialIndex::cover(vertex_t v) {
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

void ScalableKReach::PartialIndex::update_cover(vertex_t v) {
    for (vertex_t j = 0; j < graph_.num_vertices(); ++j) {
        if ((succ_.at(v).at(j) != INF8 || pred_.at(v).at(j) != INF8) &&
            degree_.at(j) > 0) {
            cover(j);
        }
    }
}

void ScalableKReach::PartialIndex::construct_bfs(vertex_t s, const std::vector<std::vector<vertex_t>> &graph,
                                                 std::vector<distance_t> &dist) {
    back_ = 0;
    front_ = 0;
    dist.at(s) = 0;
    queue_.at(back_++) = s;
    while (back_ != front_) {
        auto cur = queue_.at(front_++);
        if (dist.at(cur) >= k_) {
            continue;
        }
        for (const auto &nxt : graph.at(cur)) {
            if (dist.at(nxt) != INF8) {
                continue;
            }
            dist.at(nxt) = dist.at(cur) + 1;
            if (!isD2_ || !parent_.D1_.indexed(nxt)) {
                queue_.at(back_++) = nxt;
            }
        }
    }
}

bool ScalableKReach::PartialIndex::indexed(vertex_t v) const {
    return succ_.find(v) != succ_.end();
}

distance_t ScalableKReach::PartialIndex::distance(vertex_t s, vertex_t t) const {
    return indexed(s) ? succ_.at(s).at(t) : pred_.at(t).at(s);
}

bool ScalableKReach::PartialIndex::single_intermediate(vertex_t s, vertex_t t) const {
    for (const auto &i : succ_) {
        if (pred_.at(i.first).at(s) + succ_.at(i.first).at(t) <= k_) {
            return true;
        }
    }
    return false;
}

bool ScalableKReach::PartialIndex::double_intermediate(vertex_t s, vertex_t t) const {
    for (const auto &i : succ_) {
        for (const auto &j : succ_) {
            if (pred_.at(i.first).at(s) + succ_.at(i.first).at(j.first) + succ_.at(j.first).at(t) <= k_) {
                return true;
            }
        }
    }
    return false;
}

void ScalableKReach::PartialIndex::insert_edge(vertex_t s, vertex_t t) {
    (void) s;
    (void) t;
    construct();
}

void ScalableKReach::PartialIndex::remove_edge(vertex_t s, vertex_t t) {
    (void) s;
    (void) t;
    construct();
}

void ScalableKReach::PartialIndex::remove_vertex(vertex_t v, const std::vector<vertex_t> &out,
                                                 const std::vector<vertex_t> &in) {
    (void) v;
    (void) out;
    (void) in;
    construct();
}

void ScalableKReach::PartialIndex::insert_vertex(vertex_t v, const std::vector<vertex_t> &out,
                                                 const std::vector<vertex_t> &in) {
    (void) v;
    (void) out;
    (void) in;
    construct();
}


ScalableKReach::ScalableKReach(const Graph &graph, distance_t k, uint32_t b1, uint32_t b2)
        : graph_(graph), k_(k), D1_(*this, graph, k, b1), D2_(*this, graph, k, b2, true),
          degree_(graph.num_vertices()), distance_(graph.num_vertices(), INF8), queue_(graph.num_vertices()) {
    vector<pair<degree_t, vertex_t>> quedeg_temp_;
    quedeg_temp_.reserve(graph.num_vertices());
    DegreeQueue(less<pair<degree_t, vertex_t>>(), move(quedeg_temp_)).swap(quedeg_);
}

void ScalableKReach::construct() {
    D1_.construct();
    D2_.construct();
}

void ScalableKReach::insert_edge(vertex_t s, vertex_t t) {
    (void) s;
    (void) t;
    construct();
}

void ScalableKReach::remove_edge(vertex_t s, vertex_t t) {
    (void) s;
    (void) t;
    construct();
}

void ScalableKReach::remove_vertex(vertex_t v, const std::vector<vertex_t> &out, const std::vector<vertex_t> &in) {
    (void) v;
    (void) out;
    (void) in;
    construct();
}

bool ScalableKReach::query(vertex_t s, vertex_t t) const {
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

bool ScalableKReach::bfs(vertex_t s, vertex_t t) const {
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

void ScalableKReach::insert_vertex(vertex_t v, const std::vector<vertex_t> &out, const std::vector<vertex_t> &in) {
    (void) v;
    (void) out;
    (void) in;
    construct();
}


