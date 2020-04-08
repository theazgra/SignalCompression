#pragma once

#include <azgra/collection/enumerable_functions.h>
#include <azgra/collection/robin_hood.h>
#include <algorithm>

robin_hood::unordered_set<azgra::StringView> get_lzw_dictionary(const azgra::StringView &text);

azgra::f64 calculate_fcd(const robin_hood::unordered_set<azgra::StringView> &xDict,
                         const robin_hood::unordered_set<azgra::StringView> &yDict);