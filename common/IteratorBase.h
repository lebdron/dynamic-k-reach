#pragma once

#include "common.h"

class IteratorBase : public std::iterator<std::input_iterator_tag,
        vertex_t,
        std::ptrdiff_t,
        const vertex_t *,
        const vertex_t &>
{
protected:
    using AdjacentIterator = Adjacent::const_iterator;

    AdjacentIterator a, a_end, b, b_end;

public:
    IteratorBase(AdjacentIterator a, AdjacentIterator a_end,
                 AdjacentIterator b, AdjacentIterator b_end);

    bool operator==(const IteratorBase &it) const;

    bool operator!=(const IteratorBase &it) const;

    virtual reference operator*() const = 0;
};
