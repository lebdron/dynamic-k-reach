project(indexes)

include_directories(../common)

set(HEADER_FILES
        KReach.h
        DynamicKReachBase.h
        DynamicKReachReindex.h
        DynamicKReach.h)

set(SOURCE_FILES
        KReach.cpp
        DynamicKReachBase.cpp
        DynamicKReachReindex.cpp
        DynamicKReach.cpp)

add_library(indexes STATIC ${SOURCE_FILES} ${HEADER_FILES})

