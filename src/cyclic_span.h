#pragma once

#include <azgra/span.h>


template<typename T>
class CyclicSpan final : public azgra::Span<T>
{
public:
    CyclicSpan() : azgra::Span<T>()
    {}

    explicit CyclicSpan(const T *data, const std::size_t dataSize, const std::size_t offset)
            : azgra::Span<T>{data, dataSize}, m_offset(offset)
    {

    }

    [[nodiscard]]  inline T operator[](const std::size_t index) const override
    {
        return this->m_ptr[(index + m_offset) % this->m_size];
    }

    [[nodiscard]] inline bool operator<(const CyclicSpan<T> &other) const
    {
        return (lexicographic_compare(other) < 0);
    }

    [[nodiscard]] int lexicographic_compare(const CyclicSpan<T> &other) const
    {
        const std::size_t count = std::min(this->m_size, other.m_size);
        for (size_t i = 0; i < count; ++i)
        {
            if (operator[](i) < other[i])
            { return -1; }
            if (other[i] < operator[](i))
            { return 1; }
        }
        if (this->m_size != other.m_size)
        {
            if (this->m_size < other.m_size)
                return -1;
            else if (other.m_size < this->m_size)
                return 1;
        }
        return 0;
    }

    [[nodiscard]] std::size_t offset() const
    { return m_offset; }

private:

    std::size_t m_offset{0};
};
