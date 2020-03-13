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
    ByteLzTree tree(std::make_unique<ByteLzNode>(bytes.sub_span(0, 4), bytes.size));
//    auto x = bytes.sub_span(0, 4);
//    LzTree<azgra::byte> tree = ByteLzTree(x, bytes.size);

    for (size_t offset = 4; offset < bytes.size; offset += lookAheadSize)
    {
        auto child = std::make_unique<ByteLzNode>(bytes.sub_span(offset, lookAheadSize), bytes.size - offset);
        tree.add_child(child);
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
        auto dataToAdd = toAddDS.sub_span(i * 4, 4);
        auto newNode = std::make_unique<ByteLzNode>(dataToAdd, bytes.size - (lookAheadSize * (3 - i)));
        tree.add_child(newNode);
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

azgra::ByteArray lzss_encode(const azgra::ByteArray &data, const azgra::byte searchBufferBits)
{
    // NOTE(Moravec):   We are going to cheat and hold the whole data buffer in memory
    //                  instead of reading from the stream.

    using namespace azgra::io::stream;
    always_assert(searchBufferBits < 16 && "No bits left for look ahead buffer");

    const std::size_t lookAheadBufferBits = 16 - searchBufferBits;

    const auto searchBufferSize = static_cast<std::size_t > (pow(2, searchBufferBits));
    const auto lookAheadBufferSize = static_cast<std::size_t > (pow(2, lookAheadBufferBits));
    const std::size_t slidingWindowSize = searchBufferSize + lookAheadBufferSize;

    std::size_t bufferSize = data.size();
    std::size_t treeNodeCount = 1 + searchBufferSize - lookAheadBufferSize;

    ByteLzTree searchTree;

    std::size_t lastShift = 0;
    SlidingWindow<azgra::byte> window(data.data(), -searchBufferSize, slidingWindowSize, searchBufferSize);
    std::size_t bufferIndex = searchBufferSize;


    ByteSpan searchSpan;
    LzMatch searchMatch;
    while (bufferIndex < bufferSize)
    {
        searchSpan = window.search_span();
        searchMatch = searchTree.find_best_match(searchSpan);

        if (searchMatch.length > 0)
        {

        }

        // TODO: REPLACE
        ++bufferIndex;
    }


}

void test_lzss()
{
    const char *stringData = "abbabbbabaa";
    //const char* stringData = "abaacbacbcca";
    const std::size_t stringLen = strlen(stringData);
    azgra::ByteArray inputData(stringLen);
    std::memcpy(inputData.data(), stringData, stringLen);

    const auto lzssEncodedData = lzss_encode(inputData, 11);

    puts("are we done?");

}
