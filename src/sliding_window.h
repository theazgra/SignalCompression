#pragma once

#include "span.h"
#include <memory>

template<typename T>
class SlidingWindow
{
private:
    const T *m_data{};
    long m_begin{0};
    std::size_t m_end{0};
    std::size_t m_size{0};
    std::size_t m_delimiter{0};
    std::size_t m_searchBufferSize{0};
public:
    SlidingWindow() = default;

    explicit SlidingWindow(const T *data,
                           const long beginIndex,
                           const std::size_t windowSize,
                           const std::size_t searchBufferSize)
    {
        m_data = data;
        m_begin = beginIndex;
        m_end = beginIndex + windowSize;
        m_size = windowSize;
        m_delimiter = m_begin + searchBufferSize;
        m_searchBufferSize = searchBufferSize;
    }

    void slide(const std::size_t offset)
    {
        m_begin += offset;
        m_end += offset;
        m_delimiter += offset;
    }

    inline T operator[](const long offsetFromLookAheadBuffer)
    {
        return m_data[m_delimiter + offsetFromLookAheadBuffer];
    }

    inline span<T> search_span()
    {
        span<T> searchSpan((m_data + m_delimiter), m_searchBufferSize);
        return searchSpan;
    }
};