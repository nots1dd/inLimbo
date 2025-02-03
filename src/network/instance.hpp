/**
 * @file instance.hpp
 * @brief Implements a Unix domain socket lock to ensure only one instance of inLimbo runs at a
 * time.
 *
 * This class creates and binds to a Unix domain socket, preventing multiple instances
 * of inLimbo from running simultaneously. It also handles cleanup of the socket file
 * upon normal exit or termination.
 */
#pragma once

#include <csignal>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#define SOCKET_PATH "/tmp/inLimbo.sock"

/**
 * @class SingleInstanceLock
 * @brief Ensures a single instance of the application runs using a Unix domain socket lock.
 */
class SingleInstanceLock
{
private:
  int                server_fd; ///< File descriptor for the Unix domain socket.
  struct sockaddr_un addr;      ///< Structure to store socket address information.
  bool               locked;    ///< Indicates whether the instance is successfully locked.

public:
  /**
   * @brief Constructs a SingleInstanceLock object.
   *
   * Creates a Unix domain socket and binds it to ensure only one instance is running.
   * Registers cleanup functions for graceful termination.
   */
  SingleInstanceLock() : server_fd(-1), locked(false)
  {
    std::cout << "-- Using Unix domain socket at: " << SOCKET_PATH << "\n";

    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd == -1)
    {
      std::cerr << "**Error:** Failed to create socket: " << strerror(errno) << "\n";
      return;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1)
    {
      std::cerr << "**Another instance of inLimbo is already running!**\n";
      std::cerr << "-- Socket path: " << SOCKET_PATH << "\n";
      std::cerr << "-- Error: " << strerror(errno) << "\n";
      close(server_fd);
      return;
    }

    chmod(SOCKET_PATH, 0600); // Only owner can read/write

    locked = true;

    // Try to force cleanup the socket
    std::atexit(cleanup);
    std::signal(SIGTERM, signalHandler);
    std::signal(SIGINT, signalHandler);
    std::signal(SIGHUP, signalHandler);
  }
  /**
   * @brief Checks if the instance lock is active.
   * @return True if the socket was successfully bound, false otherwise.
   */
  bool isLocked() const { return locked; }

  /**
   * @brief Prints information about the Unix domain socket.
   *
   * Displays details such as socket path, owner UID, and file permissions.
   */
  void printSocketInfo()
  {
    struct stat socket_stat;
    if (stat(SOCKET_PATH, &socket_stat) == 0)
    {
      std::cout << "-- Socket Details:\n";
      std::cout << "   - Path: " << SOCKET_PATH << "\n";
      std::cout << "   - Owner UID: " << socket_stat.st_uid << "\n";
      std::cout << "   - Permissions: " << std::oct << (socket_stat.st_mode & 0777) << std::dec
                << "\n";
    }
    else
    {
      std::cerr << "**Warning:** Unable to retrieve socket info: " << strerror(errno) << "\n";
    }
  }
  /**
   * @brief Cleans up the Unix domain socket file.
   *
   * This function ensures that any stale socket file is removed to prevent blocking
   * subsequent launches of inLimbo.
   */
  static void cleanup()
  {
    if (unlink(SOCKET_PATH) == 0)
    {
      std::cout << "-- Socket cleaned: " << SOCKET_PATH << "\n";
    }
  }
  /**
   * @brief Signal handler for cleaning up the socket on termination signals.
   * @param signal The received signal number.
   *
   * Cleans up the socket and exits with the received signal code.
   */
  static void signalHandler(int signal)
  {
    cleanup();
    std::exit(signal);
  }

  /**
   * @brief Destructor for SingleInstanceLock.
   *
   * Cleans up the socket and closes the file descriptor if the lock was acquired.
   */
  ~SingleInstanceLock()
  {
    if (locked)
    {
      std::cout << "-- Cleaning up inLimbo socket...\n";
      close(server_fd);
      unlink(SOCKET_PATH);
    }
  }
};
