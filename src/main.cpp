#include "fibonacci_coding.h"
#include <azgra/io/text_file_functions.h>
#include <charconv>
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

//    const double bpV = static_cast<double> (values.size()) / static_cast<double>(buffer.size() * 8.0);
    const double bpV =  static_cast<double>(buffer.size() * 8.0) / static_cast<double> (values.size());
    azgra::print_colorized(eq ? azgra::ConsoleColor::ConsoleColor_Green : azgra::ConsoleColor::ConsoleColor_Red,
                           "%s\nArrays are %s\nBitsPerValue: %.4f\n",
                           inputFile.data(), eq ? "equal" : "different",
                           bpV);
}

int main(int, char **)
{
    test_fib("/mnt/d/codes/git/signal_compression/data/uniform_8.txt");
    test_fib("/mnt/d/codes/git/signal_compression/data/uniform_16.txt");
    return 0;

}
