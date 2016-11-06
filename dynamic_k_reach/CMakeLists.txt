project(dynamic_k_reach)

set(HEADER_FILES
        common.h
#        dynamic_k_reach.h
        dynamic_k_reach_v2.h
        dynamic_k_reach_v3.h)

set(SOURCE_FILES
#        dynamic_k_reach.cpp
        dynamic_k_reach_v2.cpp
        dynamic_k_reach_v3.cpp)

add_library(dynamic_k_reach STATIC ${SOURCE_FILES} ${HEADER_FILES})