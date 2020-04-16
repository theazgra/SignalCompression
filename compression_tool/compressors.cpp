#include "compressors.h"

using namespace azgra;

const char *compression_method_name(const CompressionMethod method)
{
    switch (method)
    {
        case CompressionMethod::GZIP:
            return AZGRA_NAME_OF(GZIP);
        case CompressionMethod::LZMA:
            return AZGRA_NAME_OF(LZMA);
        case CompressionMethod::BZIP2:
            return AZGRA_NAME_OF(BZIP2);
        case CompressionMethod::RePair:
            return AZGRA_NAME_OF(RePair);
        case CompressionMethod::RePairImproved:
            return AZGRA_NAME_OF(RePairImproved);
        case CompressionMethod::None:
            break;
    }
    return nullptr;
}

azgra::ByteArray gzip_encode(const azgra::ByteArray &data, azgra::i32 compressionLevel, CompressionResult &info)
{
    always_assert((compressionLevel >= 0 && compressionLevel <= 9) && "ZLIB's compress2 takes compression level from 1 to 9 included!");

    size_t compressedSize = lib_zlib::compressBound(data.size());
    azgra::ByteArray compressedBuffer;
    // Maybe try reserve or normal array.
    compressedBuffer.resize(compressedSize);


    azgra::Stopwatch s;
    s.start();
    azgra::i32 compressionResult = lib_zlib::compress2(compressedBuffer.data(), &compressedSize,
                                                       data.data(), data.size(),
                                                       compressionLevel);
    s.stop();
    info.compressionTimeMS = s.elapsed_milliseconds();

    switch (compressionResult)
    {
        case Z_MEM_ERROR:
            azgra::print_colorized(azgra::ConsoleColor::ConsoleColor_Red, "ZLIB: Not enough memory.\n");
            break;
        case Z_BUF_ERROR:
            azgra::print_colorized(azgra::ConsoleColor::ConsoleColor_Red, "ZLIB: Output buffer is too small.\n");
            break;
        default:
            break;
    }
    always_assert(compressionResult == Z_OK);

    azgra::ByteArray actualCompressedData(compressedBuffer.begin(), compressedBuffer.begin() + compressedSize);

    always_assert(actualCompressedData.size() == compressedSize);

    return actualCompressedData;
}

azgra::ByteArray lzma_encode(const azgra::ByteArray &data, azgra::i32 compressionLevel, CompressionResult &info)
{
    always_assert((compressionLevel >= 0 && compressionLevel <= 9) &&
                  "LZMA's lzma_easy_buffer_encode takes compression level from 1 to 9 included!");


    size_t maximumSize = lib_lzma::lzma_stream_buffer_bound(data.size());
    azgra::ByteArray buffer;
    buffer.resize(maximumSize);
    size_t finalSize = 0;

    azgra::Stopwatch s;
    s.start();

    auto compressionResult = lib_lzma::lzma_easy_buffer_encode(compressionLevel, lib_lzma::LZMA_CHECK_NONE,
                                                               nullptr, data.data(), data.size(),
                                                               buffer.data(), &finalSize, maximumSize);
    s.stop();
    info.compressionTimeMS = s.elapsed_milliseconds();

    switch (compressionResult)
    {
        case lib_lzma::LZMA_MEM_ERROR:
            azgra::print_colorized(azgra::ConsoleColor::ConsoleColor_Red, "LZMA: Unable to allocate memory.\n");
            break;
        case lib_lzma::LZMA_MEMLIMIT_ERROR:
            azgra::print_colorized(azgra::ConsoleColor::ConsoleColor_Red, "LZMA: Not enough memory.\n");
            break;
        case lib_lzma::LZMA_BUF_ERROR:
            azgra::print_colorized(azgra::ConsoleColor::ConsoleColor_Red, "LZMA: Can't read input buffer.\n");
            break;
        case lib_lzma::LZMA_PROG_ERROR:
            azgra::print_colorized(azgra::ConsoleColor::ConsoleColor_Red, "LZMA: Wrong arguments.\n");
            break;
        default:
            break;
    }
    always_assert(compressionResult == lib_lzma::LZMA_OK);

    azgra::ByteArray result(buffer.begin(), buffer.begin() + finalSize);
    always_assert(result.size() == finalSize);

    return result;
}

azgra::ByteArray bzip2_encode(const azgra::ByteArray &data, azgra::i32 compressionLevel, CompressionResult &info)
{
    always_assert((compressionLevel >= 0 && compressionLevel <= 9) &&
                  "BZ2's BZ2_bzBuffToBuffCompress takes compression level from 1 to 9 included!");

    // Maximum compressed size calculation, taken from documentation:
    //
    // To guarantee that the compressed data will fit in its buffer,
    // allocate an output buffer of size 1% larger than the uncompressed data,
    // plus six hundred extra bytes.

    size_t maximumSize = (size_t) ((float_t) data.size() * 3.0f) + 600;
    azgra::ByteArray buffer;
    buffer.resize(maximumSize);
    size_t compressedSize = maximumSize;

    //TODO: Find way without this copy...
    azgra::ByteArray input(data.begin(), data.end());
    always_assert(data.size() == input.size());

    // Default as stated in documentation.
    const azgra::i32 workFactor = 30;
    azgra::Stopwatch s;
    s.start();
    azgra::i32 compressionResult = lib_bzip2::BZ2_bzBuffToBuffCompress((char *) buffer.data(), (azgra::u32 *) (&compressedSize),
                                                                       (char *) input.data(), (azgra::u32) data.size(),
                                                                       compressionLevel, 0, workFactor);
    s.stop();
    info.compressionTimeMS = s.elapsed_milliseconds();

    switch (compressionResult)
    {
        case BZ_CONFIG_ERROR:
            azgra::print_colorized(azgra::ConsoleColor::ConsoleColor_Red, "BZIP2: Wrong configuration.\n");
            break;
        case BZ_PARAM_ERROR:
            azgra::print_colorized(azgra::ConsoleColor::ConsoleColor_Red, "BZIP2: Wrong parameters.\n");
            break;
        case BZ_MEM_ERROR:
            azgra::print_colorized(azgra::ConsoleColor::ConsoleColor_Red, "BZIP2: Not enough memory.\n");
            break;
        case BZ_OUTBUFF_FULL:
            azgra::print_colorized(azgra::ConsoleColor::ConsoleColor_Red, "BZIP2: Output buffer is too small.\n");
            break;
        default:
            break;
    }
    always_assert(compressionResult == BZ_OK);

    azgra::ByteArray result(buffer.begin(), buffer.begin() + compressedSize);
    always_assert(result.size() == compressedSize);

    return result;
}

CompressionResult test_compression_method(const CompressionMethod method, const char *inputFile, const azgra::i32 compressionLevel)
{
    const auto data = azgra::io::stream::InBinaryFileStream(inputFile).consume_whole_file();

    azgra::Stopwatch stopwatch;
    stopwatch.start();
    CompressionResult result = {};
    {
        azgra::ByteArray compressedData;
        switch (method)
        {
            case CompressionMethod::GZIP:
                compressedData = gzip_encode(data, compressionLevel, result);
                break;
            case CompressionMethod::LZMA:
                compressedData = lzma_encode(data, compressionLevel, result);
                break;
            case CompressionMethod::BZIP2:
                compressedData = bzip2_encode(data, compressionLevel, result);
                break;
            default:
                always_assert(false && "Missing case.");
        }
        result.compressedSize = compressedData.size();
    }

    stopwatch.stop();
    result.compressionTimeMS = stopwatch.elapsed_milliseconds();
    result.originalSize = data.size();
    result.compressionRatio = compression_ratio(result.originalSize, result.compressedSize);

    const auto MB = ((result.originalSize / 1000.0) / 1000.0);
    const auto SEC = (result.compressionTimeMS / 1000.0);
    result.speed = MB / SEC;

    return result;
}