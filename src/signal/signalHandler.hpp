#ifndef SIGNAL_HANDLER_HPP
#define SIGNAL_HANDLER_HPP

#include <iostream>
#include <csignal>
#include <execinfo.h>
#include <cstdlib>
#include <unistd.h>
#include <fstream>
#include <string>
#include <ctime> // For timestamps
#include <sys/resource.h> // For resource usage
#include <sys/utsname.h> // For system info
#include <pthread.h> // For thread info
#include <unistd.h> // For getpid, getppid
#include <climits> // For PATH_MAX

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

    // Get current timestamp as a string
    static std::string getCurrentTimestamp() {
        std::time_t now = std::time(nullptr);
        char buffer[100];
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
        return std::string(buffer);
    }

    static void createCacheDirectory(const std::string& path) {
      struct stat info;
      if (stat(path.c_str(), &info) != 0 || !(info.st_mode & S_IFDIR)) {
          mkdir(path.c_str(), 0755); // Create directory with rwx permissions
      }
    }

    // Get system and environment details
    static void logSystemInfo(std::ofstream& logFile) {
        // Process and thread information
        logFile << "Process ID: " << getpid() << "\n";
        logFile << "Parent Process ID: " << getppid() << "\n";
        logFile << "Thread ID: " << pthread_self() << "\n";

        // Current working directory
        char cwd[PATH_MAX];
        if (getcwd(cwd, sizeof(cwd))) {
            logFile << "Current Working Directory: " << cwd << "\n";
        }

        // System information
        struct utsname sysInfo;
        if (uname(&sysInfo) == 0) {
            logFile << "System Name: " << sysInfo.sysname << "\n";
            logFile << "Node Name: " << sysInfo.nodename << "\n";
            logFile << "Release: " << sysInfo.release << "\n";
            logFile << "Version: " << sysInfo.version << "\n";
            logFile << "Machine: " << sysInfo.machine << "\n";
        }

        // Resource usage
        struct rusage usage;
        if (getrusage(RUSAGE_SELF, &usage) == 0) {
            logFile << "CPU Time Used (user): " << usage.ru_utime.tv_sec << "s " << usage.ru_utime.tv_usec << "us\n";
            logFile << "CPU Time Used (system): " << usage.ru_stime.tv_sec << "s " << usage.ru_stime.tv_usec << "us\n";
            logFile << "Max Resident Set Size: " << usage.ru_maxrss << " KB\n";
        }
    }

    // Signal handling function
    static void handleSignal(int signal, siginfo_t* info, void* context) {
        const char* signalName = nullptr;
        switch (signal) {
            case SIGABRT: signalName = "SIGABRT"; break;
            case SIGSEGV: signalName = "SIGSEGV"; break;
            default: signalName = "Unknown"; break;
        }

        std::string logDir = std::string(std::getenv("HOME")) + "/.cache/inLimbo/";
        createCacheDirectory(logDir);  // Ensure the directory exists

        std::string logFileName = logDir +"debug-" + std::to_string(signal) + ".log";
        std::ofstream logFile(logFileName, std::ios::out | std::ios::app);

        if (logFile.is_open()) {
            logFile << "=== Signal Caught ===\n";
            logFile << "Timestamp: " << getCurrentTimestamp() << "\n";
            logFile << "Signal: " << signalName << " (" << signal << ")\n";
            if (info) {
                logFile << "Address causing signal: " << info->si_addr << "\n";
            }

            // Log system and environment information
            logSystemInfo(logFile);

            // Generate a backtrace
            void* buffer[128];
            int size = backtrace(buffer, 128);
            logFile << "Backtrace (" << size << " frames):\n";
            char** symbols = backtrace_symbols(buffer, size);
            for (int i = 0; i < size; ++i) {
                logFile << symbols[i] << '\n';
            }
            free(symbols);

            logFile.close();

            std::cerr << "** Critical error occurred. See " << logFileName << " for details.\n" << "Exiting... **" << std::endl;
        } else {
            std::cerr << "-- Failed to write to log file: " << logFileName << "\n";
        }

        // Clean termination
        _Exit(EXIT_FAILURE);
    }
};

#endif
