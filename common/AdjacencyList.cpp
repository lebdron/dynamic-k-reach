#include "AdjacencyList.h"

AdjacencyList::Iterator AdjacencyList::begin()
{
    return Iterator(out.begin(), out.end(), in.begin(), in.end());
}

AdjacencyList::Iterator AdjacencyList::end()
{
    return Iterator(out.end(), out.end(), in.end(), in.end());
}

AdjacencyList::ConstIterator AdjacencyList::begin() const
{
    return ConstIterator(out.begin(), out.end(), in.begin(), in.end());
}

AdjacencyList::ConstIterator AdjacencyList::end() const
{
    return ConstIterator(out.end(), out.end(), in.end(), in.end());
}
