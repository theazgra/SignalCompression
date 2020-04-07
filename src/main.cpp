//#include "lzss/lzss.h"
#include "bwt.h"
#include <azgra/io/text_file_functions.h>
#include <azgra/io/binary_file_functions.h>
#include "lzw.h"

[[maybe_unused]] static void test_bwt_mtf_rle(const char *inputFile)
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
    fprintf(stdout, "Compression Ratio: %.4f\n", cr);

//     Test the decoding.
    const auto decodedBytes = decode_bwt_mtf_rle(encodedBytes);

    // Optionally save the result.
//    azgra::io::dump_bytes(decodedBytes, "result.txt");

    const bool eq = std::equal(fileTextBytes.begin(), fileTextBytes.end(), decodedBytes.begin(), decodedBytes.end());
    if (eq)
    {
        azgra::print_colorized(azgra::ConsoleColor::ConsoleColor_Green,
                               "Encode/Decode successful.\n");
    }
    else
    {
        azgra::print_colorized(azgra::ConsoleColor::ConsoleColor_Red,
                               "Encode/Decode Failed!\n");
    }
}

static void test_fcd(const char *fileA, const char *fileB)
{
    using namespace azgra::string;
    const std::string fileAText = azgra::io::read_text_file(fileA);
    const std::string fileBText = azgra::io::read_text_file(fileB);

    const auto aDict = get_lzw_dictionary(fileAText);
    const auto bDict = get_lzw_dictionary(fileBText);

    const auto FCD = calculate_fcd(aDict, bDict);
    fprintf(stdout, "FCD = %.4f\n", FCD);
}

int main(int, char **)
{
#if DEBUG
    test_fcd("/mnt/d/codes/git/signal_compression/data/similarity/000.txt",
             "/mnt/d/codes/git/signal_compression/data/similarity/010.txt");
#else
    test_fcd("../data/similarity/000.txt", "../data/similarity/010.txt");
#endif

    return 0;
}
