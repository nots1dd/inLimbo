#pragma once

#include "utils/string/SmallString.hpp"
// Include fmt support for SmallString
// NOLINTNEXTLINE(build/include)
#include "utils/string/SmallStringFmt.hpp"
#include <memory>
#include <spdlog/spdlog.h>

namespace inlimbo
{

constexpr std::size_t kMaxLogFiles = 100;

enum class LogMode
{
  ConsoleOnly,
  FileOnly,
  ConsoleAndFile
};

auto parse_log_level(const utils::string::SmallString& level_str) -> spdlog::level::level_enum;
auto make_session_log_file() -> utils::string::SmallString;
auto cleanup_old_logs() -> void;

class Logger
{
public:
  static auto get() -> std::shared_ptr<spdlog::logger>&;

  static void init(const utils::string::SmallString& name    = "core",
                   LogMode                           mode    = LogMode::ConsoleAndFile,
                   const utils::string::SmallString& file    = "",
                   spdlog::level::level_enum         level   = spdlog::level::trace,
                   const utils::string::SmallString& pattern = "");

  static void set_level(spdlog::level::level_enum level);

  static auto is_initialized() noexcept -> bool;

  static void shutdown() noexcept;

private:
  static void init_from_env();
  static auto get_instance() -> std::shared_ptr<spdlog::logger>&;
  static void print_banner(const utils::string::SmallString& file, spdlog::level::level_enum level,
                           const utils::string::SmallString& pattern);
};

} // namespace inlimbo

#define LOG_TRACE(...)    ::inlimbo::Logger::get()->trace(__VA_ARGS__)
#define LOG_DEBUG(...)    ::inlimbo::Logger::get()->debug(__VA_ARGS__)
#define LOG_INFO(...)     ::inlimbo::Logger::get()->info(__VA_ARGS__)
#define LOG_WARN(...)     ::inlimbo::Logger::get()->warn(__VA_ARGS__)
#define LOG_ERROR(...)    ::inlimbo::Logger::get()->error(__VA_ARGS__)
#define LOG_CRITICAL(...) ::inlimbo::Logger::get()->critical(__VA_ARGS__)
