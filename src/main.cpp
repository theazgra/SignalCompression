#include "lzss.h"

int main(int, char **)
{
    test_lzss("/mnt/d/codes/git/signal_compression/data/english.txt");
    //test_lzss("/mnt/d/codes/git/signal_compression/data/czech.txt");

    return 0;
}
