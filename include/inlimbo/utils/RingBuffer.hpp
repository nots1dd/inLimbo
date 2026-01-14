#pragma once

#include <atomic>
#include <cstddef>
#include <cstring>
#include <memory>
#include <type_traits>

namespace utils
{

static inline auto next_pow2(size_t x) -> size_t
{
  if (x <= 1) return 1;
  --x;
  x |= x >> 1;
  x |= x >> 2;
  x |= x >> 4;
  x |= x >> 8;
  x |= x >> 16;
  if constexpr (sizeof(size_t) == 8)
    x |= x >> 32;
  return x + 1;
}

// A Single producer, Single consumer generic ring buffer
//
// It decides the capacity on the next_pow2 rule.
template <typename T>
class RingBuffer
{
  // although we will never hit this (as RingBuffer will be mostly containing primitive dtypes)
  // This may come in handy?
  static_assert(std::is_trivially_copyable_v<T>,
                "RingBuffer requires trivially copyable T for memcpy speed.");

public:
  explicit RingBuffer(size_t capacity)
  {
    m_capacity = next_pow2(capacity);
    m_mask     = m_capacity - 1;
    m_data     = std::make_unique<T[]>(m_capacity);
  }

  [[nodiscard]] auto capacity() const noexcept -> size_t { return m_capacity; }

  // how many elements can be read
  [[nodiscard]] auto available() const noexcept -> size_t
  {
    const size_t r = m_read.load(std::memory_order_acquire);
    const size_t w = m_write.load(std::memory_order_acquire);
    return w - r;
  }

  // how many elements can be written
  [[nodiscard]] auto space() const noexcept -> size_t
  {
    return m_capacity - available();
  }

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

    const size_t r = m_read.load(std::memory_order_acquire);
    size_t       w = m_write.load(std::memory_order_relaxed);

    size_t freeSpace = m_capacity - (w - r);
    size_t toWrite   = (count < freeSpace) ? count : freeSpace;
    if (toWrite == 0)
      return 0;

    size_t wpos = w & m_mask;

    // first chunk up to end
    size_t first = toWrite;
    size_t tillEnd = m_capacity - wpos;
    if (first > tillEnd)
      first = tillEnd;

    std::memcpy(m_data.get() + wpos, src, first * sizeof(T));

    // wrap chunk
    size_t second = toWrite - first;
    if (second > 0)
      std::memcpy(m_data.get(), src + first, second * sizeof(T));

    w += toWrite;
    m_write.store(w, std::memory_order_release);
    return toWrite;
  }

  // consumer thread
  auto read(T* dst, size_t count) noexcept -> size_t
  {
    if (!dst || count == 0)
      return 0;

    size_t       r = m_read.load(std::memory_order_relaxed);
    const size_t w = m_write.load(std::memory_order_acquire);

    size_t avail  = w - r;
    size_t toRead = (count < avail) ? count : avail;
    if (toRead == 0)
      return 0;

    size_t rpos = r & m_mask;

    size_t first = toRead;
    size_t tillEnd = m_capacity - rpos;
    if (first > tillEnd)
      first = tillEnd;

    std::memcpy(dst, m_data.get() + rpos, first * sizeof(T));

    size_t second = toRead - first;
    if (second > 0)
      std::memcpy(dst + first, m_data.get(), second * sizeof(T));

    r += toRead;
    m_read.store(r, std::memory_order_release);
    return toRead;
  }

private:
  std::unique_ptr<T[]> m_data;

  size_t m_capacity{};
  size_t m_mask{};

  // NOTE: these are monotonic counters (not positions), mask gives index
  std::atomic<size_t> m_read{0};
  std::atomic<size_t> m_write{0};
};

} // namespace utils
