#include <iostream>

#include "dynamic_k_reach.h"

using namespace std;

int main() {
    string filename("k_reach_sample");

    dynamic_k_reach dkr;
    dkr.construct_index(filename, 3);

    dkr.insert_vertex(11);
    dkr.insert_edge(11, 5);

    bool r = dkr.query_reachability(11, 7);
    cout << r << endl;

    return 0;
}