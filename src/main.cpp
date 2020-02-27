#include "huffman.h"
#include "variable_length_codes.h"
#include <azgra/io/text_file_functions.h>
#include <charconv>
#include <azgra/utilities/binary_converter.h>
#include <azgra/io/binary_file_functions.h>
#include <azgra/io/stream/in_binary_file_stream.h>
#include <iostream>

std::vector<uint32_t> read_values(const azgra::BasicStringView<char> &file)
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

void test_fib(azgra::BasicStringView<char> inputFile)
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

void test_elias_gamma(azgra::BasicStringView<char> inputFile)
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

void test_huffmann(azgra::BasicStringView<char> inputFile)
{
    const auto text = azgra::io::read_text_file(inputFile);
    const auto textView = azgra::StringView(text);
    auto symbols = get_string_symbols_info(textView);
    size_t totalSymbolCount = 0;
    for (const auto[symbol, info] : symbols)
    {
        totalSymbolCount += info.occurrenceCount;
    }
    const Huffman huffman = build_huffman_tree(symbols);


    azgra::io::stream::OutMemoryBitStream outBitStream;
    // We are passing symbols, so that we can write tree into the stream.
    huffman_encode(outBitStream, huffman, textView);

    const auto encodedBytes = outBitStream.get_flushed_buffer();
    azgra::io::dump_bytes(encodedBytes, "huffman.data");

    const auto originalByteCount = text.size();
    const auto encodedByteCount = encodedBytes.size();
    const float BPS = (static_cast<float>(encodedByteCount * 8)) / static_cast<float>(totalSymbolCount);
    fprintf(stdout, "Original size: %lu; Encoded size: %lu\n", originalByteCount, encodedByteCount);

    const auto inBuffer = azgra::io::stream::InBinaryFileStream("huffman.data").consume_whole_file();

    azgra::io::stream::InMemoryBitStream decodeStream(&inBuffer);
    std::string decoded = huffman_decode(decodeStream);


    azgra::StringView decodedView(decoded);
    azgra::io::write_text("decoded_text.txt", decodedView);
    bool eq = textView == decodedView;

    if (eq)
    {
        azgra::print_colorized(azgra::ConsoleColor::ConsoleColor_Green, "Huffman encode/decode OK. BPS: %.4f %s\n\n",
                               BPS,
                               inputFile.data());
    }
    else
    {
        azgra::print_colorized(azgra::ConsoleColor::ConsoleColor_Red, "Huffman encode/decode ERROR. %s\n\n", inputFile.data());

    }

//    std::map<char, SymbolInfo> map{
//            {'E', SymbolInfo(1, 0.5f)},
//            {'B', SymbolInfo(1, 0.4f)},
//            {'A', SymbolInfo(1, 0.3f)},
//            {'C', SymbolInfo(1, 0.2f)},
//            {'D', SymbolInfo(1, 0.1f)},
//    };
//
//    Huffman huffman = build_huffman_tree(map);
//
//    for (const auto &[symbol, code] :huffman.symbolCodes)
//    {
//        std::cout << symbol << ": ";
//        for (const auto b : code)
//        {
//            std::cout << (b ? 1 : 0);
//        }
//        std::cout << '\n';
//    }
}


int main(int, char **)
{
    /*
     *  1) TODO: Which of these codes cannot be Huffman codes for any probability assignment and why?
            a) {0, 10, 11}          SUM 2^-li = 1.0
            b) {00, 01, 10, 110}    SUM 2^-li = 0.875
            c) {01, 10}             SUM 2^-li = 0.5
        2) Classes of codes. Consider the code {0, 01}.
            a) Is it prefix code?                       [ NO], 0 is prefix of 01
            b) Is it uniquely decodable?                [YES] - extension is nonsingular
            c) It it nonsingular?                       [YES] - different codes

        3) Optimal word lengths.
            a) Can l=(1, 2, 2) be the word lengths of a binary Huffman code?        [YES] (2^-1)+(2^-2)+(2^-2) = 1 <= 1
            a) Can l=(2, 2, 3, 3) be the word lengths of a binary Huffman code?     [YES] (2^-2)+(2^-2)+(2^-3)+(2^-3) = 0.75 <= 1
     * */

    test_huffmann("../data/czech.txt");
    test_huffmann("../data/german.txt");
    test_huffmann("../data/english.txt");
    test_huffmann("../data/french.txt");
    test_huffmann("../data/hungarian.txt");

    return 0;

}
