#include "entropy.h"

double calculate_entropy(const std::map<azgra::byte , SymbolInfo> &symbolMap)
{
    double entropy = 0.0f;
    for (const auto &[symbol, info] : symbolMap)
    {
        entropy += (info.probability * log2(info.probability));
    }
    return -entropy;
}


static void find_all_symbols_in_text(std::map<azgra::byte, SymbolInfo> &symbolMap, const azgra::ByteSpan &data)
{
    for (std::size_t i = 0; i < data.size; ++i)
    {

        if (auto it = symbolMap.find(data[i]); it != symbolMap.cend())
        {
            ++symbolMap[data[i]];
            // Assert that we didn't overflow.
            assert(symbolMap[c].occurrenceCount != 0);
        }
        else
        {
            symbolMap[data[i]] = SymbolInfo(1);
        }
    }
}

std::map<azgra::byte, SymbolInfo> get_symbols_info(const azgra::ByteSpan &data)
{
    std::map<azgra::byte, SymbolInfo> symbolMap;

    find_all_symbols_in_text(symbolMap, data);

    const std::size_t totalSymbolCount = data.size;
    for (auto &[symbol, info] : symbolMap)
    {
        info.probability = static_cast<double>(info.occurrenceCount) / static_cast<double>(totalSymbolCount);
    }

    return symbolMap;
}