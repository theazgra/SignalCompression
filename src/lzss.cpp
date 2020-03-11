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
    const byte_span bytes(bytesVector.data(), bytesVector.size());

    ByteLzNode root(bytes.sub_span(0, 4), bytes.size);
    ByteLzTree tree(root);
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
    const byte_span toDeleteDS(toDelete.data(), toDelete.size());

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


    const byte_span toAddDS(toAdd.data(), toAdd.size());
    for (int i = 0; i < 3; ++i)
    {
        auto dataToAdd = toAddDS.sub_span(i * 4, 4);
        auto newNode = std::make_unique<ByteLzNode>(dataToAdd, bytes.size - (lookAheadSize * (3 - i)));
        tree.add_child(newNode);
    }
    puts("Added 3 nodes");
}
