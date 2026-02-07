#pragma once

#include <atomic>
#include <cstddef>
#include <cstring>
#include <type_traits>
#include <vector>

namespace utils
{

template <typename T>
class RingBuffer
{
  static_assert(std::is_trivially_copyable_v<T>,
                "RingBuffer requires trivially copyable T for memcpy speed.");

public:
  explicit RingBuffer(size_t capacity) : m_capacity(capacity ? capacity : 1), m_data(m_capacity) {}

  [[nodiscard]] auto capacity() const noexcept -> size_t { return m_capacity; }

  // how many elements can be read
  [[nodiscard]] auto available() const noexcept -> size_t
  {
    const size_t r = m_read.load(std::memory_order_acquire);
    const size_t w = m_write.load(std::memory_order_acquire);
    return w - r;
  }

  // how many elements can be written
  [[nodiscard]] auto space() const noexcept -> size_t { return m_capacity - available(); }

  auto clear() noexcept -> void
  {
    m_read.store(0, std::memory_order_release);
    m_write.store(0, std::memory_order_release);
  }

  // producer thread
  auto write(const T* src, size_t count) noexcept -> size_t
  {
    if (!src || count == 0)
      return 0;

    const size_t cap = m_capacity;

    const size_t r = m_read.load(std::memory_order_acquire);
    size_t       w = m_write.load(std::memory_order_relaxed);

    const size_t used      = w - r;
    const size_t freeSpace = cap - used;
    const size_t toWrite   = (count < freeSpace) ? count : freeSpace;
    if (toWrite == 0)
      return 0;

    // Compute write index once
    const size_t wpos = w % cap;

    // Chunk 1: from wpos to end
    const size_t tillEnd = cap - wpos;
    const size_t first   = (toWrite < tillEnd) ? toWrite : tillEnd;

    std::memcpy(m_data.data() + wpos, src, first * sizeof(T));

    // Chunk 2: wrapped to beginning
    const size_t second = toWrite - first;
    if (second)
      std::memcpy(m_data.data(), src + first, second * sizeof(T));

    w += toWrite;
    m_write.store(w, std::memory_order_release);
    return toWrite;
  }

  // consumer thread
  auto read(T* dst, size_t count) noexcept -> size_t
  {
    if (!dst || count == 0)
      return 0;

    const size_t cap = m_capacity;

    size_t       r = m_read.load(std::memory_order_relaxed);
    const size_t w = m_write.load(std::memory_order_acquire);

    const size_t avail  = w - r;
    const size_t toRead = (count < avail) ? count : avail;
    if (toRead == 0)
      return 0;

    // Compute read index once
    const size_t rpos = r % cap;

    // Chunk 1: from rpos to end
    const size_t tillEnd = cap - rpos;
    const size_t first   = (toRead < tillEnd) ? toRead : tillEnd;

    std::memcpy(dst, m_data.data() + rpos, first * sizeof(T));

    // Chunk 2: wrapped to beginning
    const size_t second = toRead - first;
    if (second)
      std::memcpy(dst + first, m_data.data(), second * sizeof(T));

    r += toRead;
    m_read.store(r, std::memory_order_release);
    return toRead;
  }

private:
  size_t         m_capacity{};
  std::vector<T> m_data;

  std::atomic<size_t> m_read{0};
  std::atomic<size_t> m_write{0};
};

} // namespace utils
