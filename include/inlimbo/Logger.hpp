#pragma once

#include <memory>
#include <spdlog/spdlog.h>
#include <string>

namespace inlimbo
{

enum class LogMode
{
  ConsoleOnly,
  FileOnly,
  ConsoleAndFile
};

auto parse_log_level(const std::string& level_str) -> spdlog::level::level_enum;

class Logger
{
public:
  static auto get() -> std::shared_ptr<spdlog::logger>&;

  static void init(const std::string& name = "core", LogMode mode = LogMode::ConsoleAndFile,
                   const std::string&        file    = "",
                   spdlog::level::level_enum level   = spdlog::level::trace,
                   const std::string&        pattern = "");

  static void set_level(spdlog::level::level_enum level);

  static auto is_initialized() noexcept -> bool;

  static void shutdown() noexcept;

private:
  static void init_from_env();
  static auto get_instance() -> std::shared_ptr<spdlog::logger>&;
  static void print_banner(const std::string& file, spdlog::level::level_enum level,
                           const std::string& pattern);
};

} // namespace inlimbo

#define LOG_TRACE(...)    ::inlimbo::Logger::get()->trace(__VA_ARGS__)
#define LOG_DEBUG(...)    ::inlimbo::Logger::get()->debug(__VA_ARGS__)
#define LOG_INFO(...)     ::inlimbo::Logger::get()->info(__VA_ARGS__)
#define LOG_WARN(...)     ::inlimbo::Logger::get()->warn(__VA_ARGS__)
#define LOG_ERROR(...)    ::inlimbo::Logger::get()->error(__VA_ARGS__)
#define LOG_CRITICAL(...) ::inlimbo::Logger::get()->critical(__VA_ARGS__)
