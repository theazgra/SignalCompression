//#include "lzss/lzss.h"
#include "bwt.h"
#include <azgra/io/text_file_functions.h>
#include <azgra/io/binary_file_functions.h>
#include "lzw.h"
#include "entropy.h"
#include <azgra/matrix.h>
#include <sstream>

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

[[maybe_unused]] static void test_fcd(const std::vector<const char *> &files)
{
    auto dictionaries = std::vector<robin_hood::unordered_set<azgra::StringView>>(files.size());
    auto fileTexts = std::vector<std::string>(files.size());

    for (std::size_t i = 0; i < files.size(); ++i)
    {
        fileTexts[i] = azgra::io::read_text_file(files[i]);
        dictionaries[i] = get_lzw_dictionary(fileTexts[i]);
    }

    azgra::Matrix<azgra::f64> fcdMatrix(files.size(), files.size());

    std::stringstream ss;
    ss.precision(2);
    for (std::size_t row = 0; row < fcdMatrix.rows(); ++row)
    {
        for (std::size_t col = 0; col < fcdMatrix.cols(); ++col)
        {
            const auto fcd = calculate_fcd(dictionaries[row], dictionaries[col]);
            fcdMatrix.at(row, col) = fcd;
            ss << fcd << '\t';
        }
        ss << '\n';
    }
    puts(ss.str().c_str());
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "Provide input file for entropy calculation.\n");
        return 1;
    }
    const auto inputFile = argv[1];
    const auto input = azgra::io::read_text_file(inputFile);
    azgra::ByteArray inputData(input.begin(), input.end());
    const auto symbolCount = inputData.size();
    const auto entropy = calculate_entropy(inputData);

    fprintf(stdout, "Entropy of %s: %.4f; Symbol Count: %lu\n", inputFile, entropy, symbolCount);
//    const std::vector<const char *> files = {
//            "../data/similarity/000.txt",
//            "../data/similarity/010.txt",
//            "../data/similarity/020.txt",
//            "../data/similarity/030.txt",
//            "../data/similarity/040.txt",
//            "../data/similarity/050.txt",
//            "../data/similarity/060.txt",
//            "../data/similarity/070.txt",
//            "../data/similarity/080.txt",
//            "../data/similarity/090.txt"
//    };
//
//    test_fcd(files);

    return 0;
}
