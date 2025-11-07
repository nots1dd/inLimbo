#pragma once

#include <mutex>
#include <shared_mutex>
#include <optional>

namespace threads {

template <typename TMap>
class SafeMap {
private:
    TMap map_;
    mutable std::shared_mutex mtx_;

public:
    SafeMap() = default;
    SafeMap(const SafeMap&) = delete;
    auto operator=(const SafeMap&) -> SafeMap& = delete;

    SafeMap(SafeMap&& other) noexcept {
        std::unique_lock lock(other.mtx_);
        map_ = std::move(other.map_);
    }

    auto operator=(SafeMap&& other) noexcept -> SafeMap& {
        if (this != &other) {
            std::unique_lock lhs_lock(mtx_, std::defer_lock);
            std::unique_lock rhs_lock(other.mtx_, std::defer_lock);
            std::lock(lhs_lock, rhs_lock);
            map_ = std::move(other.map_);
        }
        return *this;
    }

    void replace(TMap newMap) {
        std::unique_lock lock(mtx_);
        map_.swap(newMap); // O(1)
    }

    void clear() {
        std::unique_lock lock(mtx_);
        map_.clear();
    }

    // ---- Readers ----
    template <typename LookupFn>
    auto get(LookupFn&& fn) const -> std::optional<typename TMap::mapped_type> {
        std::shared_lock lock(mtx_);
        return fn(map_);
    }

    auto snapshot() const -> TMap {
        std::shared_lock lock(mtx_);
        return map_; // safe copy
    }

    auto empty() const -> bool {
        std::shared_lock lock(mtx_);
        return map_.empty();
    }
    
    // ---- Access for custom operations ----
    template <typename Fn>
    auto withReadLock(Fn&& fn) const -> decltype(auto) {
        std::shared_lock lock(mtx_);
        return fn(map_);
    }

    template <typename Fn>
    auto withWriteLock(Fn&& fn) -> decltype(auto) {
        std::unique_lock lock(mtx_);
        return fn(map_);
    }
};

} // namespace thread
