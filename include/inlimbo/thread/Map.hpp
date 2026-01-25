#pragma once

#include <mutex>
#include <optional>
#include <shared_mutex>

namespace threads
{

template <typename TMap> class SafeMap
{
private:
  TMap                      m_map;
  mutable std::shared_mutex m_mtx;

public:
  SafeMap()                                  = default;
  SafeMap(const SafeMap&)                    = delete;
  auto operator=(const SafeMap&) -> SafeMap& = delete;

  SafeMap(SafeMap&& other) noexcept
  {
    std::unique_lock lock(other.m_mtx);
    m_map = std::move(other.m_map);
  }

  auto operator=(SafeMap&& other) noexcept -> SafeMap&
  {
    if (this != &other)
    {
      std::unique_lock lhs_lock(m_mtx, std::defer_lock);
      std::unique_lock rhs_lock(other.m_mtx, std::defer_lock);
      std::lock(lhs_lock, rhs_lock);
      m_map = std::move(other.m_map);
    }
    return *this;
  }

  void replace(TMap newMap)
  {
    std::unique_lock lock(m_mtx);
    m_map.swap(newMap); // O(1)
  }

  void clear()
  {
    std::unique_lock lock(m_mtx);
    m_map.clear();
  }

  // ---- Readers ----
  template <typename LookupFn>
  auto get(LookupFn&& fn) const -> std::optional<typename TMap::mapped_type>
  {
    std::shared_lock lock(m_mtx);
    return fn(m_map);
  }

  auto snapshot() const -> TMap
  {
    std::shared_lock lock(m_mtx);
    return m_map; // safe copy
  }

  auto empty() const -> bool
  {
    std::shared_lock lock(m_mtx);
    return m_map.empty();
  }

  // ---- Access for custom operations ----
  template <typename Fn> auto withReadLock(Fn&& fn) const -> decltype(auto)
  {
    std::shared_lock lock(m_mtx);
    return fn(m_map);
  }

  // this shud be used with the intention of modifying the map
  template <typename Fn> auto withWriteLock(Fn&& fn) -> decltype(auto)
  {
    std::unique_lock lock(m_mtx);
    return fn(m_map);
  }
};

} // namespace threads
