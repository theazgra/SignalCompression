#include "variable_length_codes.h"
#include <azgra/io/text_file_functions.h>
#include <charconv>
#include <azgra/utilities/binary_converter.h>
#include <azgra/io/binary_file_functions.h>

std::vector<uint32_t> read_values(const azgra::BasicStringView<char> &file)
{
    std::vector<uint32_t> values = azgra::io::parse_by_lines<uint32_t>(file, [](const azgra::string::SmartStringView<char> &line)
    {
        int value;
        auto conversionResult = std::from_chars(line.begin(), line.end(), value);
        always_assert(conversionResult.ec != std::errc::invalid_argument);
        return static_cast<uint32_t > (value);
    });
    return values;
}

void test_fib(azgra::BasicStringView<char> inputFile)
{
    const auto fibSeq = generate_fibonacci_sequence(100);
    const auto values = read_values(inputFile);

    azgra::io::stream::OutMemoryBitStream encodedStream;
    encode_fibonacci(encodedStream, values, fibSeq);

    const auto buffer = encodedStream.get_flushed_buffer();
    azgra::io::stream::InMemoryBitStream inStream(&buffer);
    const auto decodedValues = decode_fibonacci<uint32_t>(inStream, fibSeq);

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

void test_elias_gamma(azgra::BasicStringView<char> inputFile)
{
    const auto values = read_values(inputFile);

    azgra::io::stream::OutMemoryBitStream encodedStream;
    encode_elias_gamma(encodedStream, values);

    const auto buffer = encodedStream.get_flushed_buffer();
    azgra::io::stream::InMemoryBitStream inStream(&buffer);
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

int main(int, char **)
{
    test_elias_gamma("../data/uniform_8.txt");
    test_fib("../data/uniform_8.txt");

    test_elias_gamma("../data/gausian_8.txt");
    test_fib("../data/gausian_8.txt");

    test_elias_gamma("../data/exponential_8.txt");
    test_fib("../data/exponential_8.txt");

    test_elias_gamma("../data/uniform_16.txt");
    test_fib("../data/uniform_16.txt");

    test_elias_gamma("../data/gausian_16.txt");
    test_fib("../data/gausian_16.txt");

    test_elias_gamma("../data/exponential_16.txt");
    test_fib("../data/exponential_16.txt");

    return 0;

}
