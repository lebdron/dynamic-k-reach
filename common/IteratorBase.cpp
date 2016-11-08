#include "IteratorBase.h"

IteratorBase::IteratorBase(AdjacentIterator a, AdjacentIterator a_end,
                           AdjacentIterator b, AdjacentIterator b_end)
        : a(a), a_end(a_end), b(b), b_end(b_end)
{

}

bool IteratorBase::operator==(const IteratorBase &it) const
{
    return a == it.a && a_end == it.a_end && b == it.b && b_end == it.b_end;
}

bool IteratorBase::operator!=(const IteratorBase &it) const
{
    return !(*this == it);
}
