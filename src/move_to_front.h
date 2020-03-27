#pragma once

#include <algorithm>
#include <utility>
#include <sstream>
#include <azgra/azgra.h>
#include <azgra/span.h>
#include <azgra/collection/enumerable_functions.h>

azgra::ByteArray get_alphabet_from_text(const azgra::ByteArray &data);

std::vector<std::size_t> apply_move_to_front_coding(const azgra::ByteArray &data);

