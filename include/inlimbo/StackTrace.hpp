#pragma once

#include <string>
#include <utility>
#include <vector>
#include <mutex>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <backtrace.h>
#include <backtrace-supported.h>
#include <cxxabi.h>

namespace core {

struct StackFrame {
    std::string function;
    std::string filename;
    int lineno = 0;

    [[nodiscard]] auto to_string() const -> std::string {
        std::ostringstream oss;
        oss << "↳ " << function
            << "  [" << filename << ":" << lineno << "]";
        return oss.str();
    }
};

struct StackTrace {
    std::vector<StackFrame> frames;

    [[nodiscard]] auto to_string() const -> std::string {
        std::ostringstream oss;
        for (size_t i = 0; i < frames.size(); ++i)
            oss << "    #" << std::setw(2) << i << "  "
                << frames[i].to_string() << "\n";
        return oss.str();
    }
};

class BacktraceCollector {
public:
    static auto capture(int skip = 0, int max_frames = 64) -> StackTrace {
        StackTrace trace;
        static backtrace_state* state =
            backtrace_create_state(nullptr, 0, error_callback, nullptr);

        if (!state) {
            trace.frames.push_back({"<no backtrace>", "<unknown>", 0});
            return trace;
        }

        backtrace_full(state, skip + 1, full_callback, error_callback, &trace);

        if ((int)trace.frames.size() > max_frames)
            trace.frames.resize(max_frames);

        return trace;
    }

private:
    static void error_callback(void*, const char* msg, int errnum) {
        std::cerr << "libbacktrace error: "
                  << (msg ? msg : "(null)") << " code=" << errnum << "\n";
    }

    static auto full_callback(void* data, uintptr_t /*pc*/,
                              const char* filename, int lineno,
                              const char* function) -> int {
        auto* trace = static_cast<StackTrace*>(data);
        StackFrame f;
        f.filename = filename ? filename : "<unknown>";
        f.function = demangle(function);
        f.lineno = lineno;
        trace->frames.push_back(std::move(f));
        return 0;
    }

    static auto demangle(const char* name) -> std::string {
        if (!name)
            return "<unknown>";
        int status = 0;
        size_t len = 0;
        char* demangled = abi::__cxa_demangle(name, nullptr, &len, &status);
        std::string result =
            (status == 0 && demangled) ? demangled : name;
        free(demangled);
        return result;
    }
};

// ---------------------------------------------------------------------------
// Time utility
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

// ---------------------------------------------------------------------------
// Event type
// ---------------------------------------------------------------------------
template <typename Payload = std::string>
struct Event {
    std::string timestamp;
    std::string category;
    Payload data;
    StackTrace trace;

    Event(std::string cat, const Payload& payload)
        : timestamp(timestamp_now()),
          category(std::move(cat)),
          data(payload),
          trace(BacktraceCollector::capture(2)) {}

    [[nodiscard]] auto summary() const -> std::string {
        std::ostringstream oss;
        oss << "┌───────────────────────────────────────────────┐\n"
            << "│ [" << timestamp << "] " << category << "\n"
            << "│ " << data << "\n"
            << "└───────────────────────────────────────────────┘\n";
        oss << "    Stack Trace:\n" << trace.to_string();
        return oss.str();
    }
};

// ---------------------------------------------------------------------------
// Event log
// ---------------------------------------------------------------------------
template <typename Payload = std::string>
class EventLog {
public:
    using event_type = Event<Payload>;

    static auto instance() -> EventLog& {
        static EventLog<Payload> inst;
        return inst;
    }

    void record(const std::string& category, const Payload& payload) {
        std::lock_guard<std::mutex> lock(mutex_);
        events_.emplace_back(category, payload);
    }

    void clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        events_.clear();
    }

    auto get_all() const -> std::vector<event_type> {
        std::lock_guard<std::mutex> lock(mutex_);
        return events_;
    }

    void dump_to_stdout() const {
        std::lock_guard<std::mutex> lock(mutex_);
        for (size_t i = 0; i < events_.size(); ++i) {
            const auto& ev = events_[i];
            std::cout << "\nEvent #" << i + 1 << "\n";
            std::cout << ev.summary();
        }
    }

private:
    EventLog() = default;
    mutable std::mutex mutex_;
    std::vector<event_type> events_;
};

class TraceScope {
public:
    explicit TraceScope(std::string category,
                        std::string details = "")
        : category_(std::move(category)),
          details_(demangle_pretty(details)) {
        EventLog<std::string>::instance().record(category_,
                                                 "Entered " + details_);
    }

    ~TraceScope() {
        EventLog<std::string>::instance().record(category_,
                                                 "Exited " + details_);
    }

private:
    std::string category_;
    std::string details_;

    static auto demangle_pretty(const std::string& raw) -> std::string {
#if defined(__GNUC__) || defined(__clang__)
        // __PRETTY_FUNCTION__ already gives a demangled signature,
        // but we can clean it up to remove extra "static", "inline", etc. if needed.
        return raw;
#elif defined(_MSC_VER)
        return raw;  // __FUNCSIG__ is already human-readable
#else
        return raw;
#endif
    }
};

} // namespace core

#if defined(__GNUC__) || defined(__clang__)
#define RECORD_FUNC_TO_BACKTRACE(cat) \
    core::TraceScope trace_scope_instance(cat, __PRETTY_FUNCTION__)
#elif defined(_MSC_VER)
#define RECORD_FUNC_TO_BACKTRACE(cat) \
    core::TraceScope trace_scope_instance(cat, __FUNCSIG__)
#else
#define RECORD_FUNC_TO_BACKTRACE(cat) \
    core::TraceScope trace_scope_instance(cat, __FUNCTION__)
#endif

#define DUMP_TRACE \
    core::EventLog<std::string>::instance().dump_to_stdout();
