#include "lzss_token.h"

LzssToken LzssToken::RawByteToken(const azgra::byte byte)
{
    LzssToken token;
    token.m_valid = true;
    token.m_isPair = false;
    token.m_byte = byte;
    return token;
}

LzssToken LzssToken::PairToken(const LzMatch &match)
{
    LzssToken token;
    token.m_valid = true;
    token.m_isPair = true;
    token.m_match = match;
    return token;
}

bool LzssToken::is_valid() const
{
    return m_valid;
}

bool LzssToken::is_pair() const
{
    return m_isPair;
}

azgra::byte LzssToken::get_byte() const
{
    return m_byte;
}

LzMatch LzssToken::get_match() const
{
    return m_match;
}

