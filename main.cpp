#include <iostream>

#include "dynamic_k_reach.h"

using namespace std;

int main()
{
    string filename("sample_3");

    dynamic_k_reach dkr;
    dkr.construct_index(filename, 3);

    cout << dkr.query_reachability(1, 4) << endl;
    dkr.remove_vertex(3);
    cout << dkr.query_reachability(1, 4) << endl;

    return 0;
}