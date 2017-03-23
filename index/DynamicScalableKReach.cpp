#include "DynamicScalableKReach.h"

using namespace std;


void DynamicScalableKReach::DynamicPartialIndex::resume_bfs(vertex_t s, const std::vector<std::vector<vertex_t>> &graph,
                                                            std::vector<distance_t> &dist) {
    back_ = 0;
    front_ = 0;
    assert(dist.at(s) != INF8);
    queue_.at(back_++) = s;
    while (back_ != front_) {
        auto cur = queue_.at(front_++);
        if (dist.at(cur) >= k_) {
            continue;
        }
        for (const auto &nxt : graph.at(cur)) {
            if (dist.at(nxt) <= dist.at(cur) + 1) {
                continue;
            }
            dist.at(nxt) = dist.at(cur) + 1;
            if (!isD2_ || !parent_.D1_.indexed(nxt)) {
                queue_.at(back_++) = nxt;
            }
        }
    }
}

void DynamicScalableKReach::DynamicPartialIndex::update_insert(vertex_t s, vertex_t t,
                                                               const std::vector<std::vector<vertex_t>> &graph,
                                                               std::vector<distance_t> &dist) {
    if (dist.at(s) >= k_ || dist.at(t) <= dist.at(s) + 1) {
        return;
    }
    dist.at(t) = dist.at(s) + 1;
    resume_bfs(t, graph, dist);
}

void DynamicScalableKReach::DynamicPartialIndex::insert_edge(vertex_t s, vertex_t t) {
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
                update_insert(s, t, graph_.graph(), succ_.at(cur));
                pred_[cur] = move(pred_temp_.at(cur));
                update_insert(t, s, graph_.reverse_graph(), pred_.at(cur));
            }
            else if (parent_.D2_.indexed(cur)) { // indexed in D2
                succ_[cur] = move(parent_.D2_.succ_.at(cur));
                for (const auto &i : succ_temp_) {
                    if (succ_.at(cur).at(i.first) != INF8) {
                        resume_bfs(i.first, graph_.graph(), succ_.at(cur));
                    }
                }
                update_insert(s, t, graph_.graph(), succ_.at(cur));
                pred_[cur] = move(parent_.D2_.pred_.at(cur));
                for (const auto &i : succ_temp_) {
                    if (pred_.at(cur).at(i.first) != INF8) {
                        resume_bfs(i.first, graph_.reverse_graph(), pred_.at(cur));
                    }
                }
                update_insert(t, s, graph_.reverse_graph(), pred_.at(cur));
            }
            else { // not indexed before
                succ_[cur] = vector<distance_t>(graph_.num_vertices(), INF8);
                construct_bfs(cur, graph_.graph(), succ_.at(cur));
                pred_[cur] = vector<distance_t>(graph_.num_vertices(), INF8);
                construct_bfs(cur, graph_.reverse_graph(), pred_.at(cur));
            }
        }
        else { // is D2
            if (succ_temp_.find(cur) != succ_temp_.end()) { // was indexed
                succ_[cur] = move(succ_temp_.at(cur));
                for (const auto &i : parent_.D1_.succ_temp_) {
                    if (succ_.at(cur).at(i.first) != INF8) {
                        resume_bfs(i.first, graph_.graph(), succ_.at(cur));
                    }
                }
                update_insert(s, t, graph_.graph(), succ_.at(cur));
                pred_[cur] = move(pred_temp_.at(cur));
                for (const auto &i : parent_.D1_.succ_temp_) {
                    if (pred_.at(cur).at(i.first) != INF8) {
                        resume_bfs(i.first, graph_.reverse_graph(), pred_.at(cur));
                    }
                }
                update_insert(t, s, graph_.reverse_graph(), pred_.at(cur));
            }
            else if (parent_.D1_.succ_temp_.find(cur) != parent_.D1_.succ_temp_.end()) { // was indexed in D1
                succ_[cur] = move(parent_.D1_.succ_temp_.at(cur));
                update_insert(s, t, graph_.graph(), succ_.at(cur));
                pred_[cur] = move(parent_.D1_.pred_temp_.at(cur));
                update_insert(t, s, graph_.reverse_graph(), pred_.at(cur));
            }
            else { // not indexed before
                succ_[cur] = vector<distance_t>(graph_.num_vertices(), INF8);
                construct_bfs(cur, graph_.graph(), succ_.at(cur));
                pred_[cur] = vector<distance_t>(graph_.num_vertices(), INF8);
                construct_bfs(cur, graph_.reverse_graph(), pred_.at(cur));
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

void DynamicScalableKReach::DynamicPartialIndex::remove_edge(vertex_t s, vertex_t t) {
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
                update_remove(s, t, graph_.graph(), graph_.reverse_graph(), succ_.at(cur));
                pred_[cur] = move(pred_temp_.at(cur));
                update_remove(t, s, graph_.reverse_graph(), graph_.graph(), pred_.at(cur));
            }
            else if (parent_.D2_.indexed(cur)) { // indexed in D2
                succ_[cur] = move(parent_.D2_.succ_.at(cur));
                for (const auto &i : succ_temp_) {
                    if (succ_.at(cur).at(i.first) != INF8) {
                        resume_bfs(i.first, graph_.graph(), succ_.at(cur));
                    }
                }
                update_remove(s, t, graph_.graph(), graph_.reverse_graph(), succ_.at(cur));
                pred_[cur] = move(parent_.D2_.pred_.at(cur));
                for (const auto &i : succ_temp_) {
                    if (pred_.at(cur).at(i.first) != INF8) {
                        resume_bfs(i.first, graph_.reverse_graph(), pred_.at(cur));
                    }
                }
                update_remove(t, s, graph_.reverse_graph(), graph_.graph(), pred_.at(cur));
            }
            else { // not indexed before
                succ_[cur] = vector<distance_t>(graph_.num_vertices(), INF8);
                construct_bfs(cur, graph_.graph(), succ_.at(cur));
                pred_[cur] = vector<distance_t>(graph_.num_vertices(), INF8);
                construct_bfs(cur, graph_.reverse_graph(), pred_.at(cur));
            }
        }
        else { // is D2
            if (succ_temp_.find(cur) != succ_temp_.end()) { // was indexed
                succ_[cur] = move(succ_temp_.at(cur));
                for (const auto &i : parent_.D1_.succ_temp_) {
                    if (succ_.at(cur).at(i.first) != INF8) {
                        resume_bfs(i.first, graph_.graph(), succ_.at(cur));
                    }
                }
                update_remove(s, t, graph_.graph(), graph_.reverse_graph(), succ_.at(cur));
                pred_[cur] = move(pred_temp_.at(cur));
                for (const auto &i : parent_.D1_.succ_temp_) {
                    if (pred_.at(cur).at(i.first) != INF8) {
                        resume_bfs(i.first, graph_.reverse_graph(), pred_.at(cur));
                    }
                }
                update_remove(t, s, graph_.reverse_graph(), graph_.graph(), pred_.at(cur));
            }
            else if (parent_.D1_.succ_temp_.find(cur) != parent_.D1_.succ_temp_.end()) { // was indexed in D1
                succ_[cur] = move(parent_.D1_.succ_temp_.at(cur));
                update_remove(s, t, graph_.graph(), graph_.reverse_graph(), succ_.at(cur));
                pred_[cur] = move(parent_.D1_.pred_temp_.at(cur));
                update_remove(t, s, graph_.reverse_graph(), graph_.graph(), pred_.at(cur));
            }
            else { // not indexed before
                succ_[cur] = vector<distance_t>(graph_.num_vertices(), INF8);
                construct_bfs(cur, graph_.graph(), succ_.at(cur));
                pred_[cur] = vector<distance_t>(graph_.num_vertices(), INF8);
                construct_bfs(cur, graph_.reverse_graph(), pred_.at(cur));
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

void DynamicScalableKReach::DynamicPartialIndex::remove_vertex(vertex_t v, const std::vector<vertex_t> &out,
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
                update_remove(v, out, graph_.graph(), graph_.reverse_graph(), succ_.at(cur));
                pred_[cur] = move(pred_temp_.at(cur));
                update_remove(v, in, graph_.reverse_graph(), graph_.graph(), pred_.at(cur));
            }
            else if (parent_.D2_.indexed(cur)) { // indexed in D2
                succ_[cur] = move(parent_.D2_.succ_.at(cur));
                for (const auto &i : succ_temp_) {
                    if (succ_.at(cur).at(i.first) != INF8) {
                        resume_bfs(i.first, graph_.graph(), succ_.at(cur));
                    }
                }
                update_remove(v, out, graph_.graph(), graph_.reverse_graph(), succ_.at(cur));
                pred_[cur] = move(parent_.D2_.pred_.at(cur));
                for (const auto &i : succ_temp_) {
                    if (pred_.at(cur).at(i.first) != INF8) {
                        resume_bfs(i.first, graph_.reverse_graph(), pred_.at(cur));
                    }
                }
                update_remove(v, in, graph_.reverse_graph(), graph_.graph(), pred_.at(cur));
            }
            else { // not indexed before
                succ_[cur] = vector<distance_t>(graph_.num_vertices(), INF8);
                construct_bfs(cur, graph_.graph(), succ_.at(cur));
                pred_[cur] = vector<distance_t>(graph_.num_vertices(), INF8);
                construct_bfs(cur, graph_.reverse_graph(), pred_.at(cur));
            }
        }
        else { // is D2
            if (succ_temp_.find(cur) != succ_temp_.end()) { // was indexed
                succ_[cur] = move(succ_temp_.at(cur));
                for (const auto &i : parent_.D1_.succ_temp_) {
                    if (succ_.at(cur).at(i.first) != INF8) {
                        resume_bfs(i.first, graph_.graph(), succ_.at(cur));
                    }
                }
                update_remove(v, out, graph_.graph(), graph_.reverse_graph(), succ_.at(cur));
                pred_[cur] = move(pred_temp_.at(cur));
                for (const auto &i : parent_.D1_.succ_temp_) {
                    if (pred_.at(cur).at(i.first) != INF8) {
                        resume_bfs(i.first, graph_.reverse_graph(), pred_.at(cur));
                    }
                }
                update_remove(v, in, graph_.reverse_graph(), graph_.graph(), pred_.at(cur));
            }
            else if (parent_.D1_.succ_temp_.find(cur) != parent_.D1_.succ_temp_.end()) { // was indexed in D1
                succ_[cur] = move(parent_.D1_.succ_temp_.at(cur));
                update_remove(v, out, graph_.graph(), graph_.reverse_graph(), succ_.at(cur));
                pred_[cur] = move(parent_.D1_.pred_temp_.at(cur));
                update_remove(v, in, graph_.reverse_graph(), graph_.graph(), pred_.at(cur));
            }
            else { // not indexed before
                succ_[cur] = vector<distance_t>(graph_.num_vertices(), INF8);
                construct_bfs(cur, graph_.graph(), succ_.at(cur));
                pred_[cur] = vector<distance_t>(graph_.num_vertices(), INF8);
                construct_bfs(cur, graph_.reverse_graph(), pred_.at(cur));
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

DynamicScalableKReach::DynamicPartialIndex::DynamicPartialIndex(DynamicScalableKReach &parent, const Graph &graph,
                                                                distance_t k, uint32_t budget, bool isD2)
        : parent_(parent), graph_(graph), k_(k), budget_(budget), isD2_(isD2),
          quedeg_(parent.quedeg_), degree_(parent.degree_),
          queue_(parent.queue_), quedist_(parent.quedist_), updated_(parent.updated_) {}

bool DynamicScalableKReach::DynamicPartialIndex::has_parent(vertex_t s,
                                                            const std::vector<std::vector<vertex_t>> &reverse_graph,
                                                            const std::vector<distance_t> &dist) {
    for (const auto &prev : reverse_graph.at(s)) {
        if (dist.at(prev) < dist.at(s)) {
            return true;
        }
    }
    return false;
}

void DynamicScalableKReach::DynamicPartialIndex::update_remove(vertex_t s, vertex_t t,
                                                               const std::vector<std::vector<vertex_t>> &graph,
                                                               const std::vector<std::vector<vertex_t>> &reverse_graph,
                                                               std::vector<distance_t> &dist) {
    if (dist.at(s) >= k_ || dist.at(t) > k_ || dist.at(s) + 1 != dist.at(t)) {
        return;
    }
    updated_.clear();
    collect_changes(t, graph, reverse_graph, dist);
    fix_changes(graph, reverse_graph, dist);
}

void
DynamicScalableKReach::DynamicPartialIndex::collect_changes(vertex_t s, const std::vector<std::vector<vertex_t>> &graph,
                                                            const std::vector<std::vector<vertex_t>> &reverse_graph,
                                                            std::vector<distance_t> &dist) {
    back_ = 0;
    front_ = 0;
    if (dist.at(s) > 0 && !has_parent(s, reverse_graph, dist)) {
        queue_.at(back_++) = s;
        dist.at(s) = INF8;
        updated_.push_back(s);
    }
    while (back_ != front_) {
        auto cur = queue_.at(front_++);
        for (const auto &nxt : graph.at(cur)) {
            if (dist.at(nxt) != INF8 && dist.at(nxt) > 0 && !has_parent(nxt, reverse_graph, dist)) {
                queue_.at(back_++) = nxt;
                dist.at(nxt) = INF8;
                updated_.push_back(nxt);
            }
        }
    }
}

void DynamicScalableKReach::DynamicPartialIndex::collect_changes(const std::vector<vertex_t> &sv,
                                                                 const std::vector<std::vector<vertex_t>> &graph,
                                                                 const std::vector<std::vector<vertex_t>> &reverse_graph,
                                                                 std::vector<distance_t> &dist) {
    back_ = 0;
    front_ = 0;
    for (const auto &s : sv) {
        if (dist.at(s) > 0 && !has_parent(s, reverse_graph, dist)) {
            queue_.at(back_++) = s;
            dist.at(s) = INF8;
            updated_.push_back(s);
        }
    }
    while (back_ != front_) {
        auto cur = queue_.at(front_++);
        for (const auto &nxt : graph.at(cur)) {
            if (dist.at(nxt) != INF8 && dist.at(nxt) > 0 && !has_parent(nxt, reverse_graph, dist)) {
                queue_.at(back_++) = nxt;
                dist.at(nxt) = INF8;
                updated_.push_back(nxt);
            }
        }
    }
}

void DynamicScalableKReach::DynamicPartialIndex::fix_changes(const std::vector<std::vector<vertex_t>> &graph,
                                                             const std::vector<std::vector<vertex_t>> &reverse_graph,
                                                             std::vector<distance_t> &dist) {
    for (const auto &w : updated_) {
        for (const auto &prv : reverse_graph.at(w)) {
            if (dist.at(prv) + 1 < dist.at(w) && dist.at(prv) + 1 <= k_) {
                dist.at(w) = dist.at(prv) + 1;
            }
        }
        quedist_.emplace(dist.at(w), w);
    }
    while (!quedist_.empty()) {
        auto d = quedist_.top().first;
        auto w = quedist_.top().second;
        quedist_.pop();
        if (d >= k_) {
            continue;
        }
        for (const auto &nxt : graph.at(w)) {
            if (dist.at(nxt) > d + 1) {
                dist.at(nxt) = d + 1;
                quedist_.emplace(dist.at(nxt), nxt);
            }
        }
    }
}

void DynamicScalableKReach::DynamicPartialIndex::update_remove(vertex_t v, const std::vector<vertex_t> &out,
                                                               const std::vector<std::vector<vertex_t>> &graph,
                                                               const std::vector<std::vector<vertex_t>> &reverse_graph,
                                                               std::vector<distance_t> &dist) {
    if (dist.at(v) > k_) {
        return;
    }
    dist.at(v) = INF8;
    updated_.clear();
    collect_changes(out, graph, reverse_graph, dist);
    fix_changes(graph, reverse_graph, dist);
}

void DynamicScalableKReach::DynamicPartialIndex::construct() {
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

bool DynamicScalableKReach::DynamicPartialIndex::indexed(vertex_t v) const {
    return succ_.find(v) != succ_.end();
}

distance_t DynamicScalableKReach::DynamicPartialIndex::distance(vertex_t s, vertex_t t) const {
    return indexed(s) ? succ_.at(s).at(t) : pred_.at(t).at(s);
}

bool DynamicScalableKReach::DynamicPartialIndex::single_intermediate(vertex_t s, vertex_t t) const {
    for (const auto &i : succ_) {
        if (pred_.at(i.first).at(s) + succ_.at(i.first).at(t) <= k_) {
            return true;
        }
    }
    return false;
}

bool DynamicScalableKReach::DynamicPartialIndex::double_intermediate(vertex_t s, vertex_t t) const {
    for (const auto &i : succ_) {
        for (const auto &j : succ_) {
            if (pred_.at(i.first).at(s) + succ_.at(i.first).at(j.first) + succ_.at(j.first).at(t) <= k_) {
                return true;
            }
        }
    }
    return false;
}

void DynamicScalableKReach::DynamicPartialIndex::set_degree() {
    copy(graph_.degree().begin(), graph_.degree().end(), degree_.begin());
    if (isD2_) {
        for (const auto &i : parent_.D1_.succ_) {
            cover(i.first);
        }
    }
}

void DynamicScalableKReach::DynamicPartialIndex::cover(vertex_t v) {
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

void DynamicScalableKReach::DynamicPartialIndex::update_cover(vertex_t v) {
    for (vertex_t j = 0; j < graph_.num_vertices(); ++j) {
        if ((succ_.at(v).at(j) != INF8 || pred_.at(v).at(j) != INF8) &&
            degree_.at(j) > 0) {
            cover(j);
        }
    }
}

void
DynamicScalableKReach::DynamicPartialIndex::construct_bfs(vertex_t s, const std::vector<std::vector<vertex_t>> &graph,
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

void DynamicScalableKReach::DynamicPartialIndex::insert_vertex(vertex_t v, const std::vector<vertex_t> &out,
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
                update_insert(v, out, in, graph_.graph(), succ_.at(cur));
                pred_[cur] = move(pred_temp_.at(cur));
                update_insert(v, in, out, graph_.reverse_graph(), pred_.at(cur));
            }
            else if (parent_.D2_.indexed(cur)) { // indexed in D2
                succ_[cur] = move(parent_.D2_.succ_.at(cur));
                for (const auto &i : succ_temp_) {
                    if (succ_.at(cur).at(i.first) != INF8) {
                        resume_bfs(i.first, graph_.graph(), succ_.at(cur));
                    }
                }
                update_insert(v, out, in, graph_.graph(), succ_.at(cur));
                pred_[cur] = move(parent_.D2_.pred_.at(cur));
                for (const auto &i : succ_temp_) {
                    if (pred_.at(cur).at(i.first) != INF8) {
                        resume_bfs(i.first, graph_.reverse_graph(), pred_.at(cur));
                    }
                }
                update_insert(v, in, out, graph_.reverse_graph(), pred_.at(cur));
            }
            else { // not indexed before
                succ_[cur] = vector<distance_t>(graph_.num_vertices(), INF8);
                construct_bfs(cur, graph_.graph(), succ_.at(cur));
                pred_[cur] = vector<distance_t>(graph_.num_vertices(), INF8);
                construct_bfs(cur, graph_.reverse_graph(), pred_.at(cur));
            }
        }
        else { // is D2
            if (succ_temp_.find(cur) != succ_temp_.end()) { // was indexed
                succ_[cur] = move(succ_temp_.at(cur));
                for (const auto &i : parent_.D1_.succ_temp_) {
                    if (succ_.at(cur).at(i.first) != INF8) {
                        resume_bfs(i.first, graph_.graph(), succ_.at(cur));
                    }
                }
                update_insert(v, out, in, graph_.graph(), succ_.at(cur));
                pred_[cur] = move(pred_temp_.at(cur));
                for (const auto &i : parent_.D1_.succ_temp_) {
                    if (pred_.at(cur).at(i.first) != INF8) {
                        resume_bfs(i.first, graph_.reverse_graph(), pred_.at(cur));
                    }
                }
                update_insert(v, in, out, graph_.reverse_graph(), pred_.at(cur));
            }
            else if (parent_.D1_.succ_temp_.find(cur) != parent_.D1_.succ_temp_.end()) { // was indexed in D1
                succ_[cur] = move(parent_.D1_.succ_temp_.at(cur));
                update_insert(v, out, in, graph_.graph(), succ_.at(cur));
                pred_[cur] = move(parent_.D1_.pred_temp_.at(cur));
                update_insert(v, in, out, graph_.reverse_graph(), pred_.at(cur));
            }
            else { // not indexed before
                succ_[cur] = vector<distance_t>(graph_.num_vertices(), INF8);
                construct_bfs(cur, graph_.graph(), succ_.at(cur));
                pred_[cur] = vector<distance_t>(graph_.num_vertices(), INF8);
                construct_bfs(cur, graph_.reverse_graph(), pred_.at(cur));
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

void DynamicScalableKReach::DynamicPartialIndex::update_insert(vertex_t v, const std::vector<vertex_t> &out,
                                                               const std::vector<vertex_t> &in,
                                                               const std::vector<std::vector<vertex_t>> &graph,
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
        resume_bfs(v, graph, dist);
    }
    else {
        for (const auto &t : out) {
            if (dist.at(v) + 1 < dist.at(t)){
                dist.at(t) = dist.at(v) + 1;
                resume_bfs(t, graph, dist);
            }
        }
    }
}


void DynamicScalableKReach::insert_edge(vertex_t s, vertex_t t) {
    D1_.insert_edge(s, t);
    D2_.insert_edge(s, t);
}

void DynamicScalableKReach::remove_edge(vertex_t s, vertex_t t) {
    D1_.remove_edge(s, t);
    D2_.remove_edge(s, t);
}

void
DynamicScalableKReach::remove_vertex(vertex_t v, const std::vector<vertex_t> &out, const std::vector<vertex_t> &in) {
    D1_.remove_vertex(v, out, in);
    D2_.remove_vertex(v, out, in);
}

DynamicScalableKReach::DynamicScalableKReach(const Graph &graph, distance_t k, uint32_t b1, uint32_t b2)
        : graph_(graph), k_(k), D1_(*this, graph, k, b1), D2_(*this, graph, k, b2, true),
          degree_(graph.num_vertices()), distance_(graph.num_vertices(), INF8), queue_(graph.num_vertices()) {
    std::vector<std::pair<degree_t, vertex_t>> quedeg_temp_;
    quedeg_temp_.reserve(graph.num_vertices());
    DegreeQueue(less<pair<degree_t, vertex_t>>(), move(quedeg_temp_)).swap(quedeg_);
    updated_.reserve(graph.num_vertices());
    std::vector<std::pair<distance_t, vertex_t>> quedist_temp_;
    quedist_temp_.reserve(graph.num_vertices());
    DistanceQueue(greater<pair<distance_t, vertex_t>>(), move(quedist_temp_)).swap(quedist_);
}

void DynamicScalableKReach::construct() {
    D1_.construct();
    D2_.construct();
}

bool DynamicScalableKReach::query(vertex_t s, vertex_t t) const {
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

bool DynamicScalableKReach::bfs(vertex_t s, vertex_t t) const {
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
DynamicScalableKReach::insert_vertex(vertex_t v, const std::vector<vertex_t> &out, const std::vector<vertex_t> &in) {
    D1_.insert_vertex(v, out, in);
    D2_.insert_vertex(v, out, in);
}
