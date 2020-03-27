#pragma once

#include <map>
#include <azgra/azgra.h>
#include <azgra/span.h>
#include "symbol_info.h"


template <typename T>
double calculate_entropy(const std::map<T, SymbolInfo> &symbolMap)
{
    double entropy = 0.0f;
    for (const auto &[symbol, info] : symbolMap)
    {
        entropy += (info.probability * log2(info.probability));
    }
    return -entropy;
}

template <typename T>
static void find_all_symbols_in_text(std::map<T, SymbolInfo> &symbolMap, const std::vector<T> &data)
{
    for (std::size_t i = 0; i < data.size(); ++i)
    {
        if (auto it = symbolMap.find(data[i]); it != symbolMap.cend())
        {
            ++symbolMap[data[i]];
            // Assert that we didn't overflow.
            assert(symbolMap[data[i]].occurrenceCount != 0);
        }
        else
        {
            symbolMap[data[i]] = SymbolInfo(1);
        }
    }
}

template <typename T>
std::map<T, SymbolInfo> get_symbols_info(const std::vector<T> &data)
{
    std::map<T, SymbolInfo> symbolMap;

    find_all_symbols_in_text(symbolMap, data);

    const std::size_t totalSymbolCount = data.size();
    for (auto &[symbol, info] : symbolMap)
    {
        info.probability = static_cast<double>(info.occurrenceCount) / static_cast<double>(totalSymbolCount);
    }

    return symbolMap;
}