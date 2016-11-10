#pragma once

#include <cstdint>
#include <cstddef>
#include <cassert>
#include <algorithm>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <utility>
#include <type_traits>
#include <iterator>
#include <queue>
#include <boost/functional/hash.hpp>

typedef uint32_t vertex_t;

typedef uint8_t weight_t;

typedef std::pair<vertex_t, vertex_t> Edge;

typedef boost::hash<Edge> EdgeHash;

typedef std::set<vertex_t> Adjacent;