#pragma once

#include "lz_tree.h"
#include "ring_buffer.h"
#include "sliding_window.h"
#include <random>
#include <azgra/io/stream/memory_bit_stream.h>
#include <azgra/io/stream/in_binary_file_stream.h>
#include <sstream>
#include <azgra/fs/file_system.h>

constexpr azgra::byte BYTE_SIZE = sizeof(azgra::byte);

constexpr bool RAW_BYTE_FLAG = false;
constexpr bool PAIR_FLAG = true;

constexpr bool IS_RAW_BYTE_FLAG(const bool flag){ return !flag; }
constexpr bool IS_PAIR_FLAG(const bool flag) { return flag; }

struct LzssHeader
{
    std::size_t fileSize{0};
    azgra::byte SBits{0};
    azgra::byte LBits{0};

    LzssHeader() = default;

    explicit LzssHeader(const std::size_t fileSize_, const azgra::byte SBits_, const azgra::byte LBits_)
            : fileSize(fileSize_), SBits(SBits_), LBits(LBits_)
    {

    }

    void write_to_encoder_stream(azgra::io::stream::OutMemoryBitStream &encoderStream)
    {
        encoderStream.write_value(fileSize);
        encoderStream.write_value(SBits);
        encoderStream.write_value(LBits);
    }

    void read_from_decoder_stream(azgra::io::stream::InMemoryBitStream &decoderStream)
    {
        fileSize = decoderStream.read_value<std::size_t>();
        SBits = decoderStream.read_value<azgra::byte>();
        LBits = decoderStream.read_value<azgra::byte>();
    }
};

struct LzssResult
{
    LzssResult() = default;
    LzssResult(const LzssResult &) = default;

    std::size_t originalSize;
    azgra::ByteArray encodedBytes;
    std::size_t encodedBytesCount;
    std::size_t maxMatchSize;
    std::size_t pairCount;
    std::size_t rawBytesCount;
    std::size_t S;
    std::size_t L;
    azgra::byte SBits;
    azgra::byte LBits;
    double bps;
};


[[nodiscard]] LzssResult lzss_encode(const azgra::ByteArray &data,
                                     const std::size_t searchBufferSize,
                                     const std::size_t lookAheadBufferSize);

azgra::ByteArray lzss_decode(const azgra::ByteArray &encodedBytes);

void test_lzss(const char *inputFile);

