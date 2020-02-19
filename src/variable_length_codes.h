#pragma once

#include <vector>
#include <type_traits>
#include <azgra/azgra.h>
#include <azgra/collection/enumerable_functions.h>
#include <azgra/io/stream/memory_bit_stream.h>
#include <azgra/utilities/binary_converter.h>
#include <type_traits>

inline std::vector<size_t> generate_fibonacci_sequence(const size_t sequenceLength)
{
    always_assert(sequenceLength <= std::numeric_limits<long>::max());

    long n = sequenceLength + 1;
    std::vector<size_t> fibN(sequenceLength);
    size_t a = 0;
    size_t b = 1;
    size_t i = 0;
    while (n-- > 1)
    {
        size_t t = a;
        a = b;
        b += t;
        fibN[i++] = b;
    }
    return fibN;
}

template<typename T>
static std::pair<size_t, size_t> largest_lte_fib_num_index(const std::vector<size_t> &fibSeq,
                                                           const T target,
                                                           const size_t maxExclusiveIndex)
{
    const auto castedTarget = static_cast<size_t >(target);
    always_assert(maxExclusiveIndex > 1);
    const auto fromIndex = static_cast<size_t>(maxExclusiveIndex - 1);
    for (auto i = fromIndex; i >= 0; --i)
    {
        if (fibSeq[i] <= castedTarget)
        {
            return std::make_pair(fibSeq[i], i);
        }
    }
    always_assert(false && "Didn't find fibonacci number!");
    return std::make_pair(-1, -1);
}

template<typename T>
void encode_fibonacci(azgra::io::stream::OutMemoryBitStream &bitStream,
                      const std::vector<T> &values,
                      const std::vector<size_t> &fibSeq)
{
    static_assert(std::is_integral_v<T>);
    const size_t valueCount = values.size();
    bitStream.write_value(valueCount);

    for (size_t valueIndex = 0; valueIndex < valueCount; ++valueIndex)
    {
        const T value = values[valueIndex];
        always_assert(value != 0);
        long remaining = static_cast<long> (value);
        std::vector<size_t> fibIndices;
        size_t maxIndex = fibSeq.size();
        while (remaining > 0)
        {
            auto[value, index] = largest_lte_fib_num_index(fibSeq, remaining, maxIndex);
            fibIndices.push_back(index);
            remaining -= value;
        }
        const size_t maxFibIndex = azgra::collection::max(fibIndices.begin(), fibIndices.end());
        for (size_t i = 0; i <= maxFibIndex; ++i)
        {
            // Index is set write 1
            // TODO(Moravec): Why do we need std::find really?
            const bool bit = std::find(fibIndices.begin(), fibIndices.end(), i) != fibIndices.end();
            bitStream << bit;
        }
        // Write terminating 1.
        bitStream << true;
    }
}

template<typename T>
std::vector<T> decode_fibonacci(azgra::io::stream::InMemoryBitStream &bitStream,
                                const std::vector<size_t> &fibSeq)
{
    const auto expectedValueCount = bitStream.read_value<size_t>();
    std::vector<T> result(expectedValueCount);

    bool prevWasOne = false;
    std::vector<size_t> fibIndices;
    long fibIndex;
    for (size_t valueIndex = 0; valueIndex < expectedValueCount; ++valueIndex)
    {
        prevWasOne = false;
        fibIndices.clear();
        fibIndex = 0;


        while (true)
        {
            if (bitStream.read_bit())   // 1
            {
                if (prevWasOne)
                {
                    // If previous was one reset here and decode from indices.
                    T value = 0;
                    for (const size_t &fibIndex : fibIndices)
                    {
                        value += fibSeq[fibIndex];
                    }
                    result[valueIndex] = value;
                    break;
                }
                else
                {
                    prevWasOne = true;
                    fibIndices.push_back(fibIndex);
                }
            }
            else                        // 0
            {
                prevWasOne = false;
            }
            ++fibIndex;
        }

    }
    return result;
}


template<typename T>
void encode_elias_gamma(azgra::io::stream::OutMemoryBitStream &bitStream,
                        const std::vector<T> &values)
{
    static_assert(std::is_integral_v<T>);
    const size_t valueCount = values.size();
    bitStream.write_value(valueCount);
    size_t bitCount;
    for (const T value : values)
    {
        if (value == 1)
        {
            bitStream << true;
            continue;
        }
        const auto valueBits = azgra::to_binary_representation(value);
        bitCount = valueBits.size();

        long i = 0;

        // Find the first 1 in binary code.
        for (i = static_cast<long>(bitCount - 1); i >= 0; --i)
        {
            if (valueBits[i])
            {
                break;
            }
        }
        if (i == static_cast<long>(bitCount))
        { continue; }

        bitStream.write_replicated_bit(false, i);

        for (; i >= 0; --i)
        {
            bitStream.write_bit(valueBits[i]);
        }
    }
}

template<typename T>
std::vector<T> decode_elias_gamma(azgra::io::stream::InMemoryBitStream &bitStream)
{
    const auto expectedValueCount = bitStream.read_value<size_t>();
    std::vector<T> result(expectedValueCount);

    bool bit;
    size_t zeroCounter;
    for (size_t valueIndex = 0; valueIndex < expectedValueCount; ++valueIndex)
    {
        zeroCounter = 0;
        bit = bitStream.read_bit();
        if (bit)
        {
            result[valueIndex] = 1;
            continue;
        }
        else
        {
            zeroCounter += 1;
            while (true)
            {
                bit = bitStream.read_bit();
                if (bit)
                {
                    // TODO: Replace later with binary shift and or!
                    T value = 0;
                    value |= (1u << zeroCounter);
                    for (size_t readIndex = 0; readIndex < zeroCounter; ++readIndex)
                    {
                        // If read 1
                        if (bitStream.read_bit())
                        {
                            value |= (1u << (zeroCounter - 1 - readIndex));
                        }
                    }

                    result[valueIndex] = value;
                    break;
                }
                else
                {
                    ++zeroCounter;
                }
            }

        }

    }
    return result;
}
