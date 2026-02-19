#include "utils/signal/Handler.hpp"

#include <cstdlib>
#include <iostream>
#include <unistd.h>

#include "Logger.hpp"
#include "StackTrace.hpp"

namespace utils::signal
{

std::atomic<bool> Handler::s_shutdownRequested{false};

Handler::Handler()  = default;
Handler::~Handler() = default;

auto Handler::getInstance() -> Handler&
{
  static Handler instance;
  return instance;
}

void Handler::setup()
{
  installFatal(SIGABRT);
  installFatal(SIGSEGV);

  installGraceful(SIGINT);
  installGraceful(SIGTERM);
  installGraceful(SIGHUP);
}

auto Handler::shutdownRequested() -> bool
{
  return s_shutdownRequested.load(std::memory_order_relaxed);
}

void Handler::installFatal(int sig)
{
  struct sigaction sa = {};
  sa.sa_sigaction     = handleFatal;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_SIGINFO;
  sigaction(sig, &sa, nullptr);
}

void Handler::installGraceful(int sig)
{
  struct sigaction sa = {};
  sa.sa_handler       = handleGraceful;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sigaction(sig, &sa, nullptr);
}

void Handler::handleFatal(int signal, siginfo_t* info, void*)
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

void Handler::handleGraceful(int signal)
{
  const char* name = signal == SIGINT    ? "SIGINT (Ctrl+C)"
                     : signal == SIGTERM ? "SIGTERM"
                     : signal == SIGHUP  ? "SIGHUP"
                                         : "UNKNOWN";

  LOG_INFO("Received {}, exiting gracefully.", name);
  _Exit(EXIT_SUCCESS);
}

} // namespace utils::signal
