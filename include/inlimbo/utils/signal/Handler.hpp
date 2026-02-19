#pragma once

#include <atomic>
#include <csignal>

namespace utils::signal
{

class Handler
{
public:
  Handler(const Handler&)                    = delete;
  auto operator=(const Handler&) -> Handler& = delete;

  static auto getInstance() -> Handler&;

  void setup();

  [[nodiscard]] static auto shutdownRequested() -> bool;

private:
  Handler();
  ~Handler();

  static std::atomic<bool> s_shutdownRequested;

  static void installFatal(int sig);
  static void installGraceful(int sig);

  static void handleFatal(int signal, siginfo_t* info, void*);
  static void handleGraceful(int signal);
};

} // namespace utils::signal
