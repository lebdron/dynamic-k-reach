#include "DynamicKReach.h"

using namespace std;

DynamicKReach::DynamicKReach(const Graph &graph, distance_t k)
        : graph_(graph), k_(k), degree_(graph.num_vertices()), queue_(graph.num_vertices()){
    vector<pair<degree_t, vertex_t>> quedeg_temp_;
    quedeg_temp_.reserve(graph.num_vertices());
    DegreeQueue(less<pair<degree_t, vertex_t>>(), move(quedeg_temp_)).swap(quedeg_);
    updated_.reserve(graph.num_vertices());
    vector<pair<distance_t, vertex_t>> quedist_temp_;
    quedist_temp_.reserve(graph.num_vertices());
    DistanceQueue(greater<pair<distance_t, vertex_t>>(), move(quedist_temp_)).swap(quedist_);
}

void DynamicKReach::construct() {
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

bool DynamicKReach::query(vertex_t s, vertex_t t) const {
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

bool DynamicKReach::indexed(vertex_t v) const {
    return index_.find(v) != index_.end();
}

distance_t DynamicKReach::distance(vertex_t s, vertex_t t) const {
    return index_.at(s).at(t);
}

void DynamicKReach::set_degree() {
    copy(graph_.degree().begin(), graph_.degree().end(), degree_.begin());
}

void DynamicKReach::cover(vertex_t v) {
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

void DynamicKReach::construct_bfs(vertex_t s, std::vector<distance_t> &dist) {
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

void DynamicKReach::insert_edge(vertex_t s, vertex_t t) {
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

void DynamicKReach::update_insert(vertex_t s, vertex_t t, std::vector<distance_t> &dist) {
    if (dist.at(s) >= k_ || dist.at(t) <= dist.at(s) + 1) {
        return;
    }
    dist.at(t) = dist.at(s) + 1;
    resume_bfs(t, dist);
}

void DynamicKReach::resume_bfs(vertex_t s, std::vector<distance_t> &dist) {
    back_ = 0;
    front_ = 0;
    assert(dist.at(s) != INF8);
    queue_.at(back_++) = s;
    while (back_ != front_) {
        auto cur = queue_.at(front_++);
        if (dist.at(cur) >= k_) {
            continue;
        }
        for (const auto &nxt : graph_.successors(cur)) {
            if (dist.at(nxt) <= dist.at(cur) + 1) {
                continue;
            }
            dist.at(nxt) = dist.at(cur) + 1;
            if (graph_.successors(nxt).size() > 0) {
                queue_.at(back_++) = nxt;
            }
        }
    }
}

void DynamicKReach::insert_vertex(vertex_t v, const std::vector<vertex_t> &out, const std::vector<vertex_t> &in) {
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

void DynamicKReach::update_insert(vertex_t v, const std::vector<vertex_t> &out, const std::vector<vertex_t> &in,
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

void DynamicKReach::remove_edge(vertex_t s, vertex_t t) {
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
            update_remove(s, t, index_.at(cur));
        }
        else {
            index_[cur] = vector<distance_t>(graph_.num_vertices(), INF8);
            construct_bfs(cur, index_.at(cur));
        }
        index_temp_.erase(cur);
        cover(cur);
    }
}

void DynamicKReach::update_remove(vertex_t s, vertex_t t, std::vector<distance_t> &dist) {
    if (dist.at(s) >= k_ || dist.at(t) > k_ || dist.at(s) + 1 != dist.at(t)) {
        return;
    }
    updated_.clear();
    collect_changes(t, dist);
    fix_changes(dist);
}

void DynamicKReach::collect_changes(vertex_t s, std::vector<distance_t> &dist) {
    back_ = 0;
    front_ = 0;
    if (dist.at(s) > 0 && !has_parent(s, dist)) {
        queue_.at(back_++) = s;
        dist.at(s) = INF8;
        updated_.push_back(s);
    }
    while (back_ != front_) {
        auto cur = queue_.at(front_++);
        for (const auto &nxt : graph_.successors(cur)) {
            if (dist.at(nxt) != INF8 && dist.at(nxt) > 0 && !has_parent(nxt, dist)) {
                queue_.at(back_++) = nxt;
                dist.at(nxt) = INF8;
                updated_.push_back(nxt);
            }
        }
    }
}

bool DynamicKReach::has_parent(vertex_t s, const std::vector<distance_t> &dist) {
    for (const auto &prev : graph_.predecessors(s)) {
        if (dist.at(prev) < dist.at(s)) {
            return true;
        }
    }
    return false;
}

void DynamicKReach::fix_changes(std::vector<distance_t> &dist) {
    for (const auto &w : updated_) {
        for (const auto &prv : graph_.predecessors(w)) {
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
        for (const auto &nxt : graph_.successors(w)) {
            if (dist.at(nxt) > d + 1) {
                dist.at(nxt) = d + 1;
                quedist_.emplace(dist.at(nxt), nxt);
            }
        }
    }
}

void DynamicKReach::remove_vertex(vertex_t v, const std::vector<vertex_t> &out, const std::vector<vertex_t> &in) {
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
            update_remove(v, out, index_.at(cur));
        }
        else {
            index_[cur] = vector<distance_t>(graph_.num_vertices(), INF8);
            construct_bfs(cur, index_.at(cur));
        }
        index_temp_.erase(cur);
        cover(cur);
    }
}

void DynamicKReach::update_remove(vertex_t v, const std::vector<vertex_t> &out, std::vector<distance_t> &dist) {
    if (dist.at(v) > k_) {
        return;
    }
    dist.at(v) = INF8;
    updated_.clear();
    collect_changes(out, dist);
    fix_changes(dist);
}

void DynamicKReach::collect_changes(const std::vector<vertex_t> &sv, std::vector<distance_t> &dist) {
    back_ = 0;
    front_ = 0;
    for (const auto &s : sv) {
        if (dist.at(s) > 0 && !has_parent(s, dist)) {
            queue_.at(back_++) = s;
            dist.at(s) = INF8;
            updated_.push_back(s);
        }
    }
    while (back_ != front_) {
        auto cur = queue_.at(front_++);
        for (const auto &nxt : graph_.successors(cur)) {
            if (dist.at(nxt) != INF8 && dist.at(nxt) > 0 && !has_parent(nxt, dist)) {
                queue_.at(back_++) = nxt;
                dist.at(nxt) = INF8;
                updated_.push_back(nxt);
            }
        }
    }
}
