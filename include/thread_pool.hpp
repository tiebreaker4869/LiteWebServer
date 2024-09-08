#pragma once

#include <mutex>
#include <queue>
#include <thread>
#include <condition_variable>
#include <functional>
#include <memory>
#include <atomic>

/**
 * @class ThreadPool
 * @brief A simple thread pool implementation for managing and executing tasks concurrently.
 *
 * This class provides a mechanism to create a pool of worker threads that can
 * execute tasks asynchronously. It allows adding tasks to a queue, which are then
 * picked up and executed by available threads in the pool.
 */
class ThreadPool
{
public:
    /**
     * @brief Construct a new Thread Pool object
     * @param thread_num The number of threads to create in the pool. If -1, uses the number of hardware threads available.
     */
    explicit ThreadPool(ssize_t thread_num = -1) : pool_(std::make_shared<Pool>())
    {
        if (thread_num <= 0)
        {
            thread_num = std::thread::hardware_concurrency();
        }

        for (ssize_t i = 0; i < thread_num; ++i)
        {
            std::thread([pool = pool_]
                        {
                        std::unique_lock<std::mutex> lock(pool->mtx);
                        while (true)
                        {
                            if (!pool->tasks.empty())
                            {
                                auto task = std::move(pool->tasks.front());
                                pool->tasks.pop();
                                lock.unlock();
                                task();
                                lock.lock();
                            } else if (pool->is_stop) {
                                break;
                            } else {
                                pool->cond.wait(lock);
                            }
                        } })
                .detach();
        }
    }

    /**
     * @brief Destroy the Thread Pool object, stopping all threads and cleaning up resources
     */
    ~ThreadPool()
    {
        if (pool_)
        {
            std::lock_guard<std::mutex> lock{pool_->mtx};
            pool_->is_stop = true;
        }
        pool_->cond.notify_all();
    }

    // Delete copy constructor and assignment operator to prevent copying
    ThreadPool(const ThreadPool &) = delete;
    ThreadPool &operator=(const ThreadPool &) = delete;

    /**
     * @brief Add a task to the thread pool
     * @tparam T The type of the task (typically a callable object)
     * @param task The task to be added to the pool
     */
    template <typename T>
    void AddTask(T &&task);

private:
    /**
     * @struct Pool
     * @brief Internal structure to manage the thread pool's shared state
     */
    struct Pool
    {
        std::atomic_bool is_stop;                ///< Flag to signal threads to stop
        std::mutex mtx;                          ///< Mutex for thread synchronization
        std::condition_variable cond;            ///< Condition variable for thread synchronization
        std::queue<std::function<void()>> tasks; ///< Queue of tasks to be executed
    };
    std::shared_ptr<Pool> pool_; ///< Shared pointer to the pool's state
};

template <typename T>
void ThreadPool::AddTask(T &&task)
{
    if (pool_)
    {
        std::lock_guard<std::mutex> lock{pool_->mtx};
        pool_->tasks.emplace(std::forward<T>(task));
    }
    pool_->cond.notify_one();
}