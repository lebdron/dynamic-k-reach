cmake_minimum_required(VERSION 3.6)
project(dynamic_k_reach)

set(CMAKE_CXX_STANDARD 14)

set(BOOST_ROOT "/opt/boost/boost_1_63_0")
set(Boost_NO_BOOST_CMAKE ON)
find_package(Boost 1.63.0)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
endif()

add_library(common
        common/common.h
        common/Mapper.cpp common/Mapper.h
        common/WeightFunction.cpp common/WeightFunction.h
        common/AdjacencyList.cpp common/AdjacencyList.h
        common/Iterator.cpp common/Iterator.h
        common/Graph.cpp common/Graph.h)
include_directories(common)

add_library(index
        index/KReach.cpp index/KReach.h index/DynamicKReach.cpp index/DynamicKReach.h)
include_directories(index)

set(SOURCE_FILES main.cpp)
add_executable(dynamic_k_reach ${SOURCE_FILES})

target_link_libraries(index common)
target_link_libraries(dynamic_k_reach index common)
target_compile_options(dynamic_k_reach PRIVATE -Wall -Wextra -Wpedantic)