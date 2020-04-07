#pragma once

#include <azgra/collection/enumerable_functions.h>
#include <azgra/collection/robin_hood.h>
#include <algorithm>

template<typename T>
struct SetIntersectionCounter
{
    size_t count = 0;

    using value_type = T;

//    struct value_type { template<typename T> value_type(const T&) { } };
    void push_back(const value_type &)
    {
        ++count;
    }
};

robin_hood::unordered_set<azgra::StringView> get_lzw_dictionary(const azgra::StringView &text);

azgra::f64 calculate_fcd(const robin_hood::unordered_set<azgra::StringView> &xDict,
                         const robin_hood::unordered_set<azgra::StringView> &yDict);