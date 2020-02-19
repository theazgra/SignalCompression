#include "fibonacci_coding.h"
#include <azgra/io/text_file_functions.h>
#include <charconv>
#include <azgra/io/binary_file_functions.h>

std::vector<uint32_t> read_values(const azgra::BasicStringView<char> &file)
{
    //auto str = std::string(file.stri);
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

    encode_with_fib_sequence(encodedStream, values, fibSeq);

    const auto buffer = encodedStream.get_flushed_buffer();

    azgra::io::stream::InMemoryBitStream inStream(&buffer);

    const auto decodedValues = decode_with_fib_seq<uint32_t>(inStream, fibSeq);

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
//    const auto values8 = read_values("/mnt/d/codes/git/signal_compression/data/uniform_8.txt");
//    const auto values16 = read_values("/mnt/d/codes/git/signal_compression/data/uniform_16.txt");
//    //const auto values8 = read_values("/mnt/d/codes/git/signal_compression/data/test.txt");
//
//    const auto fibSeq = generate_fibonacci_sequence(100);
//
//    azgra::io::stream::OutMemoryBitStream u8EncodedStream;
//    azgra::io::stream::OutMemoryBitStream u16EncodedStream;
//
//    encode_with_fib_sequence(u8EncodedStream, values8, fibSeq);
//    encode_with_fib_sequence(u16EncodedStream, values16, fibSeq);
//
//    const auto u8Buffer = u8EncodedStream.get_flushed_buffer();
//    const auto u16Buffer = u16EncodedStream.get_flushed_buffer();
//    azgra::io::dump_bytes(u8Buffer, "u8.enc");
//    azgra::io::dump_bytes(u16Buffer, "u16.enc");
//
//
//    azgra::io::stream::InMemoryBitStream inU8Stream(&u8Buffer);
//    azgra::io::stream::InMemoryBitStream inU16Stream(&u16Buffer);
//
//    const auto decodedU8 = decode_with_fib_seq<uint32_t>(inU8Stream, fibSeq);
//    const auto decodedU16 = decode_with_fib_seq<uint32_t>(inU16Stream, fibSeq);
//
//
//    const bool eq = std::equal(values8.begin(), values8.end(), decodedU8.begin(), decodedU8.end());
//    const bool eq2 = std::equal(values16.begin(), values16.end(), decodedU16.begin(), decodedU16.end());
//
//    azgra::print_colorized(eq ? azgra::ConsoleColor::ConsoleColor_Green : azgra::ConsoleColor::ConsoleColor_Red,
//                           "U8 Arrays are %s\n", eq ? "equal" : "different");
//    azgra::print_colorized(eq ? azgra::ConsoleColor::ConsoleColor_Green : azgra::ConsoleColor::ConsoleColor_Red,
//                           "U16 Arrays are %s\n", eq ? "equal" : "different");


    return 0;

}
