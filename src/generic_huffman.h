#pragma once

#include <azgra/always_on_assert.h>
#include <memory>
#include <vector>
#include <queue>
#include <map>
#include <azgra/collection/enumerable_functions.h>

namespace huffman
{

    template<typename SymbolType>
    struct HuffmanNode
    {
        SymbolType symbol{};

        int bit = -1;
        bool isLeaf = false;
        float probability{};
        size_t symbolOccurrence{9999999};

        std::shared_ptr<HuffmanNode<SymbolType>> parentA{};
        std::shared_ptr<HuffmanNode<SymbolType>> parentB{};

        HuffmanNode() = default;

        explicit HuffmanNode(SymbolType symbol_,
                             const size_t occurrence_,
                             float probability_)
        {
            symbol = symbol_;
            symbolOccurrence = occurrence_;
            probability = probability_;
        }

        explicit HuffmanNode(float probability_) : probability(probability_)
        {
        }

        explicit HuffmanNode(float probability_, std::shared_ptr<HuffmanNode> &parentB_, std::shared_ptr<HuffmanNode> &parentA_)
                : probability(probability_), parentA(parentA_), parentB(parentB_)
        {
        }

        bool operator<(const HuffmanNode &other) const
        { return (probability < other.probability); }

        bool operator==(const HuffmanNode &other) const
        { return (symbol == other.symbol); }

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

    template<typename SymbolType>
    struct HuffmanNodeComparerGreater
    {
        inline bool operator()(const std::shared_ptr<HuffmanNode<SymbolType>> &a, const std::shared_ptr<HuffmanNode<SymbolType>> &b) const
        {
            return (a->probability > b->probability);
        }
    };

    struct HuffmanSymbolInfo
    {
        size_t occurrenceCount{};
        std::vector<bool> code{};

        HuffmanSymbolInfo() = default;

        explicit HuffmanSymbolInfo(const size_t occurrenceCount_, std::vector<bool> &code_)
        {
            occurrenceCount = occurrenceCount_;
            code = std::move(code_);
        }

        HuffmanSymbolInfo(const HuffmanSymbolInfo &) = delete;
    };

    template<typename SymbolType>
    struct HuffmanTree
    {
        ////    std::map<char, SymbolInfo> symbolMap;
        std::shared_ptr<HuffmanNode<SymbolType>> root;
        std::map<SymbolType, HuffmanSymbolInfo> symbols;
    };

    namespace
    {
        template<typename SymbolType>
        static void create_symbol_code(const std::shared_ptr<HuffmanNode<SymbolType>> &currentNode,
                                       std::vector<bool> code,
                                       std::map<SymbolType, HuffmanSymbolInfo> &symbols)
        {
            bool leaf = true;

            if (currentNode->bit != -1)
                code.emplace_back(currentNode->bit == 1);

            if (currentNode->parentA)
            {
                create_symbol_code(currentNode->parentA, code, symbols);
                leaf = false;
            }
            if (currentNode->parentB)
            {
                create_symbol_code(currentNode->parentB, code, symbols);
                leaf = false;
            }
            if (leaf)
            {
                currentNode->isLeaf = true;
                symbols[currentNode->symbol] = HuffmanSymbolInfo(currentNode->symbolOccurrence, code);
            }
        }
    } // namespace

    template<typename SymbolType>
    HuffmanTree<SymbolType> build_huffman_tree(const std::vector<std::pair<SymbolType, size_t>> &symbolOccurrences)
    {
        size_t totalSymbolOccurrence = azgra::collection::sum(symbolOccurrences.begin(),
                                                              symbolOccurrences.end(),
                                                              [](const auto &pair)
                                                              { return pair.second; }, 0);

        // NOTE(Moravec):   We are not able to use std::unique_ptr with std::priority_queue
        //                  because top returns const&, which can't be moved.

        std::priority_queue<std::shared_ptr<HuffmanNode<SymbolType>>,
                std::vector<std::shared_ptr<HuffmanNode<SymbolType>>>,
                HuffmanNodeComparerGreater<SymbolType>>
                nodes;

        // Initialize nodes from symbol map
        for (const auto &[symbol, occurence] : symbolOccurrences)
        {
            const float symbolProbability = static_cast<float>(occurence) / static_cast<float>(totalSymbolOccurrence);
            nodes.push(std::make_shared<HuffmanNode<SymbolType>>(symbol, occurence, symbolProbability));
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
            auto mergedNode = std::make_shared<HuffmanNode<SymbolType>>(mergedProb, parentA, parentB);
            nodes.push(mergedNode);
        }
        always_assert(nodes.size() == 1);

        HuffmanTree<SymbolType> result = {};
        const auto rootNode = nodes.top();
        result.root = rootNode;
        create_symbol_code(rootNode, {}, result.symbols);

        return result;
    }

} // namespace huffman