#pragma once

#include <vector>
#include <type_traits>
#include <azgra/azgra.h>
#include <azgra/collection/enumerable_functions.h>
#include <azgra/io/stream/memory_bit_stream.h>

inline std::vector<size_t> generate_fibonacci_sequence(const size_t N)
{
    always_assert(N <= std::numeric_limits<long>::max());

    long n = N + 1;
    std::vector<size_t> fibN(N);
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
    always_assert(maxExclusiveIndex > 1);
    const auto fromIndex = static_cast<size_t>(maxExclusiveIndex - 1);
    for (auto i = fromIndex; i >= 0; --i)
    {
        if (fibSeq[i] <= target)
        {
            return std::make_pair(fibSeq[i], i);
        }
    }
    always_assert(false && "Didn't find fibonacci number!");
    return std::make_pair(-1, -1);
}

template<typename T>
void encode_with_fib_sequence(azgra::io::stream::OutMemoryBitStream &bitStream,
                              const std::vector<T> &values,
                              const std::vector<size_t> &fibSeq)
{
    static_assert(std::is_integral_v<T>);
    const size_t valueCount = values.size();
    bitStream.write_value(valueCount);

    for (int i = 0; i < valueCount; ++i)
    {
        const T value = values[i];
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
            const bool bit = std::find(fibIndices.begin(), fibIndices.end(), i) != fibIndices.end();
            bitStream << bit;
        }
        // Write terminating 1.
        bitStream << true;
    }

//    for (const T &value : values)
//    {
//        always_assert(value != 0);
//        long remaining = static_cast<long> (value);
//        std::vector<size_t> fibIndices;
//        size_t maxIndex = fibSeq.size();
//        while (remaining > 0)
//        {
//            auto[value, index] = largest_lte_fib_num_index(fibSeq, remaining, maxIndex);
//            fibIndices.push_back(index);
//            remaining -= value;
//        }
//        const size_t maxFibIndex = azgra::collection::max(fibIndices.begin(), fibIndices.end());
//        for (size_t i = 0; i <= maxFibIndex; ++i)
//        {
//            // Index is set write 1
//            const bool bit = std::find(fibIndices.begin(), fibIndices.end(), i) != fibIndices.end();
//            bitStream << bit;
//        }
//        // Write terminating 1.
//        bitStream << true;
//    }
}

template<typename T>
std::vector<T> decode_with_fib_seq(azgra::io::stream::InMemoryBitStream &bitStream,
                                   const std::vector<size_t> &fibSeq)
{
    const auto expectedValueCount = bitStream.read_value<size_t>();
    std::vector<T> result(expectedValueCount);

    bool prevWasOne = false;
    std::vector<size_t> fibIndices;
    long fibIndex;
    for (int valueIndex = 0; valueIndex < expectedValueCount; ++valueIndex)
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