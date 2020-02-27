#pragma once

#include <map>
#include <azgra/azgra.h>
#include <memory>
#include <queue>

struct SymbolInfo
{
    size_t occurrenceCount{};
    float probability{};

    SymbolInfo() = default;


    explicit SymbolInfo(const size_t occurrence_) : occurrenceCount(occurrence_), probability(0.0f)
    {

    }

    explicit SymbolInfo(const size_t occurrence_, const float prob)
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
    std::shared_ptr<HuffmanNode> root;
    std::map<char, std::vector<bool>> symbolCodes;
};


std::map<char, SymbolInfo> get_string_symbols_info(const azgra::StringView &string);

Huffman build_huffman_tree(const std::map<char, SymbolInfo> &symbolMap);