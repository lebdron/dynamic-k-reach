#include <sstream>
#include <fstream>
#include "Graph.h"

using namespace std;

Graph::Graph() {

}

void Graph::from_kreach(const std::string &filename) {
    stringstream ss;
    string line;

    ifstream fin(filename);
    assert(fin.is_open());

    size_t num_edges;
    getline(fin, line);
    ss = stringstream(line);
    ss >> num_vertices_ >> num_edges;

    succ_.resize(num_vertices_);
    pred_.resize(num_vertices_);
    deg_.resize(num_vertices_);

    vertex_t current_vertex = 0;
    size_t counted_edges = 0;
    for (; getline(fin, line);) {
        ss = stringstream(line);
        ss >> current_vertex;
        vertex_t num_neighbors;
        ss >> num_neighbors;
        for (vertex_t neighbor; ss >> neighbor;) {
            succ_.at(current_vertex).push_back(neighbor);
            pred_.at(neighbor).push_back(current_vertex);
            ++counted_edges;
        }
    }

    assert(current_vertex + 1 == num_vertices_);
    assert(counted_edges == num_edges);

    for (auto &i : succ_) {
        sort(i.begin(), i.end());
    }
    for (auto &i : pred_) {
        sort(i.begin(), i.end());
    }
}

const std::vector<vertex_t> &Graph::successors(vertex_t v) const {
    return succ_.at(v);
}

const std::vector<vertex_t> &Graph::predecessors(vertex_t v) const {
    return pred_.at(v);
}

const std::vector<degree_t> &Graph::degree() const {
    return deg_;
}

degree_t Graph::num_vertices() const {
    return num_vertices_;
}

void Graph::insert_edge(vertex_t s, vertex_t t) {
    // TODO check existence
    succ_.at(s).push_back(t);
    sort(succ_.at(s).begin(), succ_.at(s).end());
    pred_.at(t).push_back(s);
    sort(pred_.at(t).begin(), pred_.at(t).end());

    ++deg_.at(s);
    ++deg_.at(t);
}

void Graph::remove_edge(vertex_t s, vertex_t t) {
    // TODO check existence
    succ_.at(s).erase(find(succ_.at(s).begin(), succ_.at(s).end(), t));
    pred_.at(t).erase(find(pred_.at(t).begin(), pred_.at(t).end(), s));

    --deg_.at(s);
    --deg_.at(t);
}

void Graph::remove_vertex(vertex_t v) {
    for (const auto &i : pred_.at(v)) {
        succ_.at(i).erase(find(succ_.at(i).begin(), succ_.at(i).end(), v));
        --deg_.at(i);
    }
    for (const auto &i : succ_.at(v)) {
        pred_.at(i).erase(find(pred_.at(i).begin(), pred_.at(i).end(), v));
        --deg_.at(i);
    }
    deg_.at(v) = 0;

    succ_.at(v).clear();
    pred_.at(v).clear();
}

const std::vector<std::vector<vertex_t>> &Graph::graph() const {
    return succ_;
}

const std::vector<std::vector<vertex_t>> &Graph::reverse_graph() const {
    return pred_;
}

void Graph::compute_degree() {
    for (vertex_t i = 0; i < num_vertices_; ++i) {
        deg_.at(i) = (degree_t)succ_.at(i).size() + (degree_t)pred_.at(i).size();
    }
}

void Graph::insert_vertex(vertex_t v, const std::vector<vertex_t> &out, const std::vector<vertex_t> &in) {
    succ_.at(v).insert(succ_.at(v).end(), out.begin(), out.end());
    sort(succ_.at(v).begin(), succ_.at(v).end());
    pred_.at(v).insert(pred_.at(v).end(), in.begin(), in.end());
    sort(pred_.at(v).begin(), pred_.at(v).end());

    deg_.at(v) += out.size() + in.size();
    for (const auto &i : out){
        pred_.at(i).push_back(v);
        sort(pred_.at(i).begin(), pred_.at(i).end());
        ++deg_.at(i);
    }
    for (const auto &i : in){
        succ_.at(i).push_back(v);
        sort(succ_.at(i).begin(), succ_.at(i).end());
        ++deg_.at(i);
    }
}
