#include <iostream>
#include <fstream>

#include "dynamic_k_reach.h"

using namespace std;

int main()
{
    vector<edge_t> edges;
    {
        string filename("");
        ifstream fin(filename);
        for (vertex_t s, t; fin >> s >> t;) {
            edges.push_back(make_pair(s, t));
        }
    }

    dynamic_k_reach dkr;
    dkr.construct_index(edges, 3);
    cout << "Index constructed" << endl;

    /*{
        ifstream fin("dels_e");
        vertex_t s, t;
        while (fin >> s >> t){
            dkr.remove_edge(s, t);
            cout << "Edge removed" << endl;
        }
        fin.close();
    }*/

    /*{
        ifstream fin("dels_v_er");
        vertex_t v;
        while (fin >> v){
            dkr.remove_vertex(v);
            cout << "Vertex removed" << endl;
        }
        fin.close();
    }

    {
        ifstream fin("queries_er");
        vertex_t s, t;
        while (fin >> s >> t){
            cout << s << " " << t << ": " << dkr.query_reachability(s, t) << endl;
        }
        fin.close();
    }*/

    return 0;
}