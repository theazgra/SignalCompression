#include "lzss.h"


azgra::ByteArray lzss_encode(const azgra::ByteArray &data,
                             const azgra::byte windowBits,
                             const azgra::byte searchBufferBits)
{
    // NOTE(Moravec):   We are going to cheat and hold the whole data buffer in memory
    //                  instead of reading from the stream.

    using namespace azgra::io::stream;
    // Size of the group. 8 Flags for 8 values or pairs.
    const std::size_t GROUP_SIZE = 8;

    always_assert(searchBufferBits < windowBits && "No bits left for look ahead buffer");

    const std::size_t lookAheadBufferBits = (windowBits - searchBufferBits);

    const auto searchBufferSize = static_cast<std::size_t > (pow(2, searchBufferBits));
    const auto lookAheadBufferSize = static_cast<std::size_t > (pow(2, lookAheadBufferBits));
    const std::size_t slidingWindowSize = searchBufferSize + lookAheadBufferSize;

    fprintf(stdout, "S=%lu\nL=%lu\nW=%lu\n", searchBufferSize, lookAheadBufferSize, slidingWindowSize);

    std::size_t inputBufferSize = data.size();

    // Binary search tree.
    ByteLzTree searchTree;

    // Last shift of the window.
    std::size_t windowShift = 0;
    // Sliding window.
    SlidingWindow<azgra::byte> window(data.data(), -searchBufferSize, slidingWindowSize, searchBufferSize);
    // Current input buffer index
    std::size_t bufferIndex = lookAheadBufferSize;

    // Encoder stream.
    OutMemoryBitStream encoderStream;
    // InterBuffer stream.
    OutMemoryBitStream interBuffer;

    // Flag buffer with its index.
    azgra::byte flagBuffer = 0;
    azgra::byte flagBufferIndex = 0;

    // String being searched.
    ByteSpan searchSpan;
    // Match in the binary tree.
    LzMatch searchResult;

    std::size_t bestMatch = 0;
    std::size_t rawCount = 0;
    std::size_t pairMatchCount = 0;
    double matchSizes = 0.0;
    long long remaining = inputBufferSize;

    while (remaining > 0)
    {
        if (bufferIndex < (2 * lookAheadBufferSize))
        {
            // Before we get at least L elements in search buffer encode values with char code (0,'A')
            azgra::byte byteToEncode = window[0];
            flagBuffer |= RAW_BYTE_FLAG << flagBufferIndex++;
            interBuffer.write_aligned_byte(byteToEncode);
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
                    searchTree.add_node(window.span_from_delimiter(-static_cast<long>(lookAheadBufferSize + nodeIndex)));
                }

//                if (windowBeginIndex > 0)
//                {
                const long int deleteCount = std::min(static_cast<long>(windowShift), windowBeginIndex);
                for (int nodeIndex = 0; nodeIndex < deleteCount; ++nodeIndex)
                {
                    // If the sliding window is full removed oldest entries.
                    const long offset = static_cast<long>(deleteCount + 1) - (static_cast<long> (nodeIndex + 1));
                    const auto dataToDelete = window.span_from_begin(-offset);

                    const auto deletionResult = searchTree.delete_node(dataToDelete);
                    assert(deletionResult == NodeDeletionResult::NodeDeleted ||
                           deletionResult == NodeDeletionResult::NodeSurvived);
                }
//                }
            }

            searchSpan = window.span_from_delimiter(0, std::min(lookAheadBufferSize, static_cast<std::size_t>(remaining)));
            searchResult = searchTree.find_best_match(searchSpan);

            if (searchResult.length > 1)
            {
                matchSizes += searchResult.length;
                ++pairMatchCount;
                bestMatch = std::max(bestMatch, searchResult.length);

                // Write pair (distance,length)
                flagBuffer |= PAIR_FLAG << flagBufferIndex++;
                interBuffer.write_value(searchResult.distance, searchBufferBits);
                interBuffer.write_value(searchResult.length, lookAheadBufferBits);
                windowShift = searchResult.length;

                //fprintf(stdout, "Match: Distance: %lu\tLength: %lu\n", searchMatch.distance, searchMatch.length);
            }
            else
            {
                // Write RAW byte
                flagBuffer |= RAW_BYTE_FLAG << flagBufferIndex++;
                interBuffer.write_aligned_byte(searchSpan.ptr[0]);
                windowShift = 1;
                ++rawCount;
            }
        }

        bufferIndex += windowShift;
        remaining -= windowShift;
        window.slide(windowShift);

        assert(flagBufferIndex <= GROUP_SIZE);
        if (flagBufferIndex == GROUP_SIZE)
        {
            // Flush flag buffer and inter buffer to encoder stream.
            encoderStream.write_aligned_byte(flagBuffer);
            interBuffer.copy_aligned_buffer_and_reset(encoderStream);

            flagBufferIndex = 0;
            flagBuffer = 0;
        }
        //fprintf(stdout, "%lli/%lu\n", remaining, inputBufferSize);
    }

    if (flagBufferIndex > 0)
    {
        // Flush flag buffer and inter buffer to encoder stream.
        encoderStream.write_aligned_byte(flagBuffer);
        interBuffer.copy_aligned_buffer_and_reset(encoderStream);
    }

    //maxSizeMatch
    const auto encodedSize = encoderStream.get_flushed_buffer().size();
    const double bps = static_cast<double>(encodedSize * 16) / static_cast<double> (inputBufferSize);
    fprintf(stdout, "ESize: %lu\nBPS: %.4f\nRawCount: %lu\nPairCount: %lu\n MaxMatch: %lu\n", encodedSize, bps, rawCount, pairMatchCount,
            bestMatch);

    const double averageMatchSize = matchSizes / static_cast<double>(pairMatchCount);
    fprintf(stdout, "Average match size: %.4f\n", averageMatchSize);

    return azgra::ByteArray(10);
    //return encoderStream.get_flushed_buffer();
}


void test_lzss(const char *inputFile)
{
    using namespace azgra::io::stream;

    azgra::ByteArray inputData = InBinaryFileStream(inputFile).consume_whole_file();

//    [[maybe_unused]] const azgra::ByteArray lzssEncodedData = lzss_encode(inputData, 32, 25);
//    [[maybe_unused]] const azgra::ByteArray lzssEncodedData2 = lzss_encode(inputData, 16, 10);
    [[maybe_unused]] const azgra::ByteArray lzssEncodedData3 = lzss_encode(inputData, 16, 11);
}
