#pragma once

#include <mutex>
#include <queue>
#include <thread>
#include <condition_variable>
#include <functional>
#include <memory>
#include <atomic>

class ThreadPool
{
public:
    explicit ThreadPool(ssize_t thread_num = -1);
    ~ThreadPool();

    template <typename T>
    void AddTask(T &&task);

private:
    struct Pool
    {
        std::atomic_bool is_stop;
        std::mutex mtx;
        std::condition_variable cond;
        std::queue<std::function<void()>> tasks;
    };
    std::shared_ptr<Pool> pool_;
};