#include "lzss.h"

int main(int, char **)
{
    azgra::io::stream::OutMemoryBitStream bs;
    test_lzss("/mnt/d/codes/git/signal_compression/data/english.txt");
    test_lzss("/mnt/d/codes/git/signal_compression/data/english.txt");
    test_lzss("/mnt/d/codes/git/signal_compression/data/english.txt");
//    for (int i = 0; i < 10; ++i)
//    {
//        test_lzss("/mnt/d/codes/git/signal_compression/data/english.txt");
//        test_lzss("/mnt/d/codes/git/signal_compression/data/czech.txt");
//    }

    return 0;
}
