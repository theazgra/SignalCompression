//#include "lzss/lzss.h"
#include "bwt.h"
#include <azgra/io/text_file_functions.h>
#include <azgra/io/binary_file_functions.h>
#include "lzw.h"
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

static void test_fcd(const std::vector<const char *> &files)
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
    ss.precision(4);
    for (std::size_t row = 0; row < fcdMatrix.rows(); ++row)
    {
        for (std::size_t col = 0; col < fcdMatrix.cols(); ++col)
        {
            fcdMatrix.at(row, col) = calculate_fcd(dictionaries[row], dictionaries[col]);
            ss << fcdMatrix.at(row, col) << '\t';
        }
        ss << '\n';
    }
    puts(ss.str().c_str());

    //fprintf(stdout, "FCD = %.4f\n", FCD);
}

int main(int, char **)
{

    const std::vector<const char *> files = {
            "/mnt/d/codes/git/signal_compression/data/similarity/000.txt",
            "/mnt/d/codes/git/signal_compression/data/similarity/010.txt",
            "/mnt/d/codes/git/signal_compression/data/similarity/020.txt",
            "/mnt/d/codes/git/signal_compression/data/similarity/030.txt",
            "/mnt/d/codes/git/signal_compression/data/similarity/040.txt",
            "/mnt/d/codes/git/signal_compression/data/similarity/050.txt",
            "/mnt/d/codes/git/signal_compression/data/similarity/060.txt",
            "/mnt/d/codes/git/signal_compression/data/similarity/070.txt",
            "/mnt/d/codes/git/signal_compression/data/similarity/080.txt",
            "/mnt/d/codes/git/signal_compression/data/similarity/090.txt"
    };

    test_fcd(files);

    return 0;
}
