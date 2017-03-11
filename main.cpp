#include <KReach.h>
#include <DynamicKReach.h>
#include <ScalableKReach.h>
#include <DynamicScalableKReach.h>

#include <iostream>
#include <memory>

using namespace std;

void
equals(const Graph &graph, const AbstractKReach &lhs, const AbstractKReach &rhs) {
    for (degree_t s = 0; s < graph.num_vertices(); ++s) {
        for (degree_t t = 0; t < graph.num_vertices(); ++t) {
            if (lhs.query(s, t) != rhs.query(s, t)) {
                cout << "ERROR " << s << " " << t << " " << lhs.query(s, t) << " " << rhs.query(s, t)
                     << endl;
            }
        }
    }
}

int main() {
    Graph graph;
    graph.from_kreach("data/sample.kreach");
    graph.compute_degree();

    unique_ptr<AbstractKReach> indexes[4];
    indexes[0].reset(new KReach(graph, 3));
    indexes[1].reset(new DynamicKReach(graph, 3));
    indexes[2].reset(new ScalableKReach(graph, 3, 1000, 10000));
    indexes[3].reset(new DynamicScalableKReach(graph, 3, 1000, 10000));

    for (auto &i : indexes){
        i->construct();
    }

    vector<pair<vertex_t, vertex_t>> edges;
    for (vertex_t s = 0; s < graph.num_vertices(); ++s) {
        for (vertex_t t : graph.successors(s)) {
            edges.push_back(make_pair(s, t));
        }
    }

    /*
     * Edge operations
     */
    for (const auto &p : edges) {
        auto s = p.first;
        auto t = p.second;
        cout << s << " " << t << endl;

        graph.remove_edge(s, t);

        for (auto &i : indexes){
            i->remove_edge(s, t);
        }
        for (const auto &i : indexes){
            for (const auto &j : indexes){
                equals(graph, *i, *j);
            }
        }

        graph.insert_edge(s, t);

        for (auto &i : indexes){
            i->insert_edge(s, t);
        }
        for (const auto &i : indexes){
            for (const auto &j : indexes){
                equals(graph, *i, *j);
            }
        }

    }

    /*
     * Vertex operations
     */
    for (vertex_t s = 0; s < graph.num_vertices(); ++s){
        cout << s << endl;
        auto out = move(graph.successors(s)), in = move(graph.predecessors(s));

        graph.remove_vertex(s);

        for (auto &i : indexes){
            i->remove_vertex(s, out, in);
        }
        for (const auto &i : indexes){
            for (const auto &j : indexes){
                equals(graph, *i, *j);
            }
        }

        graph.insert_vertex(s, out, in);

        for (auto &i : indexes){
            i->insert_vertex(s, out, in);
        }
        for (const auto &i : indexes){
            for (const auto &j : indexes){
                equals(graph, *i, *j);
            }
        }
    }

    return 0;
}