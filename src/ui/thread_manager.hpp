/**
 * @file ThreadManager.hpp
 * @brief Declaration of the ThreadManager class for managing threads and their states.
 */

#ifndef THREAD_MANAGER_HPP
#define THREAD_MANAGER_HPP

#include <atomic>
#include <future>
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
    /**
     * @brief A mutex to synchronize access to the play state.
     */
    std::mutex play_mutex;

    /**
     * @brief Indicates whether the thread is currently processing.
     */
    std::atomic<bool> is_processing{false};

    /**
     * @brief Indicates whether the thread is currently playing.
     */
    std::atomic<bool> is_playing{false};

    /**
     * @brief A future object associated with the play operation.
     */
    std::future<void> play_future;
  };

  /**
   * @brief Retrieves the thread state managed by this ThreadManager.
   * @return A reference to the thread state.
   */
  ThreadState& getThreadState() { return thread_state; }

  /**
   * @brief Locks the play mutex for the provided thread state.
   *
   * This function uses a unique lock to ensure that the play mutex
   * is safely locked while performing operations on the thread state.
   *
   * @param thread_state A reference to the ThreadState object whose mutex is to be locked.
   */
  void lockPlayMutex(ThreadState& thread_state)
  {
    std::unique_lock<std::mutex> lock(thread_state.play_mutex);
  }
  /**
   * @brief Unlocks the play mutex for the provided thread state.
   *
   * This function releases the lock on the play mutex that was previously acquired
   * using `lockPlayMutex`. The play mutex will be unlocked after this function is called.
   *
   * @param thread_state A reference to the ThreadState object whose mutex is to be unlocked.
   * @note This function should be used in conjunction with `lockPlayMutex` to ensure proper
   * synchronization.
   */
  void unlockPlayMutex(ThreadState& thread_state) { thread_state.play_mutex.unlock(); }

private:
  /**
   * @brief The thread state managed by this instance of ThreadManager.
   */
  ThreadState thread_state;
};

#endif // THREAD_MANAGER_HPP
