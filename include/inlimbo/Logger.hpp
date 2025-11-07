#pragma once

#include "utils/Env-Vars.hpp"
#include <memory>
#include <string>
#include <vector>
#include <cstdlib>
#include <sstream>
#include <algorithm>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

#ifdef PLATFORM_WINDOWS
    #include <windows.h>
#else
    #include <unistd.h>
    #include <sys/utsname.h>
#endif

// singleton logging class that can be used anywhere
//
// note that log level, log file path and log file pattern can all be set via ENVIRONMENT_VARIABLES!

#define __INLIMBO_DEFAULT_LOG_FILE__    "inlimbo/logs/core.log"
#define __INLIMBO_DEFAULT_LOG_PATTERN__ "[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%n] %v"

namespace core {

enum class LogMode {
    ConsoleOnly,
    FileOnly,
    ConsoleAndFile
};

inline auto parse_log_level(const std::string& level_str) -> spdlog::level::level_enum {
    std::string s = level_str;
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    if (s == "trace") return spdlog::level::trace;
    if (s == "debug") return spdlog::level::debug;
    if (s == "info") return spdlog::level::info;
    if (s == "warn" || s == "warning") return spdlog::level::warn;
    if (s == "error") return spdlog::level::err;
    if (s == "critical" || s == "fatal") return spdlog::level::critical;
    return spdlog::level::info; // default fallback
}

template <typename T = void>
class Logger {
public:
    // Access singleton logger, lazy-init
    static auto get() -> std::shared_ptr<spdlog::logger>& {
        static std::once_flag init_flag;
        std::call_once(init_flag, []() {
            init_from_env(); // automatically setup from env vars
        });
        return get_instance();
    }

    // Optional manual init if desired
    static void init(const std::string& name = "core",
                     LogMode mode = LogMode::ConsoleAndFile,
                     const std::string& file = __INLIMBO_DEFAULT_LOG_FILE__,
                     spdlog::level::level_enum level = spdlog::level::trace,
                     const std::string& pattern = __INLIMBO_DEFAULT_LOG_PATTERN__)
    {
        auto& instance = get_instance();
        if (instance)
            return;

        std::vector<spdlog::sink_ptr> sinks;

        if (mode == LogMode::ConsoleOnly || mode == LogMode::ConsoleAndFile) {
            auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            console_sink->set_pattern(pattern);
            sinks.push_back(console_sink);
        }

        if (mode == LogMode::FileOnly || mode == LogMode::ConsoleAndFile) {
            auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(file, true);
            file_sink->set_pattern(pattern);
            sinks.push_back(file_sink);
        }

        auto logger = std::make_shared<spdlog::logger>(name, sinks.begin(), sinks.end());
        spdlog::register_logger(logger);
        logger->set_level(level);
        logger->flush_on(spdlog::level::trace);

        instance = logger;

        print_banner(file, level, pattern);
    }

    static void set_level(spdlog::level::level_enum level) {
        if (auto& inst = get_instance()) {
            inst->set_level(level);
        }
    }

    static auto is_initialized() noexcept -> bool { return static_cast<bool>(get_instance()); }

    static void shutdown() noexcept {
        if (get_instance()) {
            spdlog::drop_all();
            get_instance().reset();
        }
    }

private:
    // Initialize using environment variables
    static void init_from_env() {
        const char* env_file = std::getenv(INLIMBO_LOG_FILE_ENV);
        const char* env_level = std::getenv(INLIMBO_LOG_LEVEL_ENV);
        const char* env_pattern = std::getenv(INLIMBO_LOG_PATTERN_ENV);

        const std::string file = env_file ? env_file : "logs/core.log";
        const std::string pattern = env_pattern ? env_pattern :
            "[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%n] %v";
        const spdlog::level::level_enum level = env_level
            ? parse_log_level(env_level)
            : spdlog::level::info;

        init("core", LogMode::ConsoleAndFile, file, level, pattern);
    }

    static auto get_instance() -> std::shared_ptr<spdlog::logger>& {
        static std::shared_ptr<spdlog::logger> instance = nullptr;
        return instance;
    }

    static void print_banner(const std::string& file,
                             spdlog::level::level_enum level,
                             const std::string& pattern)
    {
        auto& log = get_instance();
        if (!log)
            return;

#if defined(PLATFORM_WINDOWS)
        SYSTEM_INFO sysinfo;
        GetSystemInfo(&sysinfo);
        DWORD pid = GetCurrentProcessId();
        std::string sysname = "Windows";
        std::string arch = std::to_string(sysinfo.dwProcessorType);
#else
        struct utsname uts{};
        uname(&uts);
        pid_t pid = getpid();
        std::string sysname = uts.sysname;
        std::string arch = uts.machine;
#endif

        const char* level_name = spdlog::level::to_short_c_str(level);
        std::ostringstream tid_ss;
        tid_ss << std::this_thread::get_id();

        log->info("┌────────────────────────────── InLimbo Logger Initialized ──────────────────────────────┐");
        log->info("│  Log Level   : {}", level_name);
        log->info("│  Pattern     : {}", pattern);
        log->info("│  Output File : {}", file);
        log->info("│  System      : {} ({})", sysname, arch);
        log->info("│  PID         : {}", pid);
        log->info("│  Thread ID   : {}", tid_ss.str());
        log->info("└────────────────────────────────────────────────────────────────────────────────────────┘");
    }
};

// ----------------------------------------
// Global Logging Macros
// ----------------------------------------
#define LOG_TRACE(...)    ::core::Logger<void>::get()->trace(__VA_ARGS__)
#define LOG_DEBUG(...)    ::core::Logger<void>::get()->debug(__VA_ARGS__)
#define LOG_INFO(...)     ::core::Logger<void>::get()->info(__VA_ARGS__)
#define LOG_WARN(...)     ::core::Logger<void>::get()->warn(__VA_ARGS__)
#define LOG_ERROR(...)    ::core::Logger<void>::get()->error(__VA_ARGS__)
#define LOG_CRITICAL(...) ::core::Logger<void>::get()->critical(__VA_ARGS__)

} // namespace core
