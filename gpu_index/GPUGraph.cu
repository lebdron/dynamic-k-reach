#include <sstream>
#include <fstream>
#include "GPUGraph.h"

using namespace std;

GPUGraph::GPUGraph() {

}

void GPUGraph::from_kreach(const std::string &filename) {
    stringstream ss;
    string line;

    ifstream fin(filename);
    assert(fin.is_open());

    getline(fin, line);
    ss = stringstream(line);
    ss >> num_vertices_ >> num_edges_;

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
    assert(counted_edges == num_edges_);

    for (auto &i : succ_) {
        sort(i.begin(), i.end());
    }
    for (auto &i : pred_) {
        sort(i.begin(), i.end());
    }

    // Forward
    h_vertices_.resize(num_vertices_ + 1);
    h_edges_.resize(num_edges_);
    for (vertex_t i = 0; i < num_vertices_; ++i) {
        h_vertices_.at(i + 1) = h_vertices_.at(i) + succ_.at(i).size();
        for (uint32_t j = 0; j < succ_.at(i).size(); ++j) {
            h_edges_.at(h_vertices_.at(i) + j) = succ_.at(i).at(j);
        }
    }

    d_vertices_ = mgpu::to_mem(h_vertices_, mgpu_context);
    d_edges_ = mgpu::to_mem(h_edges_, mgpu_context);

    // Backward
    h_rvertices_.resize(num_vertices_ + 1);
    h_redges_.resize(num_edges_);
    for (vertex_t i = 0; i < num_vertices_; ++i) {
        h_rvertices_.at(i + 1) = h_rvertices_.at(i) + pred_.at(i).size();
        for (uint32_t j = 0; j < pred_.at(i).size(); ++j) {
            h_redges_.at(h_rvertices_.at(i) + j) = pred_.at(i).at(j);
        }
    }

    d_rvertices_ = mgpu::to_mem(h_rvertices_, mgpu_context);
    d_redges_ = mgpu::to_mem(h_redges_, mgpu_context);
}

const std::vector<vertex_t> &GPUGraph::successors(vertex_t v) const {
    return succ_.at(v);
}

const std::vector<vertex_t> &GPUGraph::predecessors(vertex_t v) const {
    return pred_.at(v);
}

const std::vector<degree_t> &GPUGraph::degree() const {
    return deg_;
}

degree_t GPUGraph::num_vertices() const {
    return num_vertices_;
}

void GPUGraph::insert_edge(vertex_t s, vertex_t t) {
    // TODO check existence
    ++num_edges_;
    succ_.at(s).push_back(t);
    sort(succ_.at(s).begin(), succ_.at(s).end());
    pred_.at(t).push_back(s);
    sort(pred_.at(t).begin(), pred_.at(t).end());

    ++deg_.at(s);
    ++deg_.at(t);

    // Forward
    if (h_edges_.size() < num_edges_){
        h_edges_.insert(h_edges_.begin() + h_vertices_.at(s + 1), t);
    }
    else {
        copy_backward(h_edges_.begin() + h_vertices_.at(s + 1), 
            h_edges_.begin() + h_vertices_.at(num_vertices_), 
            h_edges_.begin() + h_vertices_.at(num_vertices_) + 1);

        h_edges_.at(h_vertices_.at(s + 1)) = t;
    }
    for (vertex_t i = s + 1; i < num_vertices_ + 1; ++i) {
        ++h_vertices_.at(i);
    }
    sort(h_edges_.begin() + h_vertices_.at(s), h_edges_.begin() + h_vertices_.at(s + 1));
    
    degree_t* vertices = d_vertices_.data() + s + 1;
    mgpu::transform([=]MGPU_DEVICE(int index) {
        ++vertices[index];
    }, num_vertices_ + 1 - (s + 1), mgpu_context);
    if (d_edges_.size() < h_edges_.size()) {
        d_edges_ = mgpu::to_mem(h_edges_, mgpu_context);
    }
    else {
        mgpu::htod(d_edges_.data() + h_vertices_.at(s), h_edges_.data() + h_vertices_.at(s), h_edges_.size() - h_vertices_.at(s));
    }

    // Backward
    if (h_redges_.size() < num_edges_){
        h_redges_.insert(h_redges_.begin() + h_rvertices_.at(t + 1), s);
    }
    else {
        copy_backward(h_redges_.begin() + h_rvertices_.at(t + 1), 
            h_redges_.begin() + h_rvertices_.at(num_vertices_), 
            h_redges_.begin() + h_rvertices_.at(num_vertices_) + 1);

        h_redges_.at(h_rvertices_.at(t + 1)) = s;
    }
    for (vertex_t i = t + 1; i < num_vertices_ + 1; ++i) {
        ++h_rvertices_.at(i);
    }
    sort(h_redges_.begin() + h_rvertices_.at(t), h_redges_.begin() + h_rvertices_.at(t + 1));
    
    degree_t* rvertices = d_rvertices_.data() + t + 1;
    mgpu::transform([=]MGPU_DEVICE(int index) {
        ++rvertices[index];
    }, num_vertices_ + 1 - (t + 1), mgpu_context);
    if (d_redges_.size() < h_redges_.size()) {
        d_redges_ = mgpu::to_mem(h_redges_, mgpu_context);
    }
    else {
        mgpu::htod(d_redges_.data() + h_rvertices_.at(t), h_redges_.data() + h_rvertices_.at(t), h_redges_.size() - h_rvertices_.at(t));
    }
}

void GPUGraph::remove_edge(vertex_t s, vertex_t t) {
    // TODO check existence
    --num_edges_;
    succ_.at(s).erase(find(succ_.at(s).begin(), succ_.at(s).end(), t));
    pred_.at(t).erase(find(pred_.at(t).begin(), pred_.at(t).end(), s));

    --deg_.at(s);
    --deg_.at(t);

    // Forward
    auto removed = find(h_edges_.begin() + h_vertices_.at(s), h_edges_.begin() + h_vertices_.at(s + 1), t) - h_edges_.begin();
    copy(h_edges_.begin() + removed + 1, h_edges_.begin() + h_vertices_.at(num_vertices_), h_edges_.begin() + removed);
    for (vertex_t i = s + 1; i < num_vertices_ + 1; ++i) {
        --h_vertices_.at(i);
    }

    degree_t* vertices = d_vertices_.data() + s + 1;
    mgpu::transform([=]MGPU_DEVICE(int index) {
        --vertices[index];
    }, num_vertices_ + 1 - (s + 1), mgpu_context);
    mgpu::htod(d_edges_.data() + h_vertices_.at(s), h_edges_.data() + h_vertices_.at(s), num_edges_ - h_vertices_.at(s));

    // Backward
    auto rremoved = find(h_redges_.begin() + h_rvertices_.at(t), h_redges_.begin() + h_rvertices_.at(t + 1), s) - h_redges_.begin();
    copy(h_redges_.begin() + rremoved + 1, h_redges_.begin() + h_rvertices_.at(num_vertices_), h_redges_.begin() + rremoved);
    for (vertex_t i = t + 1; i < num_vertices_ + 1; ++i) {
        --h_rvertices_.at(i);
    }

    degree_t* rvertices = d_rvertices_.data() + t + 1;
    mgpu::transform([=]MGPU_DEVICE(int index) {
        --rvertices[index];
    }, num_vertices_ + 1 - (t + 1), mgpu_context);
    mgpu::htod(d_redges_.data() + h_rvertices_.at(t), h_redges_.data() + h_rvertices_.at(t), num_edges_ - h_rvertices_.at(t));
}

void GPUGraph::remove_vertex(vertex_t v) {
    for (const auto &i : pred_.at(v)) {
        succ_.at(i).erase(find(succ_.at(i).begin(), succ_.at(i).end(), v));
        --deg_.at(i);
        --num_edges_;
    }
    for (const auto &i : succ_.at(v)) {
        pred_.at(i).erase(find(pred_.at(i).begin(), pred_.at(i).end(), v));
        --deg_.at(i);
        --num_edges_;
    }
    deg_.at(v) = 0;

    succ_.at(v).clear();
    pred_.at(v).clear();

    // Forward
    for (vertex_t i = 0; i < num_vertices_; ++i) {
        h_vertices_.at(i + 1) = h_vertices_.at(i) + succ_.at(i).size();
        for (uint32_t j = 0; j < succ_.at(i).size(); ++j) {
            h_edges_.at(h_vertices_.at(i) + j) = succ_.at(i).at(j);
        }
    }
    mgpu::htod(d_vertices_.data(), h_vertices_.data(), num_vertices_ + 1);
    mgpu::htod(d_edges_.data(), h_edges_.data(), num_edges_);

    // Backward
    for (vertex_t i = 0; i < num_vertices_; ++i) {
        h_rvertices_.at(i + 1) = h_rvertices_.at(i) + pred_.at(i).size();
        for (uint32_t j = 0; j < pred_.at(i).size(); ++j) {
            h_redges_.at(h_rvertices_.at(i) + j) = pred_.at(i).at(j);
        }
    }
    mgpu::htod(d_rvertices_.data(), h_rvertices_.data(), num_vertices_ + 1);
    mgpu::htod(d_redges_.data(), h_redges_.data(), num_edges_);
}

const std::vector<std::vector<vertex_t>> &GPUGraph::graph() const {
    return succ_;
}

const std::vector<std::vector<vertex_t>> &GPUGraph::reverse_graph() const {
    return pred_;
}

void GPUGraph::compute_degree() {
    for (vertex_t i = 0; i < num_vertices_; ++i) {
        deg_.at(i) = (degree_t)succ_.at(i).size() + (degree_t)pred_.at(i).size();
    }
}

void GPUGraph::insert_vertex(vertex_t v, const std::vector<vertex_t> &out, const std::vector<vertex_t> &in) {
    succ_.at(v).insert(succ_.at(v).end(), out.begin(), out.end());
    sort(succ_.at(v).begin(), succ_.at(v).end());
    pred_.at(v).insert(pred_.at(v).end(), in.begin(), in.end());
    sort(pred_.at(v).begin(), pred_.at(v).end());

    deg_.at(v) += out.size() + in.size();
    for (const auto &i : out){
        pred_.at(i).push_back(v);
        sort(pred_.at(i).begin(), pred_.at(i).end());
        ++deg_.at(i);
        ++num_edges_;
    }
    for (const auto &i : in){
        succ_.at(i).push_back(v);
        sort(succ_.at(i).begin(), succ_.at(i).end());
        ++deg_.at(i);
        ++num_edges_;
    }

    // Forward
    if (h_edges_.size() < num_edges_){
        h_edges_.resize(num_edges_);
    }
    for (vertex_t i = 0; i < num_vertices_; ++i) {
        h_vertices_.at(i + 1) = h_vertices_.at(i) + succ_.at(i).size();
        for (uint32_t j = 0; j < succ_.at(i).size(); ++j) {
            h_edges_.at(h_vertices_.at(i) + j) = succ_.at(i).at(j);
        }
    }
    mgpu::htod(d_vertices_.data(), h_vertices_.data(), num_vertices_ + 1);
    if (d_edges_.size() < h_edges_.size()) {
        d_edges_ = mgpu::to_mem(h_edges_, mgpu_context);
    }
    else {
        mgpu::htod(d_edges_.data(), h_edges_.data(), num_edges_);
    }

    // Backward
    if (h_redges_.size() < num_edges_){
        h_redges_.resize(num_edges_);
    }
    for (vertex_t i = 0; i < num_vertices_; ++i) {
        h_rvertices_.at(i + 1) = h_rvertices_.at(i) + pred_.at(i).size();
        for (uint32_t j = 0; j < pred_.at(i).size(); ++j) {
            h_redges_.at(h_rvertices_.at(i) + j) = pred_.at(i).at(j);
        }
    }
    mgpu::htod(d_rvertices_.data(), h_rvertices_.data(), num_vertices_ + 1);
    if (d_redges_.size() < h_redges_.size()) {
        d_redges_ = mgpu::to_mem(h_redges_, mgpu_context);
    }
    else {
        mgpu::htod(d_redges_.data(), h_redges_.data(), num_edges_);
    }
}


const std::vector<degree_t>& GPUGraph::vertices() const {
    return h_vertices_;
}

const std::vector<vertex_t>& GPUGraph::edges() const {
    return h_edges_;
}

const degree_t* GPUGraph::gpu_vertices() const {
    return d_vertices_.data();
}

const vertex_t* GPUGraph::gpu_edges() const {
    return d_edges_.data();
}

const std::vector<degree_t>& GPUGraph::rvertices() const {
    return h_rvertices_;
}

const std::vector<vertex_t>& GPUGraph::redges() const {
    return h_redges_;
}

const degree_t* GPUGraph::gpu_rvertices() const {
    return d_rvertices_.data();
}

const vertex_t* GPUGraph::gpu_redges() const {
    return d_redges_.data();
}