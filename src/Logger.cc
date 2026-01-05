#include "Logger.hpp"
#include "utils/Env-Vars.hpp"
#include "utils/PathResolve.hpp"
#include "utils/string/Transforms.hpp"

#include <cstdlib>
#include <sstream>
#include <thread>
#include <vector>

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#ifdef PLATFORM_WINDOWS
#include <windows.h>
#else
#include <sys/utsname.h>
#include <unistd.h>
#endif

#define __INLIMBO_DEFAULT_LOG_FILE__    "logs/core.log"
#define __INLIMBO_DEFAULT_LOG_PATTERN__ "[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v"

namespace inlimbo
{

auto parse_log_level(const utils::string::SmallString& level_str) -> spdlog::level::level_enum
{
  const auto s = utils::string::transform::tolower_ascii(level_str.c_str());

  if (s == "trace")
    return spdlog::level::trace;
  if (s == "debug")
    return spdlog::level::debug;
  if (s == "info")
    return spdlog::level::info;
  if (s == "warn" || s == "warning")
    return spdlog::level::warn;
  if (s == "error")
    return spdlog::level::err;
  if (s == "critical" || s == "fatal")
    return spdlog::level::critical;

  return spdlog::level::info;
}

auto Logger::get() -> std::shared_ptr<spdlog::logger>&
{
  static std::once_flag init_flag;
  std::call_once(init_flag, []() -> void { init_from_env(); });
  return get_instance();
}

void Logger::init(const utils::string::SmallString& name, LogMode mode,
                  const utils::string::SmallString& file, spdlog::level::level_enum level,
                  const utils::string::SmallString& pattern)
{
  auto& instance = get_instance();
  if (instance)
    return;

  std::vector<spdlog::sink_ptr> sinks;

  const utils::string::SmallString final_pattern =
    pattern.empty() ? __INLIMBO_DEFAULT_LOG_PATTERN__ : pattern;

  if (mode == LogMode::ConsoleOnly || mode == LogMode::ConsoleAndFile)
  {
    auto sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    sink->set_pattern(final_pattern.c_str());
    sinks.push_back(sink);
  }

  if (mode == LogMode::FileOnly || mode == LogMode::ConsoleAndFile)
  {
    auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(file.c_str(), true);
    sink->set_pattern(final_pattern.c_str());
    sinks.push_back(sink);
  }

  auto logger = std::make_shared<spdlog::logger>(name.c_str(), sinks.begin(), sinks.end());
  spdlog::register_logger(logger);
  logger->set_level(level);
  logger->flush_on(spdlog::level::trace);

  instance = logger;

  print_banner(file, level, final_pattern);
}

void Logger::set_level(spdlog::level::level_enum level)
{
  if (auto& inst = get_instance())
    inst->set_level(level);
}

auto Logger::is_initialized() noexcept -> bool { return static_cast<bool>(get_instance()); }

void Logger::shutdown() noexcept
{
  if (get_instance())
  {
    spdlog::drop_all();
    get_instance().reset();
  }
}

void Logger::init_from_env()
{
  utils::string::SmallString env_file    = std::getenv(INLIMBO_LOG_FILE_ENV);
  utils::string::SmallString env_level   = std::getenv(INLIMBO_LOG_LEVEL_ENV);
  utils::string::SmallString env_pattern = std::getenv(INLIMBO_LOG_PATTERN_ENV);

  const auto file =
    !env_file.empty() ? env_file : utils::getCachePathWithFile(__INLIMBO_DEFAULT_LOG_FILE__);
  const auto pattern = !env_pattern.empty() ? env_pattern : __INLIMBO_DEFAULT_LOG_PATTERN__;
  const auto level   = !env_level.empty() ? parse_log_level(env_level) : spdlog::level::info;

  init("core", LogMode::ConsoleAndFile, file.c_str(), level, pattern.c_str());
}

auto Logger::get_instance() -> std::shared_ptr<spdlog::logger>&
{
  static std::shared_ptr<spdlog::logger> instance = nullptr;
  return instance;
}

void Logger::print_banner(const utils::string::SmallString& file, spdlog::level::level_enum level,
                          const utils::string::SmallString& pattern)
{
  auto& log = get_instance();
  if (!log)
    return;

#ifdef PLATFORM_WINDOWS
  SYSTEM_INFO sysinfo;
  GetSystemInfo(&sysinfo);
  DWORD       pid     = GetCurrentProcessId();
  std::string sysname = "Windows";
  std::string arch    = std::to_string(sysinfo.dwProcessorType);
#else
  struct utsname uts{};
  uname(&uts);
  pid_t       pid     = getpid();
  std::string sysname = uts.sysname;
  std::string arch    = uts.machine;
#endif

  std::ostringstream tid;
  tid << std::this_thread::get_id();

  log->info(
    "┌────────────────────────────── InLimbo Logger Initialized ──────────────────────────────┐");
  log->info("│  Log Level   : {}", spdlog::level::to_short_c_str(level));
  log->info("│  Pattern     : {}", pattern.c_str());
  log->info("│  Output File : {}", file.c_str());
  log->info("│  System      : {} ({})", sysname, arch);
  log->info("│  PID         : {}", pid);
  log->info("│  Thread ID   : {}", tid.str());
  log->info(
    "└────────────────────────────────────────────────────────────────────────────────────────┘");
}

} // namespace inlimbo
