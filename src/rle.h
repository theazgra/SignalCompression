#pragma once

#include <azgra/azgra.h>

constexpr std::size_t RLE_MAX_RUN = 255;
constexpr std::size_t RLE_MAX_LITERALS = 255;

azgra::ByteArray rle_encode(const azgra::ByteArray &data);

azgra::ByteArray rle_decode(const azgra::ByteArray &encodedData, const std::size_t decodedDataSize);