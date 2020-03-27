#include "rle.h"

azgra::ByteArray rle_encode(const azgra::ByteArray &data)
{
    std::size_t literalCount = 0;
    std::array<azgra::byte, RLE_MAX_LITERALS> literals;

    std::size_t dataSize = data.size();
    std::size_t inBufferIndex = 0;

    azgra::ByteArray outBuffer;
    outBuffer.reserve(1024);

    while (inBufferIndex < dataSize)
    {
        azgra::byte startValue = data[inBufferIndex];
        std::size_t repCount = 1;
        while ((repCount < (dataSize - inBufferIndex)) &&
               (repCount < RLE_MAX_RUN) &&
               (data[inBufferIndex + repCount] == startValue))
        {
            ++repCount;
        }
        if ((repCount > 1) || (literalCount == RLE_MAX_LITERALS))
        {
            outBuffer.push_back(static_cast<azgra::byte>(literalCount));
            for (std::size_t lIndex = 0; lIndex < literalCount; ++lIndex)
            {
                outBuffer.push_back(literals[lIndex]);
            }
            literalCount = 0;

            outBuffer.push_back(repCount);
            outBuffer.push_back(startValue);

            inBufferIndex += repCount;
        }
        else
        {
            literals[literalCount++] = startValue;
            ++inBufferIndex;
        }
    }
    assert(inBufferIndex == dataSize);
    outBuffer.shrink_to_fit();
    return outBuffer;
}

azgra::ByteArray rle_decode(const azgra::ByteArray &encodedData, const std::size_t decodedDataSize)
{
    azgra::ByteArray outBuffer(decodedDataSize);

    std::size_t inBufferSize = encodedData.size();
    std::size_t inBufferIndex = 0;
    std::size_t outBufferIndex = 0;

    int literalCount;
    int repCount;
    azgra::byte repValue;
    while (inBufferIndex < inBufferSize)
    {
        // Read literal count;
        literalCount = encodedData[inBufferIndex++];
        while (literalCount--)
        {
            outBuffer[outBufferIndex++] = encodedData[inBufferIndex++];
        }

        repCount = encodedData[inBufferIndex++];
        repValue = encodedData[inBufferIndex++];

        while (repCount--)
        {
            outBuffer[outBufferIndex++] = repValue;
        }
    }
    return outBuffer;
}
