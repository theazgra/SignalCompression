//#include "lzss/lzss.h"
#include "move_to_front.h"
#include "entropy.h"
#include <azgra/io/text_file_functions.h>

int main(int, char **)
{
    const char *inputFile = "../data/czech.txt";
    std::string fileText = azgra::io::read_text_file(inputFile);
    const azgra::ByteSpan textDataSpan((azgra::byte*)fileText.c_str(), fileText.length());
    const auto symbolMap = get_symbols_info(textDataSpan);
    const auto entropy = calculate_entropy(symbolMap);
    fprintf(stdout, "%s entropy = %.4f\n", inputFile, entropy);

//    const char *text = "abcddcbamnopponm";
//    const auto result = apply_move_to_front_coding(text);
//    std::stringstream ss;
//    for (const auto x : result)
//    {
//        ss << x;
//    }
//    ss << '\n';
//    puts(ss.str().c_str());

    return 0;
}
