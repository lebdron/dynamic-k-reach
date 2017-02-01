#include <stack>
#include "DynamicKReach.h"

using std::stack;

void DynamicKReach::insert_update(Vertex s, Vertex t, Weight d) {
    for(const auto &p : index.in(s)){
        auto weight_ps = weight(p, s);
        for(const auto &q : index.out(t)){
            auto weight_tq = weight(t, q);
            auto pq = weight.find(p, q);
            if (!weight.defined(pq) && weight_ps + weight_tq + d <= k){
                index.insert(p, q);
                weight(p, q) = weight_ps + weight_tq + d;
            }
            else if (weight.defined(pq) && weight_ps + weight_tq + d < pq->second){
                pq->second = weight_ps + weight_tq + d;
            }
        }
    }
}

void DynamicKReach::remove_identify(Vertex s, Vertex t, Weight d) {
    for (const auto &p : index.in(s)){
        auto weight_ps = weight(p, s);
        for (const auto &q : index.out(t)){
            auto weight_tq = weight(t, q);
            auto pq = weight.find(p, q);
            if (weight.defined(pq) && weight_ps + weight_tq + d == pq->second){
                const auto &pout = graph.out(p), &qin = graph.in(q);
                InclusiveIntersection begin(pout.begin(), pout.end(), qin.begin(), qin.end()),
                        end(pout.end(), pout.end(), qin.end(), qin.end());
                if (begin != end) {
                    pq->second = 2;
                    continue;
                }
                identified.insert(Edge(p, q));
            }
        }
    }
}

void DynamicKReach::remove_update() {
    processing.insert(*identified.begin());
    identified.erase(identified.begin());
    stack<Edge> stackEdges;
    stackEdges.push(*processing.begin());
    while(!stackEdges.empty()){
        Vertex s = stackEdges.top().first, t = stackEdges.top().second;
        auto st = weight.find(s, t);
        stackEdges.pop();
        Weight result = k + 1;
        const auto &sout = index.out(s), &tin = index.in(t);
        ExclusiveIntersection begin(s, sout.begin(), sout.end(), t, tin.begin(), tin.end()),
                end(s, sout.end(), sout.end(), t, tin.end(), tin.end());
        for (; begin != end; ++begin){
            Vertex w = *begin;
            auto sw = identified.find(Edge(s, w));
            if (sw != identified.end()){
                stackEdges.push(Edge(s, t));
                stackEdges.push(Edge(s, w));
                processing.insert(*sw);
                identified.erase(sw);
                goto next;
            }
            auto wt = identified.find(Edge(w, t));
            if (wt != identified.end()){
                stackEdges.push(Edge(s, t));
                stackEdges.push(Edge(w, t));
                processing.insert(*wt);
                identified.erase(wt);
                goto next;
            }
            if (processing.count(Edge(s, w)) || processing.count(Edge(w, t))){
                continue;
            }
            if (weight(s, w) + weight(w, t) < result){
                result = weight(s, w) + weight(w, t);
            }
        }
        if (result <= k){
            st->second = result;
        }
        else{
            index.remove(s, t);
            weight.undefine(st);
        }
        processing.erase(Edge(s, t));
        next:;
    }
}

void DynamicKReach::insert_edge(Vertex s, Vertex t) {
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
    if (!index.contains(s) && !index.contains(t)){
        Vertex v = graph(s).degree() > graph(t).degree() ? s : t;
        index[v];
        index.insert(v, v);
        weight(v, v) = 0;
        for (const auto &w : graph.out(v)){
            for (const auto &q : index.out(w)){
                auto weight_wq = weight(w, q);
                auto it = weight.find(v, q);
                if (!weight.defined(it) && weight_wq + 1 <= k){
                    index.insert(v, q);
                    weight(v, q) = weight_wq + 1;
                }
                else if (weight.defined(it) && weight_wq + 1 < it->second){
                    it->second = weight_wq + 1;
                }
            }
        }
        for (const auto &w : graph.in(v)){
            for (const auto &p : index.in(w)){
                auto weight_pw = weight(p, w);
                auto it = weight.find(p, v);
                if (!weight.defined(it) && weight_pw + 1 <= k){
                    index.insert(p, v);
                    weight(p, v) = weight_pw + 1;
                }
                else if (weight.defined(it) && weight_pw + 1 < it->second){
                    it->second = weight_pw + 1;
                }
            }
        }
    }
    graph.insert(s, t);
    if (index.contains(s) && index.contains(t)){
        insert_update(s, t, 1);
    }
    else if (index.contains(s)){
        for (const auto &w : graph.out(t)){
            insert_update(s, w, 2);
        }
    }
    else{
        for (const auto &w : graph.in(s)){
            insert_update(w, t, 2);
        }
    }
}

void DynamicKReach::remove_edge(Vertex s, Vertex t) {
    if (s == t || !mapper.present(s) || !mapper.present(t)){
        return;
    }
    s = mapper(s);
    t = mapper(t);
    if (!graph.out(s).contains(t)){
        return;
    }
    graph.remove(s, t);
    if (index.contains(s) && index.contains(t)){
        remove_identify(s, t, 1);
    }
    else if (index.contains(s)){
        for (const auto &w : graph.out(t)){
            remove_identify(s, w, 2);
        }
    }
    else {
        for (const auto &w : graph.in(s)){
            remove_identify(w, t, 2);
        }
    }
    while (!identified.empty()){
        remove_update();
    }
}

void DynamicKReach::remove_vertex(Vertex v) {
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
    if (index.contains(v)){
        index.remove(v, v);
        weight.undefine(weight.find(v, v));
        remove_identify(v, v, 0);
        for (const auto &p : index.in(v)){
            index.out(p).remove(v);
            weight.undefine(weight.find(p, v));
        }
        for (const auto &q : index.out(v)){
            index.in(q).remove(v);
            weight.undefine(weight.find(v, q));
        }
        index.remove(v);
    }
    else{
        for (const auto &p : graph.in(v)){
            for(const auto &q : graph.out(v)){
                remove_identify(p, q, 2);
            }
        }
    }
    graph(v).clear();
    while(!identified.empty()){
        remove_update();
    }
}

DynamicKReach::DynamicKReach(const DynamicKReach &i) : KReach(i){

}

DynamicKReach &DynamicKReach::operator=(DynamicKReach i) {
    KReach::operator=(i);
    return *this;
}

DynamicKReach::DynamicKReach(const KReach &i) : KReach(i) {

}

DynamicKReach &DynamicKReach::operator=(KReach i) {
    KReach::operator=(i);
    return *this;
}

void DynamicKReach::remove_vertex_edges(Vertex v) {
    if (!mapper.present(v)){
        return;
    }
    Vertex v_old = v;
    v = mapper(v);
    while (!graph.in(v).empty()){
        Vertex u = *graph.in(v).begin();
        graph.remove(u, v);
        if (index.contains(u) && index.contains(v)){
            remove_identify(u, v, 1);
        }
        else if (index.contains(u)){
            for (const auto &w : graph.out(v)){
                remove_identify(u, w, 2);
            }
        }
        else {
            for (const auto &w : graph.in(u)){
                remove_identify(w, v, 2);
            }
        }
        while (!identified.empty()){
            remove_update();
        }
    }
    while (!graph.out(v).empty()){
        Vertex u = *graph.out(v).begin();
        graph.remove(v, u);
        if (index.contains(v) && index.contains(u)){
            remove_identify(v, u, 1);
        }
        else if (index.contains(v)){
            for (const auto &w : graph.out(u)){
                remove_identify(v, w, 2);
            }
        }
        else {
            for (const auto &w : graph.in(v)){
                remove_identify(w, u, 2);
            }
        }
        while (!identified.empty()){
            remove_update();
        }
    }
    if (index.contains(v)) {
        index.remove(v);
        weight.undefine(weight.find(v, v));
    }
    graph(v).clear();
    mapper.remove(v_old);
}
