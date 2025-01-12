/**
 * @file ThreadManager.hpp
 * @brief Declaration of the ThreadManager class for managing threads and their states.
 */

#ifndef THREAD_MANAGER_HPP
#define THREAD_MANAGER_HPP

#include <future>
#include <thread>
#include <mutex>
#include <atomic>

/**
 * @class ThreadManager
 * @brief A class that manages thread states and provides utilities for thread handling.
 */
class ThreadManager {
public:
    /**
     * @struct ThreadState
     * @brief Represents the state of a thread managed by ThreadManager.
     */
    struct ThreadState {
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
    void lockPlayMutex(ThreadState& thread_state) { std::unique_lock<std::mutex> lock(thread_state.play_mutex); }

private:
    /**
     * @brief The thread state managed by this instance of ThreadManager.
     */
    ThreadState thread_state;
};

#endif // THREAD_MANAGER_HPP
