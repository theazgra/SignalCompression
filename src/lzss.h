#pragma once

#include "lz_tree.h"
#include "ring_buffer.h"

template<typename T>
struct window
{
    T *start;
    T *end;
    std::size_t size;

    explicit window(T *start_, T *end_, std::size_t size_) : start(start_), end(end_), size(size_)
    {
        always_assert(size == (end - start));
    }
};


void test();
void test2();