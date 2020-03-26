#pragma once

#include <algorithm>
#include <utility>
#include <sstream>
#include <azgra/azgra.h>
#include <azgra/collection/enumerable_functions.h>

std::vector<azgra::byte> get_alphabet_from_text(const azgra::StringView &text);

std::vector<std::size_t> apply_move_to_front_coding(const azgra::StringView &text);

