#include <iostream>
#include <vector>
#include <common.h>
#include <fstream>
#include <KReach.h>
#include <DynamicKReach.h>

using std::vector;
using std::string;
using std::ifstream;
using std::cout;
using std::endl;
using std::flush;

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
    for (const auto &e : graph){
        cout << e.first << " " << e.second << endl;
        kReach.remove_edge(e.first, e.second);
        dynamicKReach.remove_edge(e.first, e.second);
        TEST_equals(kReach, dynamicKReach);
        kReach.insert_edge(e.first, e.second);
        dynamicKReach.insert_edge(e.first, e.second);
        TEST_equals(kReach, dynamicKReach);
    }
    return 0;
}