#pragma once

#include <map>
#include <azgra/azgra.h>
#include <memory>
#include <queue>
#include <azgra/io/stream/memory_bit_stream.h>

struct SymbolInfo
{
    uint16_t occurrenceCount{};
    float probability{};

    SymbolInfo() = default;


    explicit SymbolInfo(const uint16_t occurrence_) : occurrenceCount(occurrence_), probability(0.0f)
    {

    }

    explicit SymbolInfo(const uint16_t occurrence_, const float prob)
            : occurrenceCount(occurrence_), probability(prob)
    {

    }

    SymbolInfo &operator++()
    {
        ++occurrenceCount;
        return *this;
    }
};

struct HuffmanNode
{
    char symbol{};
    // Uninitialized to -1.
    int bit = -1;
    bool isLeaf = false;

    std::shared_ptr<HuffmanNode> parentA{};
    std::shared_ptr<HuffmanNode> parentB{};

    float probability{};

    HuffmanNode() = default;

    explicit HuffmanNode(char symbol_, float prob) : symbol(symbol_), probability(prob)
    {}

    explicit HuffmanNode(float prob) : probability(prob)
    {}

    explicit HuffmanNode(float prob, std::shared_ptr<HuffmanNode> &pb, std::shared_ptr<HuffmanNode> &pa)
            : probability(prob)
    {
        parentB = pb;
        parentA = pa;
    }


    inline bool operator<(const HuffmanNode &other) const
    {
        return (probability < other.probability);
    }

    inline bool operator==(const HuffmanNode &other) const
    {
        return symbol == other.symbol;
    }

    std::shared_ptr<HuffmanNode> navigate(const bool bit)
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
};

struct HuffmanNodeComparerGreater
{
    inline bool operator()(const std::shared_ptr<HuffmanNode> &a, const std::shared_ptr<HuffmanNode> &b) const
    {
        return (a->probability > b->probability);
    }
};

struct Huffman
{
    std::map<char, SymbolInfo> symbolMap;
    std::shared_ptr<HuffmanNode> root;
    std::map<char, std::vector<bool>> symbolCodes;
};


std::map<char, SymbolInfo> get_string_symbols_info(const azgra::StringView &string);

Huffman build_huffman_tree(std::map<char, SymbolInfo> &symbolMap);

void huffman_encode(azgra::io::stream::OutMemoryBitStream &stream,
                    const Huffman &huffman,
                    const azgra::StringView textToEncode);

std::string huffman_decode(azgra::io::stream::InMemoryBitStream &stream);