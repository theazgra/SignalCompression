#include "huffman.h"
#include <azgra/io/text_file_functions.h>
#include <azgra/io/binary_file_functions.h>
#include <azgra/io/stream/in_binary_file_stream.h>
#include <iostream>

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

//    test_huffmann("../data/czech.txt");
//    test_huffmann("../data/german.txt");
//    test_huffmann("../data/english.txt");
//    test_huffmann("../data/french.txt");
//    test_huffmann("../data/hungarian.txt");

    std::map<char, SymbolInfo> map{
            {'1', SymbolInfo(1, 0.25f)},
            {'2', SymbolInfo(1, 0.20f)},
            {'3', SymbolInfo(1, 0.15f)},
            {'4', SymbolInfo(1, 0.15f)},
            {'5', SymbolInfo(1, 0.10f)},
            {'6', SymbolInfo(1, 0.10f)},
            {'7', SymbolInfo(1, 0.05f)},
    };

    Huffman huffman = build_fano_tree(map);
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

    return 0;

}
