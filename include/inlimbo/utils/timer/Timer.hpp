#pragma once

#include <chrono>
#include <functional>
#include <iostream>
#include <string_view>
#include <utility>

namespace util {


// ---------------------------------------------------------------------------
// Generic Time utility
// ---------------------------------------------------------------------------
inline auto timestamp_now() -> std::string {
    using namespace std::chrono;
    auto now = system_clock::now();
    auto t = system_clock::to_time_t(now);
    auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

    std::ostringstream oss;
    oss << std::put_time(std::localtime(&t), "%F %T")
        << "." << std::setw(3) << std::setfill('0') << ms.count();
    return oss.str();
}

/// High-resolution, type-safe timer for benchmarking or profiling.
///
/// Usage examples:
/// ```cpp
/// util::Timer<> t; // starts automatically
/// // ... code ...
/// std::cout << t.elapsed_ms() << " ms\n";
///
/// auto time_taken = util::Timer<>::time_function([]{ heavy_task(); });
/// std::cout << "heavy_task took " << time_taken << " ms\n";
///
/// {
///     util::ScopedTimer timer("Sorting block");
///     std::sort(vec.begin(), vec.end());
/// } // automatically prints duration on scope exit
/// ```
template <typename ClockT = std::chrono::steady_clock, typename DurationT = std::chrono::milliseconds>
class Timer {
public:
    using clock      = ClockT;
    using duration   = DurationT;
    using time_point = typename clock::time_point;

    Timer(bool start_now = true) noexcept {
        if (start_now)
            start();
    }

    /// Start or restart the timer
    void start() noexcept {
        start_time_ = clock::now();
        running_ = true;
    }

    /// Stop the timer
    void stop() noexcept {
        end_time_ = clock::now();
        running_ = false;
    }

    /// Reset the timer and start again
    void reset() noexcept {
        running_ = true;
        start_time_ = clock::now();
    }

    void restart() noexcept {
        reset();
        start();
    }

    /// Elapsed duration since start, or until stopped
    [[nodiscard]] auto elapsed() const noexcept {
        if (running_)
            return std::chrono::duration_cast<duration>(clock::now() - start_time_);
        else
            return std::chrono::duration_cast<duration>(end_time_ - start_time_);
    }

    [[nodiscard]] auto elapsed_ms() const noexcept -> double {
        return elapsed().count();
    }

    [[nodiscard]] auto elapsed_sec() const noexcept -> double {
        return std::chrono::duration<double>(elapsed()).count();
    }

    /// Utility: measure execution time of a callable (function, lambda, etc.)
    template <typename Func, typename... Args>
    static auto time_function(Func&& func, Args&&... args) -> double {
        Timer<ClockT, DurationT> timer;
        std::invoke(std::forward<Func>(func), std::forward<Args>(args)...);
        return timer.elapsed().count();
    }

private:
    time_point start_time_{};
    time_point end_time_{};
    bool running_{false};
};


/// RAII timer that automatically prints the elapsed time when destroyed.
///
/// ```cpp
/// {
///     util::ScopedTimer t("Processing");
///     do_something();
/// } // prints "[Processing] took 123.45 ms"
/// ```
template <typename ClockT = std::chrono::steady_clock, typename DurationT = std::chrono::milliseconds>
class ScopedTimer {
public:
    explicit ScopedTimer(std::string_view label = "", std::ostream& os = std::cout)
        : label_(label), os_(os) {
        timer_.start();
    }

    ~ScopedTimer() noexcept {
        timer_.stop();
        os_ << "[" << label_ << "] took " << timer_.elapsed().count() << " " 
            << duration_unit() << '\n';
    }

    ScopedTimer(const ScopedTimer&) = delete;
    auto operator=(const ScopedTimer&) -> ScopedTimer& = delete;

private:
    static constexpr auto duration_unit() -> std::string_view {
        if constexpr (std::is_same_v<DurationT, std::chrono::nanoseconds>) return "ns";
        else if constexpr (std::is_same_v<DurationT, std::chrono::microseconds>) return "µs";
        else if constexpr (std::is_same_v<DurationT, std::chrono::milliseconds>) return "ms";
        else if constexpr (std::is_same_v<DurationT, std::chrono::seconds>) return "s";
        else return "unknown units";
    }

    std::string_view label_;
    std::ostream& os_;
    Timer<ClockT, DurationT> timer_;
};

} // namespace util
