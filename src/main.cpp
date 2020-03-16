#include "lzss/lzss.h"

int main(int, char **)
{
//    test_lzss("/mnt/d/codes/git/signal_compression/data/test.txt");
    test_lzss("/mnt/d/codes/git/signal_compression/data/czech.txt");
    test_lzss("/mnt/d/codes/git/signal_compression/data/english.txt");
    test_lzss("/mnt/d/codes/git/signal_compression/data/french.txt");
    test_lzss("/mnt/d/codes/git/signal_compression/data/german.txt");
    test_lzss("/mnt/d/codes/git/signal_compression/data/hungarian.txt");

    return 0;
}
