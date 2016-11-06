#include <gtest/gtest.h>
#include <dynamic_k_reach_v3.h>

TEST(neighbors_iterator, eq_to_union)
{
    dynamic_k_reach_v3::graph_adjacency_lists graph(10);
    srand(0);
    for (int i = 0; i < 10; ++i){
        for (int j = i + 1; j < 10; ++j){
            double x = rand() / (double) RAND_MAX;
            if (x < 0.5){
                graph.insert(i, j);
            }
        }
    }
    std::cout << std::endl;
    for (auto &v : graph) {
        std::set<vertex_t> neighbors;
        std::set_union(v.out.begin(), v.out.end(), v.in.begin(), v.in.end(),
                       std::inserter(neighbors, neighbors.end()));
        std::for_each(neighbors.begin(), neighbors.end(), [](auto n){std::cout << n << " ";});
        std::cout << std::endl;
        std::for_each(v.begin(), v.end(), [](auto n){std::cout << n << " ";});
        std::cout << std::endl;
        ASSERT_TRUE(std::equal(neighbors.begin(), neighbors.end(), v.begin()));
        ASSERT_TRUE(std::distance(v.begin(), v.end()) == neighbors.size());
        ASSERT_TRUE(v.size() == neighbors.size());
    }
}