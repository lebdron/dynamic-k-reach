#pragma once

#include <set>
#include <map>
#include <unordered_map>
#include "common.h"

class dynamic_k_reach_v3 {
    typedef std::map<vertex_t, std::set<vertex_t>> adjacency_list_t;

    struct edge_hasher{
        std::size_t operator()(const edge_t &e) const
        {
            return std::hash<vertex_t>{}(e.first) ^ std::hash<vertex_t>{}(e.second) + e.first == e.second ? e.first : 0;
        }
    };

    struct edge_comparator{
        bool operator()(const edge_t &e1, const edge_t &e2) const
        {
            return e1.first == e2.first && e1.second == e2.second || e1.first == e2.second && e1.second == e2.first;
        }
    };

    adjacency_list_t graph_in, graph_out, index_in, index_out;
    std::unordered_map<edge_t, weight_t, edge_hasher, edge_comparator> weight;



public:


};