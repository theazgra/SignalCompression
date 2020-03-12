#pragma once

#include "span.h"
#include <type_traits>
#include <vector>
#include <algorithm>

template<typename T>
class ConstRingBufferIterator
{
private:
    T const *m_data;
    std::size_t m_ringBufferSize;
    long long m_headPosition;
public:

    // iterator traits
    using difference_type = long;
    using value_type = T;
    using pointer = T const *;
    using reference = T const &;
    using iterator_category = std::input_iterator_tag;

    explicit ConstRingBufferIterator(const T *begin,
                                     const std::size_t headPosition,
                                     const std::size_t ringBufferSize)
    {
        m_data = begin;
        m_headPosition = headPosition;
        m_ringBufferSize = ringBufferSize;
    }

    ConstRingBufferIterator const &operator--()
    {
        if (++m_headPosition >= m_ringBufferSize)
        {
            m_headPosition = 0;
        }
        return *this;
    }

    ConstRingBufferIterator operator++(int)
    {
        ConstRingBufferIterator result = *this;
        if (--m_headPosition < 0)
        {
            m_headPosition = m_ringBufferSize - 1;
        }
        return result;
    }

    ConstRingBufferIterator const &operator++()
    {
        if (--m_headPosition < 0)
        {
            m_headPosition = m_ringBufferSize - 1;
        }
        return *this;
    }

    ConstRingBufferIterator operator--(int)
    {
        ConstRingBufferIterator result = *this;
        if (++m_headPosition >= m_ringBufferSize)
        {
            m_headPosition = 0;
        }
        return result;
    }

    bool operator==(const ConstRingBufferIterator &other) const
    {
        return ((m_data == other.m_data) &&
                (m_headPosition == other.m_headPosition) &&
                (m_ringBufferSize == other.m_ringBufferSize));
    }

    bool operator!=(const ConstRingBufferIterator &other) const
    {
        return !(*this == other);
    }

    T const &operator*()
    {
        return m_data[m_headPosition];
    }
};

template<typename T>
class RingBuffer
{
private:
    /**
     * Underlying buffer.
     */
    std::vector<T> m_buffer{};

    /**
     * Maximum buffer size.
     */
    std::size_t m_bufferSize{0};

    /**
     * Old tail index.
     */
    long long m_tail{-1};

    /**
     * Flag whether the buffer is already full.
     */
    bool m_fullBuffer{false};

    /**
     * Get the index of the head. The oldest element in the buffer.
     * @return The index of the head.
     */
    [[nodiscard]] std::size_t head_index() const
    {
        if (!m_fullBuffer)
        {
            return 0;
        }
        else
        {
            return ((m_tail + 1) % m_bufferSize);
        }
    }

public:
    /**
     * Default constructor.
     */
    RingBuffer() = default;

    /**
     * Create ring buffer with given capacity.
     * @param capacity Buffer size.
     */
    explicit RingBuffer(const std::size_t capacity)
    {
        m_buffer.resize(capacity);
        m_bufferSize = capacity;
    }

    /**
     * Push single value to the ring buffer. Overwrite the oldest value if full.
     * @param value Value to push to the ring buffer.
     */
    void push(const T value)
    {
        m_buffer[++m_tail] = value;

        if (m_tail >= static_cast<long long> (m_bufferSize - 1))
        {
            m_fullBuffer = true;
            m_tail = -1;
        }
    }

    /**
     * Push values to the ring buffer. Overwrite the oldest values if full.
     * @tparam It Iterator type.
     * @tparam ItValueType Value type of iterator.
     * @param begin Begin copy iterator.
     * @param end End copy iterator.
     */
    template<
            typename It,
            typename ItValueType = typename std::iterator_traits<It>::value_type>
    void push(It begin, const It end)
    {
        static_assert(std::is_same_v<ItValueType, T>);
        std::for_each(begin, end, [&](const T value)
        {
            push(value);
        });
    }

    /**
     * Get the number of elements in the ting buffer.
     * @return Number of values in the buffer.
     */
    [[nodiscard]] std::size_t element_count() const
    {
        if (m_fullBuffer)
        { return m_bufferSize; }
        else
        { return ((m_tail - head_index()) + 1); }
    }

    /**
     * Get the total size of the ring buffer.
     * @return Total size of buffer.
     */
    [[nodiscard]] std::size_t size() const noexcept
    {
        return m_bufferSize;
    }

    /**
     * Return the value at the head.
     * @return
     */
    [[nodiscard]] T head() const
    {
        return m_buffer[head_index()];
    }

    /**
     * Get the iterator from the head position.
     * @return Iterator.
     */
    [[nodiscard]] ConstRingBufferIterator<T> head_iterator_begin() const
    {
        return ConstRingBufferIterator<T>(m_buffer.data(), head_index(), m_bufferSize);
    }
};