/**
 * @file ThreadManager.hpp
 * @brief Declaration of the ThreadManager class for managing threads and their states.
 */

#pragma once

#include "workers.hpp"
#include <atomic>
#include <future>
#include <memory>
#include <mutex>
#include <thread>

/**
 * @class ThreadManager
 * @brief A class that manages thread states and provides utilities for thread handling.
 */
class ThreadManager
{
public:
  /**
   * @struct ThreadState
   * @brief Represents the state of a thread managed by ThreadManager.
   */
  struct ThreadState
  {
    std::mutex play_mutex;        ///< Mutex for synchronizing play state.
    std::mutex queue_mutex;       ///< Mutex for synchronizing queue operations.
    std::mutex audioDevicesMutex; ///< Mutex for synchronizing audio device access.

    std::atomic<bool> is_processing{false}; ///< Indicates whether the thread is processing.
    std::atomic<bool> is_playing{false};    ///< Indicates whether the thread is playing.

    std::future<void>            play_future;       ///< Future object for async play operations.
    std::unique_ptr<std::thread> mpris_dbus_thread; ///< Unique pointer for DBus thread.
    std::thread playNextSongThread; ///< Thread for handling "play next song" operations.
  };

  /**
   * @brief Construct a ThreadManager with a worker thread pool.
   */
  ThreadManager() : worker_pool(std::make_shared<WorkerThreadPool>()) {}

  /**
   * @brief Destructor to clean up resources and threads.
   */
  /*~ThreadManager() { cleanupAllThreads(); }*/

  /**
   * @brief Retrieves the thread state managed by this ThreadManager.
   * @return A reference to the thread state.
   */
  auto getThreadState() -> ThreadState& { return thread_state; }

  /**
   * @brief Get the worker thread pool.
   * @return Reference to the WorkerThreadPool.
   */
  auto getWorkerThreadPool() -> WorkerThreadPool& { return *worker_pool; }

  /**
   * @brief Locks the play mutex for the provided thread state.
   * @param thread_state Reference to the ThreadState object whose mutex is to be locked.
   */
  void lockPlayMutex(ThreadState& thread_state)
  {
    std::lock_guard<std::mutex> lock(thread_state.play_mutex);
  }

  /**
   * @brief Locks the queue mutex for the provided thread state.
   * @param thread_state Reference to the ThreadState object whose mutex is to be locked.
   */
  void lockQueueMutex(ThreadState& thread_state)
  {
    std::lock_guard<std::mutex> lock(thread_state.queue_mutex);
  }

private:
  /**
   * @brief Cleans up all threads managed by the ThreadManager.
   */
  void cleanupAllThreads()
  {
    try
    {
      // Clean up playNextSongThread if joinable.
      if (thread_state.playNextSongThread.joinable())
      {
        thread_state.playNextSongThread.join();
      }
    }
    catch (const std::exception& e)
    {
      // Log exceptions during cleanup (adjust logging mechanism as needed).
      std::cerr << "Exception during thread cleanup: " << e.what() << std::endl;
    }
    catch (...)
    {
      std::cerr << "Unknown exception during thread cleanup." << std::endl;
    }
  }

  /**
   * @brief The thread state managed by this instance of ThreadManager.
   */
  ThreadState thread_state;

  /**
   * @brief The worker thread pool.
   */
  std::shared_ptr<WorkerThreadPool> worker_pool;
};
