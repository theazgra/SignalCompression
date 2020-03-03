#pragma once
#include <stdint.h>

struct SymbolInfo
{
    /**
     * Symbol occurrence count.
     */
    uint16_t occurrenceCount{};

    /**
     * Probability of the symbol.
     */
    float probability{};

    SymbolInfo() = default;


    explicit SymbolInfo(const uint16_t occurrence_) : occurrenceCount(occurrence_), probability(0.0f)
    {}

    explicit SymbolInfo(const uint16_t occurrence_, const float prob)
            : occurrenceCount(occurrence_), probability(prob)
    {}

    SymbolInfo &operator++();
};