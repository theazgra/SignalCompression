#include <azgra/cli/cli_arguments.h>
#include "compressors.h"

int main(int argc, const char **argv)
{
    using namespace azgra::cli;
    CliArguments cli("compression_tool", "Tool used to test different lossless compression algorithms.");
    cli.print_help_on_parser_error();

    CliValueFlag<const char *> inputFileFlag("Input file", "Input file path", 'i', "input", true);
    CliValueFlag<int> compressionLevel("Level", "Level of compression", 'l', "level", false, 6);

    CliFlag gzip("Gzip method", "Gzip compression", '\0', "gzip");
    CliFlag lzma("Lzma method", "Lzma compression", '\0', "lzma");
    CliFlag bzip2("Bzip2 method", "Bzip2 compression", '\0', "bzip2");
    CliFlag repair("RePair method", "RePair compression", '\0', "repair");
    CliFlag repairImp("RePairImproved method", "RePairImproved compression", '\0', "repair-improved");

    CliFlagGroup compressionMethods("Compression method", {&gzip, &lzma, &bzip2, &repair, &repairImp},
                                    CliGroupMatchPolicy::CliGroupMatchPolicy_AtLeastOne);

    CliMethod testCompressionMethod("compress", "Compress the input file", {&inputFileFlag}, {&compressionLevel});
    cli.add_group(compressionMethods);
    cli.set_methods({&testCompressionMethod});

    if (!cli.parse(argc, argv))
    {
        return 1;
    }

    CompressionMethod method = CompressionMethod::None;
    if (gzip) method = CompressionMethod::GZIP;
    else if (lzma) method = CompressionMethod::LZMA;
    else if (bzip2) method = CompressionMethod::BZIP2;
    else if (repair) method = CompressionMethod::RePair;
    else if (repairImp) method = CompressionMethod::RePairImproved;

    const auto result = test_compression_method(method,inputFileFlag.value(), compressionLevel.value());

    fprintf(stdout, "File: %s\tCR: %.4f\tSpeed: %.4f MB/s\n", inputFileFlag.value(), result.compressionRatio, result.speed);

    return 0;
}
