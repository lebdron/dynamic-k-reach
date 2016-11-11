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

bool AdjacencyList::operator==(const AdjacencyList &adj) const {
    return out == adj.out && in == adj.in;
}

bool AdjacencyList::operator!=(const AdjacencyList &adj) const
{
    return !(*this == adj);
}

size_t AdjacencyList::degree() const
{
    return out.size() + in.size();
}

void AdjacencyList::clear()
{
    out.clear();
    in.clear();
}

bool AdjacencyList::empty() const
{
    return degree() == 0;
}
