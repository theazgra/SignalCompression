#pragma once

#include <azgra/azgra.h>
#include "entropy.h"

constexpr std::size_t RLE_MAX_RUN = 255;
constexpr std::size_t RLE_MAX_LITERALS = 255;

template<typename T>
struct RLEPair
{
    std::size_t literalCount{0};
    std::vector<T> literals;

    std::size_t runLength{0};
    T runSymbol;
};


template<typename T>
std::vector<RLEPair<T>> rle_encode(const std::vector<T> &data)
{
    std::size_t dataSize = data.size();
    std::size_t inBufferIndex = 0;

    std::size_t literalCount = 0;
    std::array<T, RLE_MAX_LITERALS> literals;

    std::vector<RLEPair<T>> outBuffer;

    while (inBufferIndex < dataSize)
    {
        const T startValue = data[inBufferIndex];
        std::size_t repCount = 1;
        while ((repCount < (dataSize - inBufferIndex)) &&
               (repCount < RLE_MAX_RUN) &&
               (data[inBufferIndex + repCount] == startValue))
        {
            ++repCount;
        }

        if ((repCount > 1) || (literalCount == RLE_MAX_LITERALS))
        {
            RLEPair<T> pair;
            pair.literalCount = literalCount;
            pair.literals = std::vector<T>(literals.begin(), literals.begin() + literalCount);
            pair.runLength = repCount;
            pair.runSymbol = startValue;

            outBuffer.push_back(std::move(pair));

            literalCount = 0;
            inBufferIndex += repCount;
        }
        else
        {
            literals[literalCount++] = startValue;
            ++inBufferIndex;
        }
    }
    if (literalCount >= 1)
    {
        RLEPair<T> pair;
        pair.literalCount = literalCount;
        pair.literals = std::vector<T>(literals.begin(), literals.begin() + literalCount);
        outBuffer.push_back(std::move(pair));
    }
    assert(inBufferIndex == dataSize);
    outBuffer.shrink_to_fit();
    return outBuffer;
}

template<typename T>
std::vector<T> rle_decode(const std::vector<RLEPair<T>> &encodedData, const std::size_t decodedDataSize)
{
    std::vector<T> outBuffer(decodedDataSize);

    std::size_t outBufferIndex = 0;
    for (const auto &pair : encodedData)
    {
        for (std::size_t l = 0; l < pair.literalCount; ++l)
        {
            outBuffer[outBufferIndex++] = pair.literals[l];
        }
        for (std::size_t r = 0; r < pair.runLength; ++r)
        {
            outBuffer[outBufferIndex++] = pair.runSymbol;
        }
    }
    return outBuffer;
}