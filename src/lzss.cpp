#include "lzss.h"

void test()
{
    std::vector<azgra::byte> bytesVector = {'m', 'i', 'n', 'e',
                                            'f', 'r', 'o', 'm',
                                            'c', 'o', 'm', 'e',
                                            'l', 'e', 'f', 't',
                                            'b', 'r', 'i', 'm',
                                            'f', 'a', 's', 't',
                                            'h', 'e', 'r', 's',
                                            's', 'l', 'o', 'w',
                                            'p', 'l', 'u', 'g',
                                            's', 't', 'e', 'p',
                                            'o', 'b', 'e', 'y',
                                            's', 'l', 'a', 'm',
                                            's', 't', 'a', 'y',
                                            'w', 'e', 'n', 't'};

    const size_t lookAheadSize = 4;
    const ByteSpan bytes(bytesVector.data(), bytesVector.size());

    //ByteLzNode root(bytes.sub_span(0, 4), bytes.size);
    ByteLzTree tree(std::make_unique<ByteLzNode>(bytes.sub_span(0, 4)));
//    auto x = bytes.sub_span(0, 4);
//    LzTree<azgra::byte> tree = ByteLzTree(x, bytes.size);

    for (size_t offset = 4; offset < bytes.size; offset += lookAheadSize)
    {
        tree.add_node(std::make_unique<ByteLzNode>(bytes.sub_span(offset, lookAheadSize)));
    }

    std::vector<azgra::byte> toDelete = {'m', 'i', 'n', 'e',
                                         'f', 'r', 'o', 'm',
                                         'c', 'o', 'm', 'e'};
    const ByteSpan toDeleteDS(toDelete.data(), toDelete.size());

    // TODO( Update distances );
    for (int i = 0; i < 3; ++i)
    {
        auto dataToDelete = toDeleteDS.sub_span(i * 4, 4);
        always_assert(tree.delete_node(dataToDelete));
    }

    puts("Deleted 3 nodes.");

    std::vector<azgra::byte> toAdd = {'f', 'i', 'r', 'e',
                                      'm', 'o', 'n', 'k',
                                      'q', 'w', 'r', 'y'};


    const ByteSpan toAddDS(toAdd.data(), toAdd.size());
    for (int i = 0; i < 3; ++i)
    {
        tree.add_node(std::make_unique<ByteLzNode>(toAddDS.sub_span(i * 4, 4)));
    }
    puts("Added 3 nodes");
}

void test2()
{
    const size_t n = 10000000;
    std::vector<int> x(n);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(0, 9999999);
    for (int j = 0; j < n; ++j)
    {
        x[j] = dist(gen);
    }
    puts("generated random numbers");

    RingBuffer<int> ring(512);

    for (int i = 0; i < n; ++i)
    {
        ring.push(x[i]);
        if (i % 500 == 0)
        {
            fprintf(stdout, "%i\n", ring.head());
        }
    }
    puts("finished");
}

inline void push_to_ring_buffer(RingBuffer<azgra::byte> &ringBuffer,
                                const azgra::ByteArray &inputBuffer,
                                const std::size_t position,
                                const std::size_t length)
{
    for (std::size_t i = 0; i < length; ++i)
    {
        ringBuffer.push(inputBuffer[position + i]);
    }
}

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

    std::size_t inputBufferSize = data.size();
    std::size_t treeNodeCount = 1 + searchBufferSize - lookAheadBufferSize;

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
    LzMatch searchMatch;
    std::size_t maxSizeMatch = 0;
    std::size_t tripletCount = 0;
    while (bufferIndex < inputBufferSize)
    {
        if (bufferIndex < (2 * lookAheadBufferSize))
        {
            // Before we get at least L elements in search buffer encode values with char code (0,'A')
            azgra::byte byteToEncode = window[0];
            flagBuffer |= RAW_BYTE_FLAG << flagBufferIndex++;
            interBuffer.write_aligned_byte(byteToEncode);
            windowShift = 1;
        }
        else
        {
            // Create nodes in the tree. Node count to add and remove is equivalent to lastShift value.
            {
                const auto windowBeginIndex = window.begin_index();
                for (std::size_t nodeIndex = 0; nodeIndex < windowShift; ++nodeIndex)
                {
                    // Create span for node. Offset from the delimiter backwards.
                    searchTree.add_node(
                            std::make_unique<ByteLzNode>(window.span_from_delimiter(-static_cast<long>(lookAheadBufferSize + nodeIndex))));

                    if (windowBeginIndex > 0)
                    {
                        // If the sliding window is full removed oldest entries.
                        const long offset = static_cast<long>(windowShift) - (static_cast<long> (nodeIndex + 1));
                        const auto dataToDelete = window.span_from_begin(-offset);

//                        assert(searchTree.has_node_with_data(dataToDelete) && "Failed to find node supposted to be in tree");

                        const bool deleted = searchTree.delete_node(dataToDelete);
                        assert(deleted);
                    }
                }
            }

            searchSpan = window.span_from_delimiter(0);
            searchMatch = searchTree.find_best_match(searchSpan);

            if (searchMatch.length > 1)
            {
                ++tripletCount;
                maxSizeMatch = std::max(maxSizeMatch, searchMatch.length);
//                azgra::print_if((searchMatch.length > 2), "Found match of size: %lu\n", searchMatch.length);
                // Write pair (distance,length)
                flagBuffer |= PAIR_FLAG << flagBufferIndex++;
                interBuffer.write_value(searchMatch.distance, searchBufferBits);
                interBuffer.write_value(searchMatch.length, lookAheadBufferBits);
                windowShift = searchMatch.length;
            }
            else
            {
                // Write RAW byte
                flagBuffer |= RAW_BYTE_FLAG << flagBufferIndex++;
                interBuffer.write_aligned_byte(searchSpan.ptr[0]);
                windowShift = 1;
            }
        }

        bufferIndex += windowShift;
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
    puts("Finished some compression...");


}

void test_lzss(const char *inputFile)
{
    using namespace azgra::io::stream;

    azgra::ByteArray inputData = InBinaryFileStream(inputFile).consume_whole_file();

    const auto lzssEncodedData = lzss_encode(inputData, 32, 27);

    puts("are we done?");
}
