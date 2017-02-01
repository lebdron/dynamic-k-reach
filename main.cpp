#include <iostream>
#include <vector>
#include <common.h>
#include <fstream>
#include <KReach.h>
#include <DynamicKReach.h>
#include <chrono>

using namespace std;

vector<Edge> read_graph(string filename){
    vector<Edge> edges;
    ifstream fin("data/" + filename);
    assert(fin.is_open());
    for (Vertex s, t; fin >> s >> t;){
        edges.push_back(Edge(s, t));
    }
    return edges;
}

int main() {
    string filename = "lastfm";
    auto graph = read_graph(filename);
    cout << "m=" << graph.size() << endl;
    KReach kReach;
    DynamicKReach dynamicKReach;
    kReach.construct_index(graph, 3);
//    dynamicKReach.construct_index(graph, 3);
    dynamicKReach = kReach;
    TEST_equals(kReach, dynamicKReach);
    DynamicKReach dynamicKReach1;
    dynamicKReach1 = kReach;
    TEST_equals(kReach, dynamicKReach1);
    /*for (const auto &e : graph){
        cout << e.first << " " << e.second << endl;
        kReach.remove_edge(e.first, e.second);
        dynamicKReach.remove_edge(e.first, e.second);
        TEST_equals(kReach, dynamicKReach);
        kReach.insert_edge(e.first, e.second);
        dynamicKReach.insert_edge(e.first, e.second);
        TEST_equals(kReach, dynamicKReach);
    }*/
    unordered_set<Vertex> vertices;
    for (const auto &e : graph){
        vertices.insert(e.first);
        vertices.insert(e.second);
    }
    for (const auto &v : vertices){
        cout << v << endl;

        auto start = chrono::steady_clock::now();
        kReach.remove_vertex(v);
        auto end = chrono::steady_clock::now();
        cout << "Reindex: " << chrono::duration_cast<chrono::milliseconds>(end - start).count() << endl;

        start = chrono::steady_clock::now();
        dynamicKReach.remove_vertex(v);
        end = chrono::steady_clock::now();
        cout << "Update opt: " << chrono::duration_cast<chrono::milliseconds>(end - start).count() << endl;
        TEST_equals(kReach, dynamicKReach);

        start = chrono::steady_clock::now();
        dynamicKReach1.remove_vertex_edges(v);
        end = chrono::steady_clock::now();
        cout << "Update naive: " << chrono::duration_cast<chrono::milliseconds>(end - start).count() << endl;
        TEST_equals(kReach, dynamicKReach1);

    }
    return 0;
}