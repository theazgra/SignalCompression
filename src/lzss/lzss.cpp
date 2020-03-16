#include <azgra/fs/file_info.h>
#include "lzss.h"

void write_tokens_to_stream(azgra::io::stream::OutMemoryBitStream &encoderStream,
                            const std::array<LzssToken, FLAG_GROUP_SIZE> &tokens,
                            const azgra::byte SBits,
                            const azgra::byte LBits,
                            const azgra::byte flagIndex)
{
    for (int fId = 0; fId < flagIndex; ++fId)
    {
        assert(tokens[fId].is_valid());
        if (tokens[fId].is_pair())
        {
            encoderStream.write_value(tokens[fId].get_match().distance, SBits);
            encoderStream.write_value(tokens[fId].get_match().length, LBits);
        }
        else
        {
            encoderStream.write_value(tokens[fId].get_byte());
        }
    }
}

LzssResult lzss_encode(const azgra::ByteArray &data,
                       const std::size_t searchBufferSize,
                       const std::size_t lookAheadBufferSize)
{
    // NOTE(Moravec):   We are going to cheat and hold the whole data buffer in memory
    //                  instead of reading from the stream.

    // TODO(Moravec):   We are not grouping bit flags in this first implementation.

    using namespace azgra::io::stream;

    const std::size_t inputBufferSize = data.size();


    const auto SBits = static_cast<azgra::byte>(azgra::io::stream::bits_required(searchBufferSize));
    const auto LBits = static_cast<azgra::byte>(azgra::io::stream::bits_required(lookAheadBufferSize));
    const auto slidingWindowSize = searchBufferSize + lookAheadBufferSize;

    //fprintf(stdout, "S=%lu(%ub)\tL=%lu(%ub)\tW=%lu\n", searchBufferSize, SBits, lookAheadBufferSize, LBits, slidingWindowSize);

    // Binary search tree.
    ByteLzTree bst;

    // Last shift of the window.
    std::size_t windowShift = 0;
    // Sliding window.
    SlidingWindow<azgra::byte> window(data.data(), -searchBufferSize, slidingWindowSize, searchBufferSize);
    // Current input buffer index
    std::size_t bufferIndex = lookAheadBufferSize;

    // Encoder stream.
    OutMemoryBitStream encoderStream;

    LzssHeader header(inputBufferSize, SBits, LBits);
    header.write_to_encoder_stream(encoderStream);

    // String being searched.
    ByteSpan searchSpan;
    // Match in the binary tree.
    LzMatch searchResult;

    azgra::byte flagBuffer = 0;
    azgra::byte flagIndex = 0;
    std::array<LzssToken, FLAG_GROUP_SIZE> interBuffer;

    std::size_t longestMatch = 0;
    std::size_t rawCount = 0;
    std::size_t pairCount = 0;
    double matchSizes = 0.0;
    long long remaining = inputBufferSize;
    while (remaining > 0)
    {
        if (bufferIndex < (2 * lookAheadBufferSize))
        {
            // Before we get at least L elements in search buffer encode values with char code (0,'A')
            flagBuffer |= static_cast<uint8_t>(RAW_BYTE_FLAG) << flagIndex;
            interBuffer[flagIndex] = LzssToken::RawByteToken(window[0]);
            ++flagIndex;
//            encoderStream.write_bit(RAW_BYTE_FLAG);
//            encoderStream.write_value(window[0]);
            windowShift = 1;
            ++rawCount;
        }
        else
        {
            // Create nodes in the tree. Node count to add and remove is equivalent to lastShift value.
            {
                const auto windowBeginIndex = window.begin_index();
                for (std::size_t nodeIndex = 0; nodeIndex < windowShift; ++nodeIndex)
                {
                    // Create Span for node. Offset from the delimiter backwards.
                    bst.add_node(window.span_from_delimiter(-static_cast<long>(lookAheadBufferSize + nodeIndex)));
                }

                const long int deleteCount = std::min(static_cast<long>(windowShift), windowBeginIndex);
                for (int nodeIndex = 0; nodeIndex < deleteCount; ++nodeIndex)
                {
                    // If the sliding window is full removed oldest entries.
                    const long offset = static_cast<long>(deleteCount + 1) - (static_cast<long> (nodeIndex + 1));
                    const auto dataToDelete = window.span_from_begin(-offset);

                    [[maybe_unused]] const auto deletionResult = bst.delete_node(dataToDelete);
                    assert(deletionResult == NodeDeletionResult::NodeDeleted ||
                           deletionResult == NodeDeletionResult::NodeSurvived);
                }
            }

            searchSpan = window.span_from_delimiter(0, std::min(lookAheadBufferSize, static_cast<std::size_t>(remaining)));
            searchResult = bst.find_best_match(searchSpan);

            if (searchResult.length > 1)
            {
//                // Write pair (distance,length)
//                encoderStream.write_bit(PAIR_FLAG);
//                encoderStream.write_value(searchResult.distance, SBits);
//                encoderStream.write_value(searchResult.length, LBits);

                flagBuffer |= static_cast<uint8_t>(PAIR_FLAG) << flagIndex;
                interBuffer[flagIndex] = LzssToken::PairToken(searchResult);
                ++flagIndex;

                windowShift = searchResult.length;

                ++pairCount;
                longestMatch = std::max(longestMatch, searchResult.length);
                matchSizes += searchResult.length;
            }
            else
            {
//                // Write RAW byte
//                encoderStream.write_bit(RAW_BYTE_FLAG);
//                encoderStream.write_value(window[0]);
//                windowShift = 1;
//                ++rawCount;

                flagBuffer |= static_cast<uint8_t>(RAW_BYTE_FLAG) << flagIndex;
                interBuffer[flagIndex] = LzssToken::RawByteToken(window[0]);
                ++flagIndex;
                windowShift = 1;
                ++rawCount;
            }
        }

        if (flagIndex >= FLAG_GROUP_SIZE)
        {
            encoderStream.write_value(flagBuffer);
            write_tokens_to_stream(encoderStream, interBuffer, SBits, LBits, flagIndex);
            flagBuffer = 0;
            flagIndex = 0;
        }

        bufferIndex += windowShift;
        remaining -= windowShift;
        window.slide(windowShift);
    }

    if (flagIndex > 0)
    {
        encoderStream.write_value(flagBuffer);
        write_tokens_to_stream(encoderStream, interBuffer, SBits, LBits, flagIndex);
    }

    LzssResult result = {};
    result.originalSize = inputBufferSize;
    result.encodedBytes = encoderStream.get_flushed_buffer();
    result.encodedBytesCount = result.encodedBytes.size();
    result.maxMatchSize = longestMatch;
    result.pairCount = pairCount;
    result.rawBytesCount = rawCount;
    result.S = searchBufferSize;
    result.L = lookAheadBufferSize;
    result.SBits = SBits;
    result.LBits = LBits;
    result.bps = static_cast<double>(result.encodedBytesCount * 8) / static_cast<double> (inputBufferSize);

    return result;
}

azgra::ByteArray lzss_decode(const azgra::ByteArray &encodedBytes)
{
    azgra::io::stream::InMemoryBitStream decoderStream(&encodedBytes);
    LzssHeader header;
    header.read_from_decoder_stream(decoderStream);

    std::size_t index = 0;
    azgra::ByteArray decodedBytes(header.fileSize);

    std::size_t distance;
    std::size_t length;
    std::size_t offset;
    azgra::byte flagBuffer;
    std::array<bool, FLAG_GROUP_SIZE> flags{};
    while (index < header.fileSize)
    {
        flagBuffer = decoderStream.read_value<azgra::byte>();
        for (long fId = (FLAG_GROUP_SIZE - 1); fId >= 0; --fId)
        {
            flags[fId] = (flagBuffer & (1u << static_cast<uint8_t> (fId)));
        }

        for (std::size_t fId = 0; fId < FLAG_GROUP_SIZE; ++fId)
        {
            if (IS_RAW_BYTE_FLAG(flags[fId]))
            {
                if (index >= header.fileSize)
                    break;
                decodedBytes[index++] = decoderStream.read_value<azgra::byte>();
            }
            else
            {
                if (index >= header.fileSize)
                    break;
                assert(IS_PAIR_FLAG(flags[fId]));
                distance = decoderStream.read_value<std::size_t>(header.SBits);
                length = decoderStream.read_value<std::size_t>(header.LBits);
                assert(length > 0);

                assert(index >= distance);
                offset = index - distance;
                assert(length <= index - offset);

                for (std::size_t i = 0; i < length; ++i)
                {
                    decodedBytes[index++] = decodedBytes[offset + i];
                }
            }
        }
    }

    return decodedBytes;
}

static void report_lzss_result(const char *inputFile, const LzssResult &result, const bool equal)
{
    std::stringstream ss;
    const auto fileName = azgra::fs::FileInfo(azgra::StringView(inputFile)).get_filename();
    ss << "File\t\tPairCount\tRawCount\tSize\t\tEnc.Size(b)\tS\tL\tBPS\tMaxMatch\n";
    ss << fileName << '\t' << result.pairCount << "\t\t" << result.rawBytesCount << "\t\t"
       << result.originalSize << "\t\t" << (result.encodedBytesCount * 8) << "\t\t" << result.S << '\t'
       << result.L << '\t' << result.bps << '\t' << result.maxMatchSize;


    azgra::print_colorized(equal ? azgra::ConsoleColor::ConsoleColor_Green : azgra::ConsoleColor::ConsoleColor_Red,
                           "%s\n",
                           ss.str().c_str());
}


void test_lzss(const char *inputFile)
{
    using namespace azgra::io::stream;

    azgra::ByteArray inputData = InBinaryFileStream(inputFile).consume_whole_file();

    //    File           Triplets    FileSize  WindowSize   Max.match    Enc.Size         bps
//    -----------------------------------------------------------------------------------
//            czech.txt         28735      150849        4096          16      689640       4.57

    const LzssResult lzssEncodedData1 = lzss_encode(inputData, 4096, 16);
    const auto decodedBytes = lzss_decode(lzssEncodedData1.encodedBytes);
    const bool eq1 = std::equal(inputData.begin(), inputData.end(), decodedBytes.begin(), decodedBytes.end());
    report_lzss_result(inputFile, lzssEncodedData1, eq1);

    const LzssResult lzssEncodedData2 = lzss_encode(inputData, 16384, 32);
    const bool eq2 = std::equal(inputData.begin(), inputData.end(), decodedBytes.begin(), decodedBytes.end());
    report_lzss_result(inputFile, lzssEncodedData2, eq2);

    const LzssResult lzssEncodedData3 = lzss_encode(inputData, 32768, 64);
    const bool eq3 = std::equal(inputData.begin(), inputData.end(), decodedBytes.begin(), decodedBytes.end());
    report_lzss_result(inputFile, lzssEncodedData3, eq3);

    puts("-------------------------------");
}


