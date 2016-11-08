project(common)

set(HEADER_FILES
        common.h
        AdjacencyList.h
        Graph.h
        Index.h
        WeightFunction.h
        Mapper.h
        IteratorBase.h
        IntersectionIterator.h
        UnionIterator.h)

set(SOURCE_FILES
        AdjacencyList.cpp
        Graph.cpp
        Index.cpp
        WeightFunction.cpp
        Mapper.cpp
        IteratorBase.cpp
        IntersectionIterator.cpp
        UnionIterator.cpp)

add_library(common STATIC ${SOURCE_FILES} ${HEADER_FILES})