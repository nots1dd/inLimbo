#pragma once

#include <atomic>
#include <memory>
#include <optional>

namespace threads
{

/*
threads::SafeMap is a thread-safe wrapper around a map-like container using an atomic
std::shared_ptr. Readers access immutable snapshots without locking, while
writers modify a private copy and atomically publish it.

Reader Behavior
- Lock-free
- Atomic snapshot load
- Always see consistent data
- No read/write blocking

Writer Behavior:
- Copy current map (most expensive downside)
- Apply modifications
- Atomically replace snapshot
- Old data stays alive while readers use it

Advantages:
- Very fast reads
- No reader blocking
- Strong write safety
- Simple concurrency model

Limitations:
- Writes are O(N) due to full copy
- Temporary memory spikes during writes
- Readers may see slightly stale data
- Requires copyable map type
- Not ideal for write-heavy workloads

Optimized for many readers and few writers. Trades write cost for safe,
lock-free read performance.

*/

template <typename TMap>
class SafeMap
{
private:
  std::atomic<std::shared_ptr<TMap>> m_mapPtr;

public:
  SafeMap() { m_mapPtr.store(std::make_shared<TMap>(), std::memory_order_release); }

  SafeMap(const SafeMap&)                    = delete;
  auto operator=(const SafeMap&) -> SafeMap& = delete;

  SafeMap(SafeMap&& other) noexcept
  {
    auto ptr = std::atomic_load(&other.m_mapPtr);
    std::atomic_store(&m_mapPtr, std::move(ptr));
  }

  auto operator=(SafeMap&& other) noexcept -> SafeMap&
  {
    if (this != &other)
    {
      auto ptr = std::atomic_load(&other.m_mapPtr);
      std::atomic_store(&m_mapPtr, std::move(ptr));
    }
    return *this;
  }

  // -------------------------------------------------
  // WRITERS
  // -------------------------------------------------

  void replace(TMap newMap)
  {
    auto newPtr = std::make_shared<TMap>(std::move(newMap));
    std::atomic_store(&m_mapPtr, std::move(newPtr));
  }

  void clear()
  {
    auto newPtr = std::make_shared<TMap>();
    std::atomic_store(&m_mapPtr, std::move(newPtr));
  }

  // -------------------------------------------------
  // READERS
  // -------------------------------------------------

  template <typename LookupFn>
  auto get(LookupFn&& fn) const -> std::optional<typename TMap::mapped_type>
  {
    auto ptr = std::atomic_load(&m_mapPtr);
    return fn(*ptr);
  }

  auto snapshot() const -> TMap
  {
    auto ptr = std::atomic_load(&m_mapPtr);
    return *ptr; // copy
  }

  [[nodiscard]] auto empty() const -> bool
  {
    auto ptr = std::atomic_load(&m_mapPtr);
    return ptr->empty();
  }

  template <typename Fn>
  auto withReadLock(Fn&& fn) const -> decltype(auto)
  {
    auto ptr = std::atomic_load(&m_mapPtr);

    if constexpr (std::is_void_v<std::invoke_result_t<Fn, const TMap&>>)
    {
      fn(*ptr);
    }
    else
    {
      return fn(*ptr);
    }
  }

  template <typename Fn>
  auto withWriteLock(Fn&& fn) -> decltype(auto)
  {
    auto oldPtr = std::atomic_load(&m_mapPtr);
    auto newPtr = std::make_shared<TMap>(*oldPtr);

    if constexpr (std::is_void_v<std::invoke_result_t<Fn, TMap&>>)
    {
      fn(*newPtr);
      std::atomic_store(&m_mapPtr, std::move(newPtr));
    }
    else
    {
      auto result = fn(*newPtr);
      std::atomic_store(&m_mapPtr, std::move(newPtr));
      return result;
    }
  }
};

} // namespace threads
