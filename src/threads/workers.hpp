#pragma once

#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

/**
 * @brief A thread pool that manages a collection of worker threads.
 *
 * This class allows the management of a pool of threads to execute tasks concurrently.
 * The pool will execute submitted tasks asynchronously using worker threads. The tasks
 * are queued and workers are assigned to execute them. Once a task is completed, the
 * worker is available for another task. The pool supports graceful shutdown and ensures
 * no task is left unfinished when the pool is stopped.
 */
class WorkerThreadPool
{
public:
  /**
   * @brief Constructs a worker thread pool with a specified number of threads.
   *
   * Initializes the pool with a given number of threads, which defaults to the number
   * of hardware threads available on the system. Each worker thread will continuously
   * wait for tasks and execute them as they are added to the queue.
   *
   * @param thread_count The number of threads to initialize. Defaults to the number
   * of hardware threads.
   */
  explicit WorkerThreadPool(size_t thread_count = std::thread::hardware_concurrency()) : stop(false)
  {
    for (size_t i = 0; i < thread_count; ++i)
    {
      workers.emplace_back(
        [this]
        {
          for (;;)
          {
            std::function<void()> task;
            {
              std::unique_lock<std::mutex> lock(queue_mutex);
              condition.wait(lock, [this] { return stop || !tasks.empty(); });

              if (stop && tasks.empty())
                return;

              task = std::move(tasks.front());
              tasks.pop();
            }
            task();
          }
        });
    }
  }

  /**
   * @brief Enqueues a task to be executed by the worker threads.
   *
   * This method allows submitting a task to the pool. The task will be executed by one of the
   * available worker threads. The function returns a std::future that can be used to retrieve
   * the result of the task once it is completed.
   *
   * @tparam F The type of the callable (function, lambda, etc.) to be executed.
   * @tparam Args The types of arguments that the callable accepts.
   * @param f The function or callable to be executed.
   * @param args The arguments to pass to the callable.
   * @return std::future<typename std::result_of<F(Args...)>::type> A future representing the result
   * of the task once it finishes executing.
   *
   * @throws std::runtime_error If the thread pool has been stopped and no more tasks can be
   * enqueued.
   */
  template <class F, class... Args>
  auto enqueue(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type>
  {
    using return_type = typename std::result_of<F(Args...)>::type;

    auto task = std::make_shared<std::packaged_task<return_type()>>(
      std::bind(std::forward<F>(f), std::forward<Args>(args)...));

    std::future<return_type> result = task->get_future();
    {
      std::unique_lock<std::mutex> lock(queue_mutex);

      // Prevent enqueueing after stopping the pool
      if (stop)
      {
        throw std::runtime_error("Enqueue on stopped ThreadPool");
      }

      tasks.emplace([task]() { (*task)(); });
    }
    condition.notify_one();
    return result;
  }

  /**
   * @brief Gracefully shuts down the thread pool.
   *
   * This method will stop accepting new tasks and wait for all worker threads to finish
   * their current tasks before destroying the pool. Once all threads have finished,
   * the destructor will join all threads, ensuring they are properly cleaned up.
   */
  ~WorkerThreadPool()
  {
    {
      std::unique_lock<std::mutex> lock(queue_mutex);
      stop = true;
    }
    condition.notify_all();

    // Wait for all threads to complete
    for (std::thread& worker : workers)
    {
      if (worker.joinable())
      {
        worker.join();
      }
    }
  }

  // Prevent copying and assignment
  WorkerThreadPool(const WorkerThreadPool&)                    = delete;
  auto operator=(const WorkerThreadPool&) -> WorkerThreadPool& = delete;

private:
  std::vector<std::thread>          workers; /**< Vector holding worker threads */
  std::queue<std::function<void()>> tasks;   /**< Queue of tasks to be executed */

  std::mutex              queue_mutex; /**< Mutex for synchronizing access to the task queue */
  std::condition_variable condition;   /**< Condition variable to notify workers of new tasks */
  std::atomic<bool>       stop;        /**< Flag to indicate if the pool should stop */
};
