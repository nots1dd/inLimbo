#pragma once

#include <atomic>
#include <csignal>
#include <cstdlib>
#include <unistd.h>

#include "Logger.hpp"
#include "StackTrace.hpp"

namespace utils
{

class SignalHandler
{
public:
  SignalHandler(const SignalHandler&)                    = delete;
  auto operator=(const SignalHandler&) -> SignalHandler& = delete;

  static auto getInstance() -> SignalHandler&
  {
    static SignalHandler instance;
    return instance;
  }

  void setup()
  {
    installFatal(SIGABRT);
    installFatal(SIGSEGV);

    installGraceful(SIGINT);  // Ctrl+C
    installGraceful(SIGTERM); // kill
    installGraceful(SIGHUP);  // terminal closed
  }

  [[nodiscard]] static auto shutdownRequested() -> bool
  {
    return s_shutdownRequested.load(std::memory_order_relaxed);
  }

private:
  SignalHandler()  = default;
  ~SignalHandler() = default;

  static inline std::atomic<bool> s_shutdownRequested{false};

  static void installFatal(int sig)
  {
    struct sigaction sa{};
    sa.sa_sigaction = handleFatal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_SIGINFO;
    sigaction(sig, &sa, nullptr);
  }

  static void installGraceful(int sig)
  {
    struct sigaction sa{};
    sa.sa_handler = handleGraceful;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(sig, &sa, nullptr);
  }

  // -----------------------------
  // Fatal crash handler (will print backtrace)
  // -----------------------------
  static void handleFatal(int signal, siginfo_t* info, void*)
  {
    const char* name = signal == SIGSEGV ? "SIGSEGV" : signal == SIGABRT ? "SIGABRT" : "UNKNOWN";

    LOG_CRITICAL("Fatal signal caught: {} ({})", name, signal);

    if (info && info->si_addr)
      LOG_CRITICAL("Fault address: {}", fmt::ptr(info->si_addr));

    LOG_CRITICAL("Printing backtrace:");
    DUMP_TRACE();

    LOG_CRITICAL("Application terminated due to fatal signal.");
    _Exit(EXIT_FAILURE);
  }

  static void handleGraceful(int signal)
  {
    const char* name = signal == SIGINT    ? "SIGINT (Ctrl+C)"
                       : signal == SIGTERM ? "SIGTERM"
                       : signal == SIGHUP  ? "SIGHUP"
                                           : "UNKNOWN";

    LOG_INFO("Received {}, exiting gracefully.", name);
    _Exit(EXIT_SUCCESS);
  }
};

} // namespace utils
