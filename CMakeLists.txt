cmake_minimum_required(VERSION 3.6)
project(dynamic_kreach)

set(CMAKE_CXX_STANDARD 14)

add_library(index
        index/common.h
        index/Graph.h
        index/AbstractKReach.h
        index/KReach.h
        index/DynamicKReach.h
        index/ScalableKReach.h
        index/DynamicScalableKReach.h
        index/Graph.cpp
        index/KReach.cpp
        index/DynamicKReach.cpp
        index/ScalableKReach.cpp
        index/DynamicScalableKReach.cpp
        )
target_include_directories(index PUBLIC index)
target_compile_options(index PRIVATE -Wall -Wextra -Wpedantic)

add_executable(main
        main.cpp
        )
target_link_libraries(main index)
target_compile_options(main PRIVATE -Wall -Wextra -Wpedantic)