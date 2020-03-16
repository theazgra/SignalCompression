#pragma once

#include "lz_match.h"

class LzssToken
{
private:
    /**
     * Only token created by one of the two static method is valid.
     */
    bool m_valid{false};

    /**
     * True if is pair of distance and length.
     */
    bool m_isPair{false};

    /**
     * Raw byte data.
     */
    azgra::byte m_byte{0};

    /**
     * Pair match.
     */
    LzMatch m_match;
public:
    LzssToken() = default;

    [[nodiscard]] static LzssToken RawByteToken(const azgra::byte byte);

    [[nodiscard]] static LzssToken PairToken(const LzMatch &match);

    [[nodiscard]] bool is_valid() const;

    [[nodiscard]] bool is_pair() const;

    [[nodiscard]] azgra::byte get_byte() const;

    [[nodiscard]] LzMatch get_match() const;
};