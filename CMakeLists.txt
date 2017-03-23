cmake_minimum_required(VERSION 3.8)
project(dynamic_kreach)

set(CMAKE_CXX_STANDARD 14)

find_package(CUDA REQUIRED)

list(APPEND CUDA_NVCC_FLAGS -gencode arch=compute_35,code=compute_35 -gencode arch=compute_35,code=sm_35 -gencode arch=compute_52,code=compute_52 -gencode arch=compute_52,code=sm_52 -std=c++11 -Xcompiler=-Wundef -O0 -g -Xcompiler=-Werror -lineinfo  --expt-extended-lambda -use_fast_math)

file(GLOB moderngpu_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/external/moderngpu/*)
cuda_add_library(moderngpu ${moderngpu_SOURCES})
target_include_directories(moderngpu PUBLIC external)

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

cuda_add_library(gpu_index
        gpu_index/context.h
        gpu_index/GPUGraph.h
        gpu_index/GPUKReach.h
        gpu_index/ScalableGPUKReach.h
        gpu_index/DynamicScalableGPUKReach.h
        gpu_index/DynamicGPUKReach.h
        gpu_index/context.cu
        gpu_index/GPUGraph.cu
        gpu_index/GPUKReach.cu
        gpu_index/DynamicGPUKReach.cu
        gpu_index/ScalableGPUKReach.cu
        gpu_index/DynamicScalableGPUKReach.cu
)
target_include_directories(gpu_index PUBLIC gpu_index)
target_link_libraries(gpu_index index moderngpu)

cuda_add_executable(main main.cu)
target_link_libraries(main index gpu_index)
target_compile_options(main PRIVATE -Wall -Wextra -Wpedantic)