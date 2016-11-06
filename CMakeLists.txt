cmake_minimum_required(VERSION 3.6)
project(dyn_k_reach)

add_definitions(-std=c++14)

#Boost
#[[find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})]]

set(SOURCE_FILES main.cpp)
add_executable(dyn_k_reach ${SOURCE_FILES})

include_directories(dynamic_k_reach)

add_subdirectory(dynamic_k_reach)
add_subdirectory(test)

target_link_libraries(dyn_k_reach dynamic_k_reach)