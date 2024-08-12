#ifndef LWS_LOG_BLOCK_QUEUE_H_
#define LWS_LOG_BLOCK_QUEUE_H_

#include <iostream>
#include <cstdlib>
#include <sys/time.h>

#include "lock/locker.h"

// 循环数组实现的线程安全的阻塞队列，一把大🔓保平安
template <typename T>
class BlockQueue
{
public:
    BlockQueue(int max_size = 1000)
        : max_size_(max_size), size_(0), front_(0), back_(0)
    {
        if (max_size <= 0)
        {
            exit(-1);
        }

        array_ = new T[max_size_];
    }

    ~BlockQueue()
    {
        LockGuard lock(mutex_);
        if (array_)
        {
            delete[] array_;
        }
    }

    void Clear()
    {
        LockGuard lock(mutex_);
        front_ = 0;
        back_ = 0;
        size_ = 0;
    }

    bool Full()
    {
        LockGuard lock(mutex_);
        return size_ >= max_size_;
    }

    bool Empty()
    {
        LockGuard lock(mutex_);
        return size_ == 0;
    }

    bool Front(T &elem)
    {
        LockGuard lock(mutex_);
        if (size_ == 0)
        {
            return false;
        }
        elem = array_[front_];
        return true;
    }

    bool Back(T &elem)
    {
        LockGuard lock(mutex_);
        if (size_ == 0)
        {
            return false;
        }
        int back_index = (back_ - 1 + max_size_) % max_size_;
        elem = array_[back_index];
        return true;
    }

    int Size()
    {
        LockGuard lock(mutex_);
        return size_;
    }

    int MaxSize()
    {
        LockGuard lock(mutex_);
        return max_size_;
    }

    bool Push(const T &elem)
    {
        LockGuard lock(mutex_);
        while (size_ >= max_size_) // 使用 while 而不是 if 处理虚假唤醒
        {
            cond_.Wait(mutex_.Get());
        }

        array_[back_] = elem;
        back_ = (back_ + 1) % max_size_;
        ++size_;

        cond_.Broadcast(); // 通知消费者有新元素可用
        return true;
    }

    bool Pop(T &elem)
    {
        LockGuard lock(mutex_);
        while (size_ <= 0) // 使用 while 而不是 if 处理虚假唤醒
        {
            cond_.Wait(mutex_.Get());
        }

        elem = array_[front_];
        front_ = (front_ + 1) % max_size_;
        --size_;

        cond_.Broadcast(); // 通知生产者有空位可用
        return true;
    }

    bool Pop(T &elem, int ms_timeout)
    {
        LockGuard lock(mutex_);
        struct timespec t;
        struct timeval now;
        gettimeofday(&now, nullptr);

        t.tv_sec = now.tv_sec + ms_timeout / 1000;
        t.tv_nsec = (ms_timeout % 1000) * 1000 * 1000;

        while (size_ <= 0)
        {
            if (!cond_.TimeWait(mutex_.Get(), t))
            {
                return false; // 超时返回 false
            }
        }

        elem = array_[front_];
        front_ = (front_ + 1) % max_size_;
        --size_;

        cond_.Broadcast(); // 通知生产者有空位可用
        return true;
    }

private:
    Mutex mutex_;
    ConditionVariable cond_;
    T *array_;
    int size_;
    int max_size_;
    int front_;
    int back_;
};

#endif