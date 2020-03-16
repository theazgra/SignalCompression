#include <azgra/fs/file_info.h>
#include "lzss.h"


LzssResult lzss_encode(const azgra::ByteArray &data,
                       const std::size_t searchBufferSize,
                       const std::size_t lookAheadBufferSize)
{
    // NOTE(Moravec):   We are going to cheat and hold the whole data buffer in memory
    //                  instead of reading from the stream.

    // TODO(Moravec):   We are not grouping bit flags in this first implementation.

    using namespace azgra::io::stream;

    const std::size_t inputBufferSize = data.size();

    const auto SBits = static_cast<azgra::byte>(ceil(log2(searchBufferSize)));
    const auto LBits = static_cast<azgra::byte>(ceil(log2(lookAheadBufferSize)));
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
            encoderStream.write_bit(RAW_BYTE_FLAG);
            encoderStream.write_value(window[0]);
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
                matchSizes += searchResult.length;
                ++pairCount;
                longestMatch = std::max(longestMatch, searchResult.length);

                // Write pair (distance,length)
                encoderStream.write_bit(PAIR_FLAG);
                encoderStream.write_value(searchResult.distance, SBits);
                encoderStream.write_value(searchResult.length, LBits);
                windowShift = searchResult.length;
                //fprintf(stdout, "Match: Distance: %lu\tLength: %lu\n", searchMatch.distance, searchMatch.length);
            }
            else
            {
                // Write RAW byte
                encoderStream.write_bit(RAW_BYTE_FLAG);
                encoderStream.write_value(window[0]);
                windowShift = 1;
                ++rawCount;
            }
        }

        bufferIndex += windowShift;
        remaining -= windowShift;
        window.slide(windowShift);
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
    result.bps = static_cast<double>(result.encodedBytesCount * 16) / static_cast<double> (inputBufferSize);

    return result;
}

static void report_lzss_result(const char *inputFile, const LzssResult &result)
{
    std::stringstream ss;
    const auto fileName = azgra::fs::FileInfo(azgra::StringView(inputFile)).get_filename();
    ss << "File\t\tPairCount\tRawCount\tSize\t\tEnc.Size\tS\tL\tBPS\n";
    ss << fileName << '\t' << result.pairCount << "\t\t" << result.rawBytesCount << "\t\t"
       << result.originalSize << "\t\t" << result.encodedBytesCount << "\t\t" << result.S << '\t'
       << result.L << '\t' << result.bps << '\n';
    puts(ss.str().c_str());
}


void test_lzss(const char *inputFile)
{
    using namespace azgra::io::stream;

    azgra::ByteArray inputData = InBinaryFileStream(inputFile).consume_whole_file();

    //    File           Triplets    FileSize  WindowSize   Max.match    Enc.Size         bps
//    -----------------------------------------------------------------------------------
//            czech.txt         28735      150849        4096          16      689640       4.57

    [[maybe_unused]] const LzssResult lzssEncodedData1 = lzss_encode(inputData, 4096, 16);
    report_lzss_result(inputFile, lzssEncodedData1);

    [[maybe_unused]] const LzssResult lzssEncodedData2 = lzss_encode(inputData, 16384, 32);
    report_lzss_result(inputFile, lzssEncodedData2);

    [[maybe_unused]] const LzssResult lzssEncodedData3 = lzss_encode(inputData, 32768, 64);
    report_lzss_result(inputFile, lzssEncodedData3);
}
