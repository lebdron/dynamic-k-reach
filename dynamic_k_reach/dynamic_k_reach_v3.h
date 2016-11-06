#pragma once

#include <set>
#include <map>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <iterator>
#include <cstddef>
#include "common.h"

#include "gtest/gtest_prod.h"

class dynamic_k_reach_v3
{
    typedef std::set<vertex_t> adjacent_t;

    struct neighbors_t
    {
        adjacent_t out, in;

        class iterator : public std::iterator<std::input_iterator_tag, vertex_t, ptrdiff_t, const vertex_t *, vertex_t>
        {
            adjacent_t::iterator out, in;
            vertex_t value;
            neighbors_t &outer;

            void set_value();

        public:
            explicit iterator(neighbors_t &outer, adjacent_t::iterator out, adjacent_t::iterator in);

            iterator &operator++();

            iterator operator++(int);

            bool operator==(iterator it) const;

            bool operator!=(iterator it) const;

            reference operator*() const;
        };

        iterator begin();

        iterator end();

        size_t size() const;
    };

    class graph_adjacency_lists : public std::vector<neighbors_t>
    {
    public:
        graph_adjacency_lists() : vector() {}

        graph_adjacency_lists(uint32_t n) : vector(n) {}

        void insert(vertex_t s, vertex_t t);

        void erase(vertex_t s, vertex_t t);
    };

    class index_adjacency_lists : public std::unordered_map<vertex_t, neighbors_t>
    {
    public:
        void insert(vertex_t s, vertex_t t);

        void erase(vertex_t s, vertex_t t);
    };

    struct edge_hasher
    {
        std::size_t operator()(const edge_t &e) const;
    };

    struct edge_comparator
    {
        bool operator()(const edge_t &e1, const edge_t &e2) const;
    };

    typedef std::unordered_map<edge_t, weight_t, edge_hasher, edge_comparator> weight_function_t;

    class weight_function : public weight_function_t
    {
    public:
        weight_t &operator()(vertex_t s, vertex_t t);

        size_type count(vertex_t s, vertex_t t) const;
    };

    weight_t k;
    graph_adjacency_lists graph;
    index_adjacency_lists index;
    weight_function weight;

    std::unordered_map<vertex_t, vertex_t> mapping;

    void generate_cover();

    void bfs(vertex_t s);

    FRIEND_TEST(neighbors_iterator, eq_to_union);

public:
    void construct_index(std::vector<edge_t> edges, weight_t k);

    void edge_insert(vertex_t s, vertex_t t);

};