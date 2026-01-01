#pragma once

#include "utils/Env-Vars.hpp"
#include <sstream>
#include <string>
#include <unistd.h>
#include <vector>

namespace core
{

inline auto is_stack_trace_enabled() -> bool
{
  static const bool enabled = []() -> bool
  {
    if (const char* v = std::getenv(INLIMBO_STACK_TRACE_DUMP_ENV))
    {
      std::string s(v);
      for (auto& c : s)
        c = static_cast<char>(std::tolower(c));
      return s == "1" || s == "true" || s == "yes" || s == "on";
    }
    return false;
  }();
  return enabled;
}

inline auto use_color() -> bool { return isatty(fileno(stdout)); }

inline auto color(const char* c, const std::string& s) -> std::string
{
  if (!use_color())
    return s;
  return std::string(c) + s + "\033[0m";
}

inline auto bold(const std::string& s) { return color("\033[1m", s); }
inline auto cyan(const std::string& s) { return color("\033[36m", s); }
inline auto green(const std::string& s) { return color("\033[32m", s); }
inline auto red(const std::string& s) { return color("\033[31m", s); }
inline auto dim(const std::string& s) { return color("\033[2m", s); }
inline auto yellow(const std::string& s) { return color("\033[33m", s); }

inline auto simplify_symbol(std::string s) -> std::string
{
  auto cut = [&](const std::string& tok) -> void
  {
    if (auto p = s.find(tok); p != std::string::npos)
      s.erase(p);
  };

  cut("std::__cxx11::");
  cut("std::");
  cut("core::");
  cut("inline ");
  cut("static ");

  if (auto p = s.find('<'); p != std::string::npos)
    s.erase(p);

  return s;
}

struct StackFrame
{
  std::string function;
  std::string file;
  int         line = 0;

  [[nodiscard]] auto to_string(size_t depth) const -> std::string
  {
    std::ostringstream oss;
    oss << "  ";
    for (size_t i = 0; i < depth; ++i)
      oss << "│ ";

    oss << cyan("→ ") << cyan(function) << "\n";
    oss << "  ";
    for (size_t i = 0; i < depth; ++i)
      oss << "│ ";

    oss << dim("  at ") << dim(file) << yellow(":" + std::to_string(line)) << "\n";
    return oss.str();
  }
};

struct StackTrace
{
  std::vector<StackFrame> frames;

  [[nodiscard]] auto to_string() const -> std::string
  {
    std::ostringstream oss;
    for (size_t i = 0; i < frames.size(); ++i)
      oss << frames[i].to_string(i);
    return oss.str();
  }
};

#ifdef INLIMBO_DEBUG_BUILD

#include "utils/timer/Timer.hpp"
#include <backtrace-supported.h>
#include <backtrace.h>
#include <cstdlib>
#include <cxxabi.h>
#include <iostream>
#include <mutex>
#include <utility>

class BacktraceCollector
{
public:
  static auto capture(int skip = 0) -> StackTrace
  {
    StackTrace              trace;
    static backtrace_state* state = backtrace_create_state(nullptr, 0, error_cb, nullptr);

    if (!state)
      return trace;

    backtrace_full(state, skip + 1, frame_cb, error_cb, &trace);
    return trace;
  }

private:
  static void error_cb(void*, const char*, int) {}

  static auto frame_cb(void* data, uintptr_t, const char* file, int line, const char* func) -> int
  {
    auto*      trace = static_cast<StackTrace*>(data);
    StackFrame f;
    f.file     = file ? file : "<unknown>";
    f.line     = line;
    f.function = simplify_symbol(demangle(func));
    trace->frames.emplace_back(std::move(f));
    return 0;
  }

  static auto demangle(const char* name) -> std::string
  {
    if (!name)
      return "<unknown>";
    int         st  = 0;
    char*       out = abi::__cxa_demangle(name, nullptr, nullptr, &st);
    std::string r   = (st == 0 && out) ? out : name;
    std::free(out);
    return r;
  }
};

template <typename Payload = std::string> struct Event
{
  std::string ts;
  std::string cat;
  Payload     data;
  StackTrace  trace;

  Event(std::string c, Payload p)
      : ts(util::timestamp_now()), cat(std::move(c)), data(std::move(p)),
        trace(BacktraceCollector::capture(3))
  {
  }

  [[nodiscard]] auto summary(size_t index) const -> std::string
  {
    constexpr size_t width = 60;

    auto pad = [](std::string s) -> std::string
    {
      if (s.size() < width - 2)
        s += std::string(width - 2 - s.size(), ' ');
      return s;
    };

    std::ostringstream oss;

    std::string line1 = bold(ts) + "  " + cat;
    std::string line2 = data;

    oss << "┌" << std::string(width - 2, '-') << "┐\n";
    oss << "│ " << "\n";
    oss << "│ " << "#" << index << " " << pad(line1) << "\n";
    oss << "│ " << pad(line2) << "\n";
    oss << "│ " << "\n";
    oss << "└" << std::string(width - 2, '-') << "┘\n";

    oss << trace.to_string();
    return oss.str();
  }
};

template <typename T> using Events = std::vector<Event<T>>;

template <typename Payload = std::string> class EventLog
{
public:
  static auto instance() -> EventLog&
  {
    static EventLog inst;
    return inst;
  }

  void record(const std::string& cat, const Payload& p)
  {
    std::lock_guard<std::mutex> l(m_mutex);
    m_payloadEvents.emplace_back(cat, p);
  }

  void dump_to_stdout() const
  {
    std::lock_guard<std::mutex> l(m_mutex);
    size_t                      index = 0;
    for (const auto& e : m_payloadEvents)
    {
      std::cout << "\n" << e.summary(index++);
    }
  }

private:
  mutable std::mutex m_mutex;
  Events<Payload>    m_payloadEvents;
};

class TraceScope
{
public:
  TraceScope(std::string cat, std::string fn) : m_cat(std::move(cat)), m_func(simplify_symbol(fn))
  {
    EventLog<std::string>::instance().record(m_cat, green("enter ") + m_func);
  }

  ~TraceScope() { EventLog<std::string>::instance().record(m_cat, red("exit  ") + m_func); }

private:
  std::string m_cat;
  std::string m_func;
};

#else

class TraceScope
{
public:
  TraceScope(...) {}
};

#endif

} // namespace core

#if defined(__GNUC__) || defined(__clang__)
#define RECORD_FUNC_TO_BACKTRACE(cat) core::TraceScope trace_scope(cat, __PRETTY_FUNCTION__)
#elif defined(_MSC_VER)
#define RECORD_FUNC_TO_BACKTRACE(cat) core::TraceScope trace_scope(cat, __FUNCSIG__)
#else
#define RECORD_FUNC_TO_BACKTRACE(cat) core::TraceScope trace_scope(cat, __FUNCTION__)
#endif

#ifdef INLIMBO_DEBUG_BUILD
#define DUMP_TRACE()                                                                           \
  do                                                                                           \
  {                                                                                            \
    if (core::is_stack_trace_enabled())                                                        \
      core::EventLog<std::string>::instance().dump_to_stdout();                                \
    else                                                                                       \
      std::cout << "[trace disabled] set " << INLIMBO_STACK_TRACE_DUMP_ENV << "=1 to dump.\n"; \
  } while (0)
#else
#define DUMP_TRACE()                                                      \
  do                                                                      \
  {                                                                       \
    std::cout << "[trace disabled] build without INLIMBO_DEBUG_BUILD.\n"; \
  } while (0)
#endif
