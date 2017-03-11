#pragma once

#include <vector>
#include <string>
#include <cassert>
#include <cstdint>
#include <algorithm>
#include <unordered_map>
#include <queue>

using vertex_t = uint32_t;
using distance_t = uint8_t;
using degree_t = vertex_t;

using Index = std::unordered_map<vertex_t, std::vector<distance_t>>;

using DegreeQueue = std::priority_queue<std::pair<degree_t, vertex_t>>;

using DistanceQueue =  std::priority_queue<std::pair<distance_t, vertex_t>,
        std::vector<std::pair<distance_t, vertex_t>>,
        std::greater<std::pair<distance_t, vertex_t>>>;

const distance_t INF8 = 0x3f;

