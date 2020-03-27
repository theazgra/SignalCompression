//#include "lzss/lzss.h"
#include "bwt.h"
#include <azgra/io/text_file_functions.h>
#include <azgra/io/binary_file_functions.h>

int main(int, char **)
{
    // Move to front test.
//    const char *test = "abcddcbamnopponm";
//    std::size_t testSize = strlen(test);
//    azgra::ByteArray testBytes(testSize);
//    std::memcpy(testBytes.data(), test, testSize);
//    const auto mtfResult = encode_with_move_to_front(testBytes);
//    const auto mtfDecoded = decode_move_to_front(mtfResult);
//    const bool eq = std::equal(testBytes.begin(), testBytes.end(), mtfDecoded.begin(), mtfDecoded.end());

    const char *inputFile = "../data/czech.txt";
    std::string fileText = azgra::io::read_text_file(inputFile);
    azgra::ByteArray fileTextBytes(fileText.begin(), fileText.end());

    const auto entropyBefore = calculate_entropy(fileTextBytes);
    fprintf(stdout, "%s entropy = %.4f\n", inputFile, entropyBefore);
    const azgra::ByteSpan bwtInput(fileTextBytes.data(), fileTextBytes.size());

    const auto encodedBytes = encode_with_bwt_mtf_rle(bwtInput);

    const auto entropyAfter = calculate_entropy(encodedBytes);
    fprintf(stdout, "%s entropy after BWT+MTF = %.4f\n", inputFile, entropyAfter);

    const auto decodedBytes = decode_bwt_mtf_rle(encodedBytes);
    azgra::io::dump_bytes(decodedBytes, "result.txt");

    const bool eq = std::equal(fileTextBytes.begin(), fileTextBytes.end(), decodedBytes.begin(), decodedBytes.end());
    if (eq)
    {
        azgra::print_colorized(azgra::ConsoleColor::ConsoleColor_Green,
                               "Encode/Decode successful.\n");
    }
    else {
        azgra::print_colorized(azgra::ConsoleColor::ConsoleColor_Red,
                               "Encode/Decode Failed!\n");
    }

    return 0;
}
