#ifndef SIGNAL_HANDLER_HPP
#define SIGNAL_HANDLER_HPP

#include <iostream>
#include <csignal>
#include <execinfo.h>
#include <cstdlib>
#include <unistd.h>
#include <string>

class SignalHandler {
public:
    // Singleton instance getter
    static SignalHandler& getInstance() {
        static SignalHandler instance;
        return instance;
    }

    // Initialize the signal handler
    void setup() {
        struct sigaction sa;
        sa.sa_sigaction = SignalHandler::handleSignal; // Use static member function
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = SA_SIGINFO; // Use siginfo_t for extended info

        sigaction(SIGABRT, &sa, nullptr); // Catch SIGABRT
        sigaction(SIGSEGV, &sa, nullptr); // Catch SIGSEGV
    }

private:
    SignalHandler() = default; // Private constructor for singleton
    ~SignalHandler() = default;

    // Non-copyable and non-movable
    SignalHandler(const SignalHandler&) = delete;
    SignalHandler& operator=(const SignalHandler&) = delete;

    // Signal handling function
    static void handleSignal(int signal, siginfo_t* info, void* context) {
        const char* signalName = nullptr;
        switch (signal) {
            case SIGABRT: signalName = "SIGABRT"; break;
            case SIGSEGV: signalName = "SIGSEGV"; break;
            default: signalName = "Unknown"; break;
        }

        std::cerr << "\n=== Signal Caught ===\n";
        std::cerr << "Signal: " << signalName << " (" << signal << ")\n";
        if (info) {
            std::cerr << "Address causing signal: " << info->si_addr << "\n";
        }

        // Generate a backtrace
        void* buffer[128];
        int size = backtrace(buffer, 128);
        std::cerr << "Backtrace (" << size << " frames):\n";
        char** symbols = backtrace_symbols(buffer, size);
        for (int i = 0; i < size; ++i) {
            std::cerr << symbols[i] << '\n';
        }
        free(symbols);

        // Clean termination
        _Exit(EXIT_FAILURE);
    }
};

#endif
