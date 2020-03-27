#pragma once

#include <stdint.h>

struct SymbolInfo
{
    /**
     * Symbol occurrence count.
     */
    uint32_t occurrenceCount{};

    /**
     * Probability of the symbol.
     */
    double probability{};

    SymbolInfo() = default;


    explicit SymbolInfo(const uint32_t occurrence_) : occurrenceCount(occurrence_), probability(0.0f)
    {}

    explicit SymbolInfo(const uint32_t occurrence_, const double prob)
            : occurrenceCount(occurrence_), probability(prob)
    {}

    constexpr SymbolInfo &operator++() noexcept
    {
        ++occurrenceCount;
        return *this;
    }
};