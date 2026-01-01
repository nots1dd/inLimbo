#pragma once

#include <cstddef>
#include <mutex>
#include <vector>

template <typename T> class RingBuffer
{
private:
  std::vector<T>     m_buffer;
  size_t             m_capacity;
  size_t             m_readPos{0};
  size_t             m_writePos{};
  size_t             m_size{0};
  mutable std::mutex m_mutex;

public:
  explicit RingBuffer(size_t capacity) : m_buffer(capacity), m_capacity(capacity) {}

  // Returns number of elements actually written
  auto write(const T* data, size_t count) -> size_t
  {
    std::lock_guard<std::mutex> lock(m_mutex);

    size_t space   = m_capacity - m_size;
    size_t toWrite = std::min(count, space);

    for (size_t i = 0; i < toWrite; ++i)
    {
      m_buffer[m_writePos] = data[i];
      m_writePos           = (m_writePos + 1) % m_capacity;
    }

    m_size += toWrite;
    return toWrite;
  }

  // Returns number of elements actually read
  auto read(T* data, size_t count) -> size_t
  {
    std::lock_guard<std::mutex> lock(m_mutex);

    size_t toRead = std::min(count, m_size);

    for (size_t i = 0; i < toRead; ++i)
    {
      data[i]   = m_buffer[m_readPos];
      m_readPos = (m_readPos + 1) % m_capacity;
    }

    m_size -= toRead;
    return toRead;
  }

  auto capacity() const -> size_t
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_capacity;
  }

  auto available() const -> size_t
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_size;
  }

  auto space() const -> size_t
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_capacity - m_size;
  }

  void clear()
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_readPos  = 0;
    m_writePos = 0;
    m_size     = 0;
  }
};
