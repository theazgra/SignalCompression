#pragma once

#include <vector>
#include <type_traits>
#include <azgra/azgra.h>
#include <azgra/io/stream/memory_bit_stream.h>
#include <azgra/utilities/binary_converter.h>
#include <type_traits>


constexpr size_t fibonacci_sequence_length = 90;

constexpr auto generate_fibonacci_sequence()
{
    long n = fibonacci_sequence_length + 1;
    std::array fibN = std::array<size_t, fibonacci_sequence_length>();
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

constexpr auto fibonacci_sequence = generate_fibonacci_sequence();


template<typename T>
static std::pair<size_t, size_t> largest_lte_fib_num_index(const T target,
                                                           const size_t maxExclusiveIndex)
{
    const auto castedTarget = static_cast<size_t >(target);
    always_assert(maxExclusiveIndex > 1);
    const auto fromIndex = static_cast<size_t>(maxExclusiveIndex - 1);
    for (auto i = fromIndex; i >= 0; --i)
    {
        if (fibonacci_sequence[i] <= castedTarget)
        {
            return std::make_pair(fibonacci_sequence[i], i);
        }
    }
    always_assert(false && "Didn't find fibonacci number!");
    return std::make_pair(-1, -1);
}

void reset_fibonacci_indices(std::array<bool, fibonacci_sequence_length> &fibonacci_indices, const size_t upToIndex)
{
    for (size_t i = 0; i <= upToIndex; ++i)
    {
        fibonacci_indices[i] = false;
    }
}

template<typename T>
void encode_fibonacci(azgra::io::stream::OutMemoryBitStream &bitStream,
                      const std::vector<T> &values)
{
    static_assert(std::is_integral_v<T>);
    const size_t valueCount = values.size();
    bitStream.write_value(valueCount);

    std::array<bool, fibonacci_sequence_length> fibonacci_indices{};
    size_t maxFibIndex = 0;

    for (size_t valueIndex = 0; valueIndex < valueCount; ++valueIndex)
    {
        reset_fibonacci_indices(fibonacci_indices, maxFibIndex);
        maxFibIndex = 0;

        const T value = values[valueIndex];
        always_assert(value != 0);
        long remaining = static_cast<long> (value);
        size_t maxIndex = fibonacci_sequence_length;
        while (remaining > 0)
        {
            auto[value, index] = largest_lte_fib_num_index(remaining, maxIndex);
            maxFibIndex = std::max(maxFibIndex, index);

            fibonacci_indices[index] = true;
            remaining -= value;
        }
        for (size_t i = 0; i <= maxFibIndex; ++i)
        {
            bitStream << fibonacci_indices[i];
        }
        // Write terminating 1.
        bitStream << true;
    }
}

template<typename T>
std::vector<T> decode_fibonacci(azgra::io::stream::InMemoryBitStream &bitStream)
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
                        value += fibonacci_sequence[fibIndex];
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
