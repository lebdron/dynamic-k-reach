project(test)

add_subdirectory(lib/gtest)

include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
add_executable(run_test neighbors_iterator_check.cpp)

target_link_libraries(run_test gtest gtest_main)
target_link_libraries(run_test dynamic_k_reach)