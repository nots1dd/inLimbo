#pragma once

#include <chrono>
#include <functional>
#include <iostream>
#include <string_view>
#include <utility>

namespace utils
{

namespace timer
{

// ---------------------------------------------------------------------------
// Generic Time utility
// ---------------------------------------------------------------------------
inline auto timestamp_now() -> std::string
{
  using namespace std::chrono;
  auto now = system_clock::now();
  auto t   = system_clock::to_time_t(now);
  auto ms  = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

  std::ostringstream oss;
  oss << std::put_time(std::localtime(&t), "%F %T") << "." << std::setw(3) << std::setfill('0')
      << ms.count();
  return oss.str();
}

[[nodiscard]] inline auto nowUnix() noexcept -> std::int64_t
{
  using namespace std::chrono;
  return duration_cast<seconds>(system_clock::now().time_since_epoch()).count();
}

inline static auto fmtTime(double sec) -> std::string
{
  int s = static_cast<int>(sec);
  int m = s / 60;
  s %= 60;

  std::ostringstream oss;
  oss << std::setw(2) << std::setfill('0') << m << ":" << std::setw(2) << std::setfill('0') << s;
  return oss.str();
}

} // namespace timer

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
template <typename ClockT    = std::chrono::steady_clock,
          typename DurationT = std::chrono::milliseconds>
class Timer
{
public:
  using clock      = ClockT;
  using duration   = DurationT;
  using time_point = typename clock::time_point;

  Timer(bool start_now = true) noexcept
  {
    if (start_now)
      start();
  }

  /// Start or restart the timer
  void start() noexcept
  {
    m_startTime = clock::now();
    m_isRunning = true;
  }

  /// Stop the timer
  void stop() noexcept
  {
    m_endTime   = clock::now();
    m_isRunning = false;
  }

  /// Reset the timer and start again
  void reset() noexcept
  {
    m_isRunning = true;
    m_startTime = clock::now();
  }

  void restart() noexcept
  {
    reset();
    start();
  }

  /// Elapsed duration since start, or until stopped
  [[nodiscard]] auto elapsed() const noexcept
  {
    if (m_isRunning)
      return std::chrono::duration_cast<duration>(clock::now() - m_startTime);
    else
      return std::chrono::duration_cast<duration>(m_endTime - m_startTime);
  }

  [[nodiscard]] auto elapsed_ms() const noexcept -> double { return elapsed().count(); }

  [[nodiscard]] auto elapsed_sec() const noexcept -> double
  {
    return std::chrono::duration<double>(elapsed()).count();
  }

  /// Utility: measure execution time of a callable (function, lambda, etc.)
  template <typename Func, typename... Args>
  static auto time_function(Func&& func, Args&&... args) -> double
  {
    Timer<ClockT, DurationT> timer;
    std::invoke(std::forward<Func>(func), std::forward<Args>(args)...);
    return timer.elapsed().count();
  }

private:
  time_point m_startTime{};
  time_point m_endTime{};
  bool       m_isRunning{false};
};

/// RAII timer that automatically prints the elapsed time when destroyed.
///
/// ```cpp
/// {
///     util::ScopedTimer t("Processing");
///     do_something();
/// } // prints "[Processing] took 123.45 ms"
/// ```
template <typename ClockT    = std::chrono::steady_clock,
          typename DurationT = std::chrono::milliseconds>
class ScopedTimer
{
public:
  explicit ScopedTimer(std::string_view label = "", std::ostream& os = std::cout)
      : m_label(label), m_ostream(os)
  {
    m_timer.start();
  }

  ~ScopedTimer() noexcept
  {
    m_timer.stop();
    m_ostream << "[" << m_label << "] took " << m_timer.elapsed().count() << " " << duration_unit()
              << '\n';
  }

  ScopedTimer(const ScopedTimer&)                    = delete;
  auto operator=(const ScopedTimer&) -> ScopedTimer& = delete;

private:
  static constexpr auto duration_unit() -> std::string_view
  {
    if constexpr (std::is_same_v<DurationT, std::chrono::nanoseconds>)
      return "ns";
    else if constexpr (std::is_same_v<DurationT, std::chrono::microseconds>)
      return "µs";
    else if constexpr (std::is_same_v<DurationT, std::chrono::milliseconds>)
      return "ms";
    else if constexpr (std::is_same_v<DurationT, std::chrono::seconds>)
      return "s";
    else
      return "unknown units";
  }

  std::string_view         m_label;
  std::ostream&            m_ostream;
  Timer<ClockT, DurationT> m_timer;
};

} // namespace utils
