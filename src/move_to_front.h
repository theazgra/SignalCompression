#pragma once

#include <algorithm>
#include <utility>
#include <sstream>
#include <azgra/azgra.h>
#include <azgra/span.h>
#include <azgra/collection/enumerable_functions.h>

struct MTFResult
{
    azgra::ByteArray alphabet{};
    std::vector<std::size_t> indices{};

    MTFResult() = default;

    MTFResult(azgra::ByteArray &&alphabet_, std::vector<std::size_t> &&indices_)
    {
        alphabet = std::move(alphabet_);
        indices = std::move(indices_);
    }

    MTFResult(const MTFResult &) = delete;
};

azgra::ByteArray get_alphabet_from_text(const azgra::ByteArray &data);

MTFResult encode_with_move_to_front(const azgra::ByteArray &data);

azgra::ByteArray decode_move_to_front(const MTFResult &mtf);

