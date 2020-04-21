#pragma once
#include "benchmark_record.h"
#include <azgra/utilities/stopwatch.h>
#include <azgra/io/stream/in_binary_file_stream.h>

namespace lib_zlib
{
#include <zlib.h>
}

namespace lib_lzma
{
#include <lzma.h>
}

namespace lib_bzip2
{
#include <bzlib.h>
}

enum class CompressionMethod
{
	GZIP,
	LZMA,
	BZIP2,
	RePair,
    RePairImproved,
    None,
};

const char* compression_method_name(CompressionMethod method);

inline double compression_ratio(std::size_t originalSize, std::size_t compressedSize)
{
    return (static_cast<double>(compressedSize) / static_cast<double >(originalSize));
}

azgra::ByteArray gzip_encode(const azgra::ByteArray& data, azgra::i32 compressionLevel, CompressionResult& info);
azgra::ByteArray lzma_encode(const azgra::ByteArray& data, azgra::i32 compressionLevel, CompressionResult& info);
azgra::ByteArray bzip2_encode(const azgra::ByteArray& data, azgra::i32 compressionLevel, CompressionResult& info);
CompressionResult test_compression_method(CompressionMethod method, const char *inputFile, azgra::i32 compressionLevel);
CompressionResult test_compression_method(CompressionMethod method, const azgra::ByteArray &data, azgra::i32 compressionLevel);
