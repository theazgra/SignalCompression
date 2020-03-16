#pragma once

#include "Span.h"
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
    std::size_t m_lookAheadBufferSize{0};
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
        m_searchBufferSize = searchBufferSize;
        m_delimiter = m_begin + searchBufferSize;
        m_lookAheadBufferSize = windowSize - searchBufferSize;
    }

    void slide(const std::size_t offset)
    {
        m_begin += offset;
        m_end += offset;
        m_delimiter += offset;
    }

    inline T operator[](const long offsetFromLookAheadBuffer) const
    {
        return m_data[m_delimiter + offsetFromLookAheadBuffer];
    }

    [[nodiscard]] inline Span<T> span_from_delimiter(const long offset, const std::size_t spanSize)
    {
        Span<T> searchSpan((m_data + (m_delimiter + offset)), spanSize);
        return searchSpan;
    }

    [[nodiscard]] inline Span<T> span_from_delimiter(const long offset)
    {
        return span_from_delimiter(offset, m_lookAheadBufferSize);
    }

    [[nodiscard]] inline Span<T> span_from_begin(const long offset)
    {
        Span<T> searchSpan((m_data + (m_begin + offset)), m_lookAheadBufferSize);
        return searchSpan;
    }

    [[nodiscard]] long begin_index() const
    {
        return m_begin;
    }

    [[nodiscard]] std::size_t in_search_buffer() const
    {
        return std::min(m_delimiter, m_searchBufferSize);
    }
};