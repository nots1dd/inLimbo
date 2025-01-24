#ifndef WORKER_THREAD_POOL_HPP
#define WORKER_THREAD_POOL_HPP

#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class WorkerThreadPool
{
public:
  /**
   * @brief Construct a worker thread pool with a specified number of threads
   * @param thread_count Number of worker threads to initialize
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
   * @brief Enqueue a task to be executed by worker threads
   * @tparam F Type of the function/callable
   * @tparam Args Types of arguments
   * @return std::future containing the result of the task
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
   * @brief Gracefully shutdown the thread pool
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

  // Prevent copying
  WorkerThreadPool(const WorkerThreadPool&)            = delete;
  WorkerThreadPool& operator=(const WorkerThreadPool&) = delete;

private:
  std::vector<std::thread>          workers;
  std::queue<std::function<void()>> tasks;

  std::mutex              queue_mutex;
  std::condition_variable condition;
  std::atomic<bool>       stop;
};

#endif // WORKER_THREAD_POOL_HPP
