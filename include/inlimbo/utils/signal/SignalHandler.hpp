#pragma once

#include <csignal>
#include <cstdlib>
#include <execinfo.h>
#include "Logger.hpp"

namespace utils {

class SignalHandler {
public:
    SignalHandler(const SignalHandler&) = delete;
    auto operator=(const SignalHandler&) -> SignalHandler& = delete;

    static auto getInstance() -> SignalHandler& {
        static SignalHandler instance;
        return instance;
    }

    void setup() {
        struct sigaction sa {};
        sa.sa_sigaction = SignalHandler::handleSignal;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = SA_SIGINFO;

        sigaction(SIGABRT, &sa, nullptr);
        sigaction(SIGSEGV, &sa, nullptr);
    }

private:
    SignalHandler() = default;
    ~SignalHandler() = default;

    static void handleSignal(int signal, siginfo_t* info, void* /*context*/) {
        const char* signalName = "Unknown";
        switch (signal) {
            case SIGABRT: signalName = "SIGABRT"; break;
            case SIGSEGV: signalName = "SIGSEGV"; break;
        }

        LOG_CRITICAL("Fatal signal caught: {} ({})", signalName, signal);
        if (info && info->si_addr)
            LOG_CRITICAL("Fault address: {}", fmt::ptr(info->si_addr));

        // Generate backtrace inline (for console + file if logger supports it)
        LOG_CRITICAL("Printing Backtrace...");
        DUMP_TRACE;
        LOG_CRITICAL("Backtrace print ended.");

        LOG_CRITICAL("Application terminated due to fatal signal.");

        _Exit(EXIT_FAILURE);
    }
};

}
