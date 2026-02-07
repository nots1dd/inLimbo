#pragma once

#include <atomic>
#include <memory>
#include <utility>

// A simple C++20 generic thread safe snapshot utility that does:
//
// Multiple reads on the container
// Single thread can write to snapshot
//
// When interfacing with the snapshot, the container is always consistent.
//
// Works by storing the generic as an atomic shared ptr (std::shared_ptr<const T>)
//
// Was created to reduce the verbosity of writing mutexes and concurrency handling.
// For frontend creators, this is a PITA so this container is perfect for fetching and storing
// anything in a thread safe manner.
//
// Mainly written for enabling hot-reloadble configs in frontend.

namespace utils
{

template <typename T>
class Snapshot
{
public:
  using Ptr = std::shared_ptr<const T>;

  Snapshot() : m_ptr(std::make_shared<const T>()) {}
  explicit Snapshot(T initial) : m_ptr(std::make_shared<const T>(std::move(initial))) {}

  // --------------------------
  // READ (lock-free)
  // --------------------------
  [[nodiscard]] auto get() const noexcept -> Ptr { return m_ptr.load(std::memory_order_acquire); }

  // --------------------------
  // WRITE (swap snapshot)
  // --------------------------
  auto set(T value) noexcept -> void
  {
    Ptr next = std::make_shared<const T>(std::move(value));
    m_ptr.store(std::move(next), std::memory_order_release);
  }

  // build new snapshot by copying old snapshot, mutating, swapping
  template <typename Fn>
  auto update(Fn&& fn) -> void
  {
    auto cur = get();

    // NOTE: creates a mutable copy, then stores as const snapshot
    T nextValue = *cur;
    fn(nextValue);
    set(std::move(nextValue));
  }

private:
  std::atomic<Ptr> m_ptr;
};

} // namespace utils
