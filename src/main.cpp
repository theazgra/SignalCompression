//#include "lzss/lzss.h"
#include "move_to_front.h"

int main(int, char **)
{
//    test_lzss("../data/czech.txt");
//    test_lzss("../data/english.txt");
//    test_lzss("../data/french.txt");
//    test_lzss("../data/german.txt");
//    test_lzss("../data/hungarian.txt");

    const char *text = "abcddcbamnopponm";
    const auto result = apply_move_to_front_coding(text);

    std::stringstream ss;

    for (const auto x : result)
    {
        ss << x;
    }

    ss << '\n';
    puts(ss.str().c_str());

    return 0;
}
