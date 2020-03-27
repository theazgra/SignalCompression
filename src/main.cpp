//#include "lzss/lzss.h"
#include "bwt.h"
#include <azgra/io/text_file_functions.h>
#include <azgra/io/binary_file_functions.h>

static void test_bwt_mtf_rle(const char *inputFile)
{
    std::string fileText = azgra::io::read_text_file(inputFile);
    azgra::ByteArray fileTextBytes(fileText.begin(), fileText.end());

    const auto entropyBefore = calculate_entropy(fileTextBytes);
    fprintf(stdout, "%s entropy = %.4f\n", inputFile, entropyBefore);

    const azgra::ByteSpan bwtInput(fileTextBytes.data(), fileTextBytes.size());

    const auto encodedBytes = encode_with_bwt_mtf_rle(bwtInput);

    const auto sizeBefore = fileTextBytes.size();
    const auto sizeAfter = encodedBytes.size();
    const double cr = static_cast<double>(sizeAfter) / static_cast<double>(sizeBefore);
    fprintf(stdout, "Compression Ratio: %.4f\n",cr);

    // Test the decoding.
//    const auto decodedBytes = decode_bwt_mtf_rle(encodedBytes);
//
//    // Optionally save the result.
////    azgra::io::dump_bytes(decodedBytes, "result.txt");
//
//    const bool eq = std::equal(fileTextBytes.begin(), fileTextBytes.end(), decodedBytes.begin(), decodedBytes.end());
//    if (eq)
//    {
//        azgra::print_colorized(azgra::ConsoleColor::ConsoleColor_Green,
//                               "Encode/Decode successful.\n");
//    }
//    else
//    {
//        azgra::print_colorized(azgra::ConsoleColor::ConsoleColor_Red,
//                               "Encode/Decode Failed!\n");
//    }
}

int main(int, char **)
{
    test_bwt_mtf_rle("../data/czech.txt");
    test_bwt_mtf_rle("../data/german.txt");
    test_bwt_mtf_rle("../data/english.txt");
    test_bwt_mtf_rle("../data/french.txt");
    test_bwt_mtf_rle("../data/hungarian.txt");

    return 0;
}
