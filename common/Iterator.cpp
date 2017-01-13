#include "Iterator.h"

Union::Union(AdjacentIterator cur1, AdjacentIterator end1, AdjacentIterator cur2, AdjacentIterator end2)
        : cur1(cur1), end1(end1), cur2(cur2), end2(end2) {
}

Union &Union::operator++() {
    if (cur1 != end1 && cur2 != end2){
        if (*cur1 == *cur2){
            ++cur1;
            ++cur2;
        }
        else if (*cur1 < *cur2){
            ++cur1;
        }
        else {
            ++cur2;
        }
    }
    else if (cur1 != end1){
        ++cur1;
    }
    else if (cur2 != end2){
        ++cur2;
    }
    return *this;
}

Union Union::operator++(int) {
    Union tmp(*this);
    ++(*this);
    return tmp;
}

bool Union::operator==(Union it) const {
    return cur1 == it.cur1 && end1 == it.end1 && cur2 == it.cur2 && end2 == it.end2;
}

bool Union::operator!=(Union it) const {
    return !(*this == it);
}

const Vertex &Union::operator*() const {
    if (cur1 != end1 && cur2 != end2){
        return *cur1 < *cur2 ? *cur1 : *cur2;
    }
    else if (cur1 != end1){
        return *cur1;
    }
    else {
        return *cur2;
    }
}

InclusiveIntersection::InclusiveIntersection(AdjacentIterator cur1, AdjacentIterator end1, AdjacentIterator cur2,
                                             AdjacentIterator end2)
        : cur1(cur1), end1(end1), cur2(cur2), end2(end2) {
    next();
}

InclusiveIntersection &InclusiveIntersection::operator++() {
    ++cur1;
    ++cur2;
    next();
    return *this;
}

InclusiveIntersection InclusiveIntersection::operator++(int) {
    InclusiveIntersection tmp(*this);
    ++(*this);
    return tmp;
}

bool InclusiveIntersection::operator==(InclusiveIntersection it) const {
    return cur1 == it.cur1 && end1 == it.end1 && cur2 == it.cur2 && end2 == it.end2;
}

bool InclusiveIntersection::operator!=(InclusiveIntersection it) const {
    return !(*this == it);
}

const Vertex &InclusiveIntersection::operator*() const {
    return *cur1;
}

void InclusiveIntersection::next() {
    while (cur1 != end1 && cur2 != end2 && *cur1 != *cur2){
        if (*cur1 < *cur2){
            ++cur1;
        }
        else {
            ++cur2;
        }
    }
    if (cur1 == end1 || cur2 == end2){
        cur1 = end1;
        cur2 = end2;
    }
}

ExclusiveIntersection::ExclusiveIntersection(Vertex v1, AdjacentIterator cur1, AdjacentIterator end1, Vertex v2,
                                             AdjacentIterator cur2, AdjacentIterator end2)
        : v1(v1), cur1(cur1), end1(end1), v2(v2), cur2(cur2), end2(end2) {
    next();
}

ExclusiveIntersection &ExclusiveIntersection::operator++() {
    ++cur1;
    ++cur2;
    next();
    return *this;
}

ExclusiveIntersection ExclusiveIntersection::operator++(int) {
    ExclusiveIntersection tmp(*this);
    ++(*this);
    return tmp;
}

bool ExclusiveIntersection::operator==(ExclusiveIntersection it) const {
    return v1 == it.v1 && cur1 == it.cur1 && end1 == it.end1 && v2 == it.v2 && cur2 == it.cur2 && end2 == it.end2;
}

bool ExclusiveIntersection::operator!=(ExclusiveIntersection it) const {
    return !(*this == it);
}

const Vertex &ExclusiveIntersection::operator*() const {
    return *cur1;
}

AdjacentIterator ExclusiveIntersection::first() const {
    return cur1;
}

AdjacentIterator ExclusiveIntersection::second() const {
    return cur2;
}

void ExclusiveIntersection::next() {
    while (cur1 != end1 && cur2 != end2 && (*cur1 != *cur2 || *cur1 == v1 || *cur2 == v2)){
        if (*cur1 < *cur2){
            ++cur1;
        }
        else {
            ++cur2;
        }
    }
    if (cur1 == end1 || cur2 == end2){
        cur1 = end1;
        cur2 = end2;
    }
}
