#pragma once

#include <azgra/azgra.h>

/**
 * Match in the LZSS Binary Search Tree.
 */
struct LzMatch
{
    /**
     * Distance to the matched string.
     */
    std::size_t distance{0};
    /**
     * Length of the matched string.
     */
    std::size_t length{0};

    LzMatch() = default;

    /**
     * Create the match.
     * @param dist Distance to the matched string.
     * @param len Length of the match.
     */
    explicit LzMatch(const std::size_t dist, std::size_t len) : distance(dist), length(len)
    {}
};
