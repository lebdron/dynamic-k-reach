cmake_minimum_required(VERSION 3.6)
project(dyn_k_reach)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

#Boost
find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

set(SOURCE_FILES main.cpp dynamic_k_reach.cpp)
add_executable(dyn_k_reach ${SOURCE_FILES})