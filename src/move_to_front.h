#pragma once

#include <algorithm>
#include <utility>
#include <sstream>
#include <azgra/azgra.h>
#include <azgra/span.h>
#include <azgra/collection/enumerable_functions.h>
#include "rle.h"

struct MTFResult
{
    azgra::ByteArray alphabet{};
    std::vector<RLEPair<std::size_t>> rleEncodedIndices{};
    std::size_t indicesCount;

    MTFResult() = default;

    MTFResult(azgra::ByteArray &&alphabet_, std::vector<RLEPair<std::size_t>> &&rleIndices, const std::size_t indicesCount_)
    {
        alphabet = std::move(alphabet_);
        rleEncodedIndices = std::move(rleIndices);
        indicesCount = indicesCount_;
    }

    MTFResult(const MTFResult &) = delete;
};

azgra::ByteArray get_alphabet_from_text(const azgra::ByteArray &data);

MTFResult encode_with_move_to_front(const azgra::ByteArray &data);

azgra::ByteArray decode_move_to_front(const MTFResult &mtf);

