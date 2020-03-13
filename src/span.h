#pragma once

#include <azgra/azgra.h>

template<typename T>
struct span
{
    const T *ptr;
    std::size_t size;

    span() = default;

    explicit span(const T *data, const std::size_t dataSize) : ptr(data), size(dataSize)
    {
    }

    [[nodiscard]] span<T> sub_span(const std::size_t offset, const std::size_t subSpanSize) const
    {
        return span<T>(ptr + offset, subSpanSize);
    }

    [[nodiscard]] bool operator==(const span<T> &other) const
    {
        if (size != other.size)
            return false;
        for (size_t i = 0; i < size; ++i)
        {
            if (ptr[i] != other.ptr[i])
            {
                return false;
            }
        }
        return true;
    }

    [[nodiscard]] bool operator!=(const span<T> &other) const
    {
        return !(*this == other);
    }

    [[nodiscard]] bool operator<(const span<T> &other) const
    {
        const std::size_t toSize = std::min(size, other.size);
        for (size_t i = 0; i < toSize; ++i)
        {
            if (ptr[i] < other.ptr[i])
            { return true; }
            if (other.ptr[i] < ptr[i])
            { return false; }
        }
        if (size < other.size)
            return true;
        else if (other.size < size)
            return false;
        return false;
    }

    [[nodiscard]] std::size_t match_length(const span<T> &other) const
    {
        const std::size_t toSize = std::min(size, other.size);
        std::size_t len = 0;
        for (size_t i = 0; i < toSize; ++i)
        {
            if (ptr[i] != other.ptr[i])
            {
                return len;
            }
            ++len;
        }
        return len;
    }

};

typedef span<azgra::byte> ByteSpan;