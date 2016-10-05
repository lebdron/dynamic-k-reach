#include <iostream>
#include <set>
#include <map>
#include <fstream>
#include <queue>
#include <algorithm>

using namespace std;

typedef map<uint32_t, set<uint32_t>> graph_t;
typedef pair<uint32_t, uint8_t> edge_t;
typedef map<uint32_t, map<uint32_t, uint8_t>> index_t;
typedef set<uint32_t> cover_t;

void bfs(const uint32_t s,
         const graph_t &graph,
         const cover_t &cover,
         index_t &index,
         const uint8_t k)
{
    queue<uint32_t> frontier;
    cover_t visited;

    frontier.push(s);
    visited.insert(s);
    uint8_t cur_level = 1, next_level = 0;
    for (uint8_t level = 0; level < k && !frontier.empty(); ){
        uint32_t u = frontier.front();
        frontier.pop();
        for (const auto &v : graph.at(u)){
            if (!visited.count(v)){
                visited.insert(v);
                frontier.push(v);
                ++next_level;
                if (cover.count(v)){
                    if (level + 1 < k - 2){
                        index[s][v] = k - 2;
                    } else {
                        index[s][v] = level + 1;
                    }
                }
            }
        }
        if (!--cur_level){
            swap(cur_level, next_level);
            ++level;
        }
    }
}

void construct_index(const graph_t &graph,
                     const cover_t &cover,
                     index_t &index,
                     const uint8_t k)
{
    for (const auto &i : cover){
        bfs(i, graph, cover, index, k);
    }
}

bool query_index(const uint32_t s,
                 const uint32_t t,
                 const graph_t &in_neighbors,
                 const graph_t &out_neighbors,
                 const index_t &index,
                 const uint8_t k)
{
    if (index.count(s) && index.count(t)){ // case 1
        return index.at(s).count(t);
    } else if (index.count(s) && !index.count(t)){ // case 2
        const auto &in_nei = in_neighbors.at(t);
        const auto &list = index.at(s);
        return any_of(list.begin(), list.end(),
                      [&in_nei, &k](const edge_t &e){return in_nei.count(e.first)
                                                 && e.second <= k - 1;});
    } else if (!index.count(s) && index.count(t)){ // case 3
        const auto &out_nei = out_neighbors.at(s);
        const auto &list = index.at(t);
        return any_of(list.begin(), list.end(),
                      [&out_nei, &k](const edge_t &e){return out_nei.count(e.first)
                                                 && e.second <= k - 1;});
    } else { // case 4
        const auto &out_nei = out_neighbors.at(s);
        const auto &in_nei = in_neighbors.at(t);
        return any_of(out_nei.begin(), out_nei.end(), [&index, &in_nei, &k](const uint32_t &i){
            const auto &list = index.at(i);
            return any_of(list.begin(), list.end(),
                          [&in_nei, &k](const edge_t &e){return in_nei.count(e.first)
                                                     && e.second <= k - 2;});
        });
    }
}

void add_edge(const uint32_t s,
              const uint32_t t,
              graph_t &in_neighbors,
              graph_t &out_neighbors,
              cover_t &cover,
              index_t &index,
              const uint8_t k)
{
    out_neighbors[s].insert(t);
    in_neighbors[t].insert(s);
    const auto &graph = in_neighbors;
    if(!cover.count(s) && !cover.count(t)){
        if (out_neighbors[s].size() < out_neighbors[t].size() &&
                !any_of(out_neighbors[s].begin(), out_neighbors[s].end(),
                        [&cover](const uint32_t v){return cover.count(v); })){
            cover.insert(s);
            bfs(s, out_neighbors, cover, index, k);
        } else {
            cover.insert(t);
            bfs(t, out_neighbors, cover, index, k);
        }
    }

    if (cover.count(s) && cover.count(t) && (!index[s].count(t) || index[s][t] > 1)){
        queue<uint32_t> frontier;
        cover_t visited;

        frontier.push(s);
        visited.insert(s);
        index[s][t] = 1;
        uint8_t cur_level = 1, next_level = 0;
        for (uint8_t level = 1; level < k && !frontier.empty(); ){
            uint32_t u = frontier.front();
            frontier.pop();
            for (const auto &v : graph.at(u)){
                if (!visited.count(v)){
                    visited.insert(v);
                    frontier.push(v);
                    ++next_level;
                    if (cover.count(v)){
                        if (level + 1 < k - 2){
                            index[v][t] = k - 2;
                        } else {
                            index[v][t] = level + 1;
                        }
                    }
                }
            }
            if (!--cur_level){
                swap(cur_level, next_level);
                ++level;
            }
        }
    }
}

void remove_edge(const uint32_t s,
                 const uint32_t t,
                 graph_t &in_neighbors,
                 graph_t &out_neighbors,
                 const cover_t &cover,
                 index_t &index,
                 const uint8_t k)
{
    out_neighbors[s].erase(t);
    in_neighbors[t].erase(s);

    const auto &graph = in_neighbors;

    queue<uint32_t> frontier;
    cover_t visited;

    frontier.push(s);
    visited.insert(s);
    if (cover.count(s)){
        index.erase(s);
        bfs(s, out_neighbors, cover, index, k);
    }
    uint8_t cur_level = 1, next_level = 0;
    for (uint8_t level = 1; level < k && !frontier.empty(); ){
        uint32_t u = frontier.front();
        frontier.pop();
        for (const auto &v : graph.at(u)){
            if (!visited.count(v)){
                visited.insert(v);
                frontier.push(v);
                ++next_level;
                if (cover.count(v)){
                    index.erase(v);
                    bfs(v, out_neighbors, cover, index, k);
                }
            }
        }
        if (!--cur_level){
            swap(cur_level, next_level);
            ++level;
        }
    }
}

void add_vertex(const uint32_t v,
                graph_t &in_neighbors,
                graph_t &out_neighbors)
{
    in_neighbors[v];
    out_neighbors[v];
}

void remove_vertex(const uint32_t v,
                   graph_t &in_neighbors,
                   graph_t &out_neighbors,
                   cover_t &cover,
                   index_t &index,
                   const uint8_t k)
{
    in_neighbors.erase(v);
    out_neighbors.erase(v);
    cover.erase(v);
    index.erase(v);
}

int main() {
    string filename("k_reach_sample");
    ifstream fin(filename);
    graph_t out_neighbors, in_neighbors;
    index_t index;
    const uint8_t k = 3;

    uint32_t s, t;
    while (fin >> s >> t){
        out_neighbors[s].insert(t);
        out_neighbors[t];
        in_neighbors[t].insert(s);
        in_neighbors[s];
    }
    fin.close();

    cover_t cover({2, 4, 7, 9});

    construct_index(out_neighbors, cover, index, k);

    add_vertex(0, in_neighbors, out_neighbors);
    remove_edge(5, 7, in_neighbors, out_neighbors, cover, index, k);

    return 0;
}