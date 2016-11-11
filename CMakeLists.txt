cmake_minimum_required(VERSION 3.6)
project(DynamicKReachability)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -Wpedantic -Wextra -Werror")

find_package(Boost)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
endif()

set(SOURCE_FILES main.cpp)
add_executable(main ${SOURCE_FILES})

include_directories(common)
include_directories(indexes)

add_subdirectory(common)
add_subdirectory(indexes)
add_subdirectory(test)

target_link_libraries(main indexes common)