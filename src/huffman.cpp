#include <queue>
#include "huffman.h"

static void find_all_symbols(std::map<char, SymbolInfo> &symbolMap, const azgra::StringView &string)
{
    for (char c : string)
    {

        if (auto it = symbolMap.find(c); it != symbolMap.cend())
        {
            ++symbolMap[c];
        }
        else
        {
            symbolMap[c] = SymbolInfo(1);
        }
    }
}

static void calculate_symbol_probability(std::map<char, SymbolInfo> &symbolMap, const azgra::StringView &string)
{
    const auto totalSymbolCount = string.length();
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
    calculate_symbol_probability(symbolMap, string);

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
        symbolCodes[currentNode->symbol] = code;
    }
}

Huffman build_huffman_tree(const std::map<char, SymbolInfo> &symbolMap)
{
    std::set<std::shared_ptr<HuffmanNode>> nodeSet;

    // Initialize nodes from symbol map
    for (const auto &[symbol, info] : symbolMap)
    {
        nodeSet.insert(std::make_shared<HuffmanNode>(symbol, info.probability));
    }

    while (nodeSet.size() != 1)
    {
        // the lowest probability
        auto parentA = *--nodeSet.end();
        auto parentB = *--(--nodeSet.end());

        parentA->bit = 1;
        parentB->bit = 0;

        nodeSet.erase(parentB);
        nodeSet.erase(parentA);

        const float mergedProb = parentB->probability + parentA->probability;
        auto mergedNode = std::make_shared<HuffmanNode>(mergedProb, parentA, parentB);
        nodeSet.insert(mergedNode);
    }
    always_assert(nodeSet.size() == 1);

    Huffman result = {};
    result.root = *nodeSet.begin();
    create_symbol_code(*nodeSet.begin(), {}, result.symbolCodes);

    return result;
}