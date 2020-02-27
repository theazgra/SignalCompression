#include <queue>
#include "huffman.h"

SymbolInfo &SymbolInfo::operator++()
{
    ++occurrenceCount;
    return *this;
}

bool HuffmanNode::operator<(const HuffmanNode &other) const
{ return (probability < other.probability); }

bool HuffmanNode::operator==(const HuffmanNode &other) const
{ return symbol == other.symbol; }

bool FanoNode::operator<(const FanoNode &other) const
{ return (probability < other.probability); }

bool FanoNode::operator==(const FanoNode &other) const
{ return (symbol == other.symbol); }

std::shared_ptr<HuffmanNode> HuffmanNode::navigate(const bool bit)
{
    if (parentA && parentA->bit == bit)
    {
        return parentA;
    }
    if (parentB && parentB->bit == bit)
    {
        return parentB;
    }
    always_assert(false && "Wrong huffman tree. Failed to navigate.");
}

static void find_all_symbols(std::map<char, SymbolInfo> &symbolMap, const azgra::StringView &string)
{
    for (char c : string)
    {
        if (auto it = symbolMap.find(c); it != symbolMap.cend())
        {
            ++symbolMap[c];
            // Assert that we didn't overflow.
            assert(symbolMap[c].occurrenceCount != 0);
        }
        else
        {
            symbolMap[c] = SymbolInfo(1);
        }
    }
}

inline void calculate_symbol_probability(std::map<char, SymbolInfo> &symbolMap, const size_t totalSymbolCount)
{
    for (auto &[symbol, info] : symbolMap)
    {
        info.probability = static_cast<float>(info.occurrenceCount) / static_cast<float>(totalSymbolCount);
    }
}

static float calculate_entropy(const std::map<char, SymbolInfo> &symbolMap)
{
    float entropy = 0.0f;
    for (auto &[symbol, info] : symbolMap)
    {
        entropy += (info.probability * log2(info.probability));
    }
    return -entropy;
}

std::map<char, SymbolInfo> get_string_symbols_info(const azgra::StringView &string)
{
    std::map<char, SymbolInfo> symbolMap;

    find_all_symbols(symbolMap, string);
    calculate_symbol_probability(symbolMap, string.size());

    float entropy = calculate_entropy(symbolMap);
    fprintf(stdout, "Entropy: %.4f\n", entropy);

    return symbolMap;
}

static void create_symbol_code(const std::shared_ptr<HuffmanNode> &currentNode,
                               std::vector<bool> code,
                               std::map<char, std::vector<bool>> &symbolCodes)
{
    bool leaf = true;

    if (currentNode->bit != -1)
        code.emplace_back(currentNode->bit == 1);

    if (currentNode->parentA)
    {
        create_symbol_code(currentNode->parentA, code, symbolCodes);
        leaf = false;
    }
    if (currentNode->parentB)
    {
        create_symbol_code(currentNode->parentB, code, symbolCodes);
        leaf = false;
    }
    if (leaf)
    {
        currentNode->isLeaf = true;
        symbolCodes[currentNode->symbol] = code;
    }
}

static void split_fano_tree(std::vector<std::shared_ptr<FanoNode>> &nodes,
                            const size_t fromIndex,
                            const size_t toIndex)
{
    using namespace azgra::collection;
    size_t bestSplitIndex = 0;
    if ((toIndex - fromIndex) > 2)
    {

        float minDiff = std::numeric_limits<float>::max();
        const auto selectNodeProb = [](std::shared_ptr<FanoNode> &node)
        { return node->probability; };
        for (size_t splitIndex = (fromIndex + 1); splitIndex < toIndex; ++splitIndex)
        {
            const float leftGroupCumulativeProbability = sum(nodes.begin() + fromIndex, (nodes.begin() + splitIndex), selectNodeProb, 0.0f);
            const float rightGroupCumulativeProbability = sum((nodes.begin() + splitIndex), nodes.begin() + toIndex, selectNodeProb, 0.0f);

            const float diff = abs(leftGroupCumulativeProbability - rightGroupCumulativeProbability);
            if (diff < minDiff)
            {
                minDiff = diff;
                bestSplitIndex = splitIndex;
            }
        }

    }
    else
    {
        bestSplitIndex = fromIndex + 1;
    }

    for (size_t i = fromIndex; i < toIndex; ++i)
    {
        nodes[i]->bits.push_back((i < bestSplitIndex));
    }

    const size_t leftGroupSize = (bestSplitIndex - fromIndex);
    const size_t rightGroupSize = (toIndex - bestSplitIndex);

    if (leftGroupSize > 1)
    {
        split_fano_tree(nodes, fromIndex, bestSplitIndex);
    }
    if (rightGroupSize > 1)
    {
        split_fano_tree(nodes, bestSplitIndex, toIndex);
    }
}

Huffman build_fano_tree(std::map<char, SymbolInfo> &symbolMap)
{
    std::vector<std::shared_ptr<FanoNode>> nodes(symbolMap.size());
    size_t index = 0;
    // Initialize nodes from symbol map
    for (const auto &[symbol, info] : symbolMap)
    {
        nodes[index++] = std::make_shared<FanoNode>(symbol, info.probability);
    }

    std::sort(nodes.begin(), nodes.end());

    split_fano_tree(nodes, 0, nodes.size());

    for (const auto &node : nodes)
    {
        std::cout << "Node: " << node->symbol << '\t';
        for (const bool bit : node->bits)
            std::cout << (bit ? 1 : 0);
        std::cout << '\n';
    }

    puts("VOILA");
}

Huffman build_huffman_tree(std::map<char, SymbolInfo> &symbolMap)
{
    // NOTE(Moravec):   We are not able to use std::unique_ptr with std::priority_queue
    //                  because top returns const&, which can't be moved.

    std::priority_queue<std::shared_ptr<HuffmanNode>,
            std::vector<std::shared_ptr<HuffmanNode>>,
            HuffmanNodeComparerGreater> nodes;

    // Initialize nodes from symbol map
    for (const auto &[symbol, info] : symbolMap)
    {
        nodes.push(std::make_shared<HuffmanNode>(symbol, info.probability));
    }

    while (nodes.size() != 1)
    {
        // the lowest probability
        auto parentA = nodes.top();
        nodes.pop();
        auto parentB = nodes.top();
        nodes.pop();

        parentA->bit = 1;
        parentB->bit = 0;

        const float mergedProb = parentB->probability + parentA->probability;
        auto mergedNode = std::make_shared<HuffmanNode>(mergedProb, parentA, parentB);
        nodes.push(mergedNode);
    }
    always_assert(nodes.size() == 1);

    Huffman result = {};
    const auto rootNode = nodes.top();
    result.root = rootNode;
    result.symbolMap = std::move(symbolMap);
    create_symbol_code(rootNode, {}, result.symbolCodes);

    return result;
}

inline void write_code(azgra::io::stream::OutMemoryBitStream &stream, const std::vector<bool> &code)
{
    for (const bool bit : code)
    {
        stream << bit;
    }
}

void huffman_encode(azgra::io::stream::OutMemoryBitStream &stream,
                    const Huffman &huffman,
                    const azgra::StringView textToEncode)
{
    // write number of symbols
    const size_t symbolCount = huffman.symbolMap.size();
    stream.write_value(symbolCount);

    // Write symbol and its occurrence count so we can build huffman tree when decompressing
    for (const auto&[symbol, info] : huffman.symbolMap)
    {
        stream.write_value(symbol);
        stream.write_value<uint16_t>(info.occurrenceCount);
    }

    // Write text size
    stream.write_value(static_cast<size_t>(textToEncode.size()));

    // Encode text
    for (const char symbol : textToEncode)
    {
        write_code(stream, huffman.symbolCodes.at(symbol));
    }
}

static Huffman decode_huffman_tree_from_stream(azgra::io::stream::InMemoryBitStream &stream,
                                               const size_t symbolCount)
{
    std::map<char, SymbolInfo> symbolMap;
    size_t totalOccurrence = 0;
    char symbol;
    uint16_t symbolOccurrence;

    for (size_t sId = 0; sId < symbolCount; ++sId)
    {
        symbol = stream.read_value<char>();
        symbolOccurrence = stream.read_value<uint16_t>();

        symbolMap[symbol] = SymbolInfo(symbolOccurrence);
        totalOccurrence += symbolOccurrence;
    }

    calculate_symbol_probability(symbolMap, totalOccurrence);

    return build_huffman_tree(symbolMap);
}

std::string huffman_decode(azgra::io::stream::InMemoryBitStream &stream)
{
    const auto symbolCount = stream.read_value<size_t>();
    const Huffman huffman = decode_huffman_tree_from_stream(stream, symbolCount);

    const auto expectedSymbolCount = stream.read_value<size_t>();
    std::vector<char> decodedBytes(expectedSymbolCount);

    bool bit;
    std::shared_ptr<HuffmanNode> currentNode;
    for (size_t i = 0; i < expectedSymbolCount; ++i)
    {
        currentNode = huffman.root;
        while (!currentNode->isLeaf)
        {
            bit = stream.read_bit();
            currentNode = currentNode->navigate(bit);
        }
        decodedBytes[i] = currentNode->symbol;
        // decode code
    }


    return std::string(reinterpret_cast<char const *>(decodedBytes.data()), decodedBytes.size());
}