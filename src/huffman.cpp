#include <queue>
#include "huffman.h"

SymbolInfo &SymbolInfo::operator++()
{
    ++occurrenceCount;
    return *this;
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

inline void write_code(azgra::io::stream::OutMemoryBitStream &stream, const std::vector<bool> &code)
{
    for (const bool bit : code)
    {
        stream << bit;
    }
}

void huffman_encode(azgra::io::stream::OutMemoryBitStream &stream,
                    const huffman::HuffmanTree<azgra::StringView::value_type> &huffman,
                    const azgra::StringView textToEncode)
{
    // write number of symbols
    const size_t symbolCount = huffman.symbols.size();
    stream.write_value(symbolCount);

    // Write symbol and its occurrence count so we can build huffman tree when decompressing
    for (const auto&[symbol, info] : huffman.symbols)
    {
        stream.write_value(symbol);
        stream.write_value<uint16_t>(info.occurrenceCount);
    }

    // Write text size
    stream.write_value(static_cast<size_t>(textToEncode.size()));

    // Encode text
    for (const char symbol : textToEncode)
    {
        write_code(stream, huffman.symbols.at(symbol).code);
    }
}

static huffman::HuffmanTree<char> decode_huffman_tree_from_stream(azgra::io::stream::InMemoryBitStream &stream,
                                                                  const size_t symbolCount)
{
    std::vector<std::pair<char, size_t>> symbols(symbolCount);
    char symbol;
    uint16_t symbolOccurrence;

    for (size_t sId = 0; sId < symbolCount; ++sId)
    {
        symbol = stream.read_value<char>();
        symbolOccurrence = stream.read_value<uint16_t>();

        symbols[sId] = {symbol, symbolOccurrence};
    }

    return huffman::build_huffman_tree(symbols);
}

std::string huffman_decode(azgra::io::stream::InMemoryBitStream &stream)
{
    const auto symbolCount = stream.read_value<size_t>();
    const huffman::HuffmanTree<std::string::value_type> huffman = decode_huffman_tree_from_stream(stream, symbolCount);

    const auto expectedSymbolCount = stream.read_value<size_t>();
    std::vector<char> decodedBytes(expectedSymbolCount);

    bool bit;
    std::shared_ptr<huffman::HuffmanNode<char>> currentNode;
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