#include <iostream>

#include "dynamic_k_reach.h"

using namespace std;

int main() {
    string filename("k_reach_sample_2");

    dynamic_k_reach dkr;
    dkr.construct_index(filename, 1);

    /*cout << dkr.query_reachability(2, 9) << endl;

    dkr.insert_edge(4, 7);

    cout << dkr.query_reachability(2, 9) << endl;*/
    cout << dkr.query_reachability(2, 5) << endl;
    dkr.insert_edge(2, 5);
    cout << dkr.query_reachability(2, 5) << endl;

    return 0;
}