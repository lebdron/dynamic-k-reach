project(test)

add_subdirectory(lib/gtest)

include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
add_executable(run_tests Mapper_tests.cpp Iterator_tests.cpp KReach_tests.cpp DynamicKReachReindex_tests.cpp DynamicKReach_tests.cpp)

target_link_libraries(run_tests gtest gtest_main)
target_link_libraries(run_tests indexes common)