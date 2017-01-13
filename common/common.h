#pragma once

#include <cassert>

#include <cstdint>
#include <utility>
#include <boost/functional/hash.hpp>

using Vertex = uint32_t;
using Weight = uint8_t;
using Edge = std::pair<Vertex, Vertex>;
using EdgeHash = boost::hash<Edge>;