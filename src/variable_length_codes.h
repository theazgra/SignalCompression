#pragma once

#include <vector>
#include <type_traits>
#include <azgra/azgra.h>
#include <azgra/io/stream/memory_bit_stream.h>
#include <azgra/utilities/binary_converter.h>
#include <type_traits>
#include <azgra/io/text_file_functions.h>
#include <charconv>
#include <azgra/io/binary_file_functions.h>
#include <azgra/io/stream/in_binary_file_stream.h>


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

static std::vector<uint32_t> read_values(const azgra::BasicStringView<char> &file)
{
    std::vector<uint32_t> values = azgra::io::parse_by_lines<uint32_t>(file, [](const azgra::string::SmartStringView<char> &line)
    {
        int value{};
        auto conversionResult = std::from_chars(line.begin(), line.end(), value);
        always_assert(conversionResult.ec != std::errc::invalid_argument);
        return static_cast<uint32_t > (value);
    });
    return values;
}

static void test_fib(azgra::BasicStringView<char> inputFile)
{
    const auto values = read_values(inputFile);

    azgra::io::stream::OutMemoryBitStream encodedStream;
    encode_fibonacci(encodedStream, values);

    const auto buffer = encodedStream.get_flushed_buffer();

    azgra::io::dump_bytes(buffer, "fibonacci.data");
    const auto inBuffer = azgra::io::stream::InBinaryFileStream("fibonacci.data").consume_whole_file();

    azgra::io::stream::InMemoryBitStream inStream(&inBuffer);
    const auto decodedValues = decode_fibonacci<uint32_t>(inStream);

    const bool eq = std::equal(values.begin(), values.end(), decodedValues.begin(), decodedValues.end());

    const double bpV = static_cast<double>(buffer.size() * 8.0) / static_cast<double> (values.size());

    if (!eq)
    {
        azgra::print_colorized(azgra::ConsoleColor::ConsoleColor_Red,
                               "Failed fibonacci code for %s\n",
                               inputFile.data());
    }
    else
    {
        azgra::print_colorized(azgra::ConsoleColor::ConsoleColor_Green,
                               "Fibonacci\t%s\t\tBitsPerSymbol = %.4f\n",
                               inputFile.data(), bpV);
    }
}

static void test_elias_gamma(azgra::BasicStringView<char> inputFile)
{
    const auto values = read_values(inputFile);

    azgra::io::stream::OutMemoryBitStream encodedStream;
    encode_elias_gamma(encodedStream, values);

    const auto buffer = encodedStream.get_flushed_buffer();

    azgra::io::dump_bytes(buffer, "elias.data");
    const auto inBuffer = azgra::io::stream::InBinaryFileStream("elias.data").consume_whole_file();

    azgra::io::stream::InMemoryBitStream inStream(&inBuffer);
    const auto decodedValues = decode_elias_gamma<uint32_t>(inStream);

    const bool eq = std::equal(values.begin(), values.end(), decodedValues.begin(), decodedValues.end());

    const double bpV = static_cast<double>(buffer.size() * 8.0) / static_cast<double> (values.size());

    if (!eq)
    {
        azgra::print_colorized(azgra::ConsoleColor::ConsoleColor_Red,
                               "Failed elias gamma code for %s\n",
                               inputFile.data());
    }
    else
    {
        azgra::print_colorized(azgra::ConsoleColor::ConsoleColor_Green,
                               "EliasGamma\t%s\t\tBitsPerSymbol = %.4f\n",
                               inputFile.data(), bpV);
    }
}