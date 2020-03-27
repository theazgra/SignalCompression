//#include "lzss/lzss.h"
#include "bwt.h"
#include <azgra/io/text_file_functions.h>

int main(int, char **)
{
//    const char *inputFile = "../data/czech.txt";
//    std::string fileText = azgra::io::read_text_file(inputFile);
//    azgra::ByteArray fileTextBytes(fileText.begin(), fileText.end());
//    const auto symbolMap = get_symbols_info(fileTextBytes);
//    const auto entropy = calculate_entropy(symbolMap);
//    fprintf(stdout, "%s entropy = %.4f\n", inputFile, entropy);

    const char *testData = "swiss miss";
    const azgra::ByteSpan testDatSpan(reinterpret_cast<const azgra::byte *>(testData), strlen(testData));

    const auto bwtEncoded = apply_burrows_wheeler_transform(testDatSpan);
//    const auto moveToFrontIndices = apply_move_to_front_coding(bwtEncoded);
//
//    const auto symbolMap = get_symbols_info(moveToFrontIndices);
//    const auto entropy = calculate_entropy(symbolMap);
//    fprintf(stdout, "AfterBWT+MTF entropy = %.4f\n", entropy);


    return 0;
}
