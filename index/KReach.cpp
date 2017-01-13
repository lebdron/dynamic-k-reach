#include <queue>
#include "KReach.h"

using std::multimap;
using std::unordered_set;
using std::unordered_map;
using std::queue;
using std::greater;
using std::make_pair;
using std::swap;

void KReach::construct_cover() {
    for (Vertex s = 0; s < mapper.size(); ++s){
        if (index.contains(s)){
            continue;
        }
        for (const auto &t : graph(s)){
            if (index.contains(t)){
                continue;
            }
            index[s];
            index[t];
            break;
        }
    }
}

void KReach::construct_index() {
    for (const auto &v : index){
        bfs(v.first);
    }
}

void KReach::bfs(Vertex s) {
    unordered_set<Vertex> visited;
    queue<Vertex> frontier;
    visited.insert(s);
    frontier.push(s);
    index.insert(s, s);
    weight(s, s) = 0;
    for (Weight level = 0; !frontier.empty() && level < k; ++level){
        size_t num_frontiers = frontier.size();
        for (size_t i = 0; i < num_frontiers; ++i){
            Vertex u = frontier.front();
            frontier.pop();
            for (const auto &v : graph.out(u)){
                if (visited.count(v)){
                    continue;
                }
                visited.insert(v);
                if (index.contains(v)){
                    index.insert(s, v);
                    weight(s, v) = level + 1;
                }
                if (graph.out(v).empty()){
                    continue;
                }
                frontier.push(v);
            }
        }
    }
}

void KReach::clear() {
    k = 0;
    graph.clear();
    index.clear();
    weight.clear();
    mapper.clear();
}

void KReach::clear_index() {
    for (auto &v : index){
        v.second.clear();
    }
    weight.clear();
}

void KReach::construct_index(std::vector<Edge> edges, Weight k) {
    clear();
    this->k = k;
    {
        using Degree = uint32_t;
        unordered_map<Vertex, Degree> degree;
        for (const auto &e : edges){
            ++degree[e.first];
            ++degree[e.second];
        }
        multimap<Degree, Vertex, greater<Degree>> order;
        for (const auto &v : degree){
            order.insert(make_pair(v.second, v.first));
        }
        for (const auto &v : order){
            mapper[v.second];
        }
        graph.resize(degree.size());
        for (const auto &e : edges){
            graph.insert(mapper(e.first), mapper(e.second));
        }
    }
    construct_cover();
    construct_index();
}

bool KReach::query(Vertex s, Vertex t) const {
    if (!mapper.present(s) || !mapper.present(t)){
        return false;
    }
    if (s == t){
        return true;
    }
    s = mapper(s);
    t = mapper(t);
    if (index.contains(s) && index.contains(t)){
        return weight.defined(weight.find(s, t));
    }
    else if (index.contains(s)){
        const auto &i = index.out(s), &g = graph.in(t);
        InclusiveIntersection begin(i.begin(), i.end(), g.begin(), g.end()),
                end(i.end(), i.end(), g.end(), g.end());
        for (; begin != end; ++begin){
            if (weight(s, *begin) <= k - 1){
                return true;
            }
        }
    }
    else if (index.contains(t)){
        const auto &g = graph.out(s), &i = index.in(t);
        InclusiveIntersection begin(g.begin(), g.end(), i.begin(), i.end()),
                end(g.end(), g.end(), i.end(), i.end());
        for (; begin != end; ++begin){
            if (weight(*begin, t) <= k - 1){
                return true;
            }
        }
    }
    else {
        for (const auto &w : graph.out(s)){
            auto &i = index.out(w), &g = graph.in(t);
            InclusiveIntersection begin(i.begin(), i.end(), g.begin(), g.end()),
                    end(i.end(), i.end(), g.end(), g.end());
            for (; begin != end; ++begin){
                if (weight(w, *begin) <= k - 2) {
                    return true;
                }
            }
        }
    }
    return false;
}

void KReach::insert_edge(Vertex s, Vertex t) {
    if (s == t){
        return;
    }
    insert_vertex(s);
    insert_vertex(t);
    s = mapper(s);
    t = mapper(t);
    if (graph.out(s).contains(t)){
        return;
    }
    graph.insert(s, t);
    if (!index.contains(s) && !index.contains(t)){
        Vertex v = graph(s).degree() > graph(t).degree() ? s : t;
        index[v];
    }
    clear_index();
    construct_index();
}

void KReach::insert_vertex(Vertex v) {
    if (!mapper.present(v) && mapper[v] >= graph.size()){
        graph.resize(mapper.size());
    }
}

void KReach::remove_edge(Vertex s, Vertex t) {
    if (s == t || !mapper.present(s) || !mapper.present(t)){
        return;
    }
    s = mapper(s);
    t = mapper(t);
    if (!graph.out(s).contains(t)){
        return;
    }
    graph.remove(s, t);
    clear_index();
    construct_index();
}

void KReach::remove_vertex(Vertex v) {
    if (!mapper.present(v)){
        return;
    }
    Vertex v_old = v;
    v = mapper(v);
    mapper.remove(v_old);
    for (const auto &p : graph.in(v)){
        graph.out(p).remove(v);
    }
    for (const auto &q : graph.out(v)){
        graph.in(q).remove(v);
    }
    graph(v).clear();
    if (index.contains(v)){
        index.remove(v);
    }
    clear_index();
    construct_index();
}

void TEST_equals(const KReach& i1, const KReach& i2) {
    assert(i1.k == i2.k);
    assert(i1.graph == i2.graph);
    assert(i1.index == i2.index);
    assert(i1.weight == i2.weight);
    assert(i1.mapper == i2.mapper);
}

KReach::KReach(const KReach &i)
        : k(i.k), graph(i.graph), index(i.index), weight(i.weight), mapper(i.mapper){

}

KReach &KReach::operator=(KReach i) {
    swap(k, i.k);
    swap(graph, i.graph);
    swap(index, i.index);
    swap(weight, i.weight);
    swap(mapper, i.mapper);
    return *this;
}
