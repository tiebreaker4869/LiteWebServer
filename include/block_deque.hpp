#pragma once

#include <cassert>
#include <mutex>
#include <deque>
#include <condition_variable>
#include <sys/time.h>

template <typename T>
class BlockDeque
{
public:
    explicit BlockDeque(size_t capacity = 1024);
    ~BlockDeque();

    void Clear();
    void Close();
    void Flush();

    void PushBack(const T &item);
    void PushFront(const T &item);
    bool PopFront(T &item);
    bool PopFront(T &item, int timeout);

    bool Empty() const;
    bool Full() const;
    size_t Size() const;
    size_t Capacity() const;
    T Front() const;
    T Back() const;

private:
    std::deque<T> deq_;
    size_t capacity_;
    mutable std::mutex mtx_;
    bool is_close_;
    std::condition_variable cond_consumer_;
    std::condition_variable cond_producer_;
};

template <typename T>
BlockDeque<T>::BlockDeque(size_t capacity) : capacity_{capacity}
{
    assert(capacity > 0);
    is_close_ = false;
}

template <typename T>
BlockDeque<T>::~BlockDeque()
{
    Close();
}

template <typename T>
void BlockDeque<T>::Close()
{
    {
        std::lock_guard<std::mutex> guard{mtx_};
        deq_.clear();
        is_close_ = true;
    }

    cond_producer_.notify_all();
    cond_consumer_.notify_all();
}

template <typename T>
void BlockDeque<T>::Flush()
{
    cond_consumer_.notify_all();
}

template <typename T>
void BlockDeque<T>::Clear()
{
    std::lock_guard<std::mutex> guard{mtx_};
    deq_.clear();
}

template <typename T>
T BlockDeque<T>::Front() const
{
    std::lock_guard<std::mutex> guard{mtx_};
    return deq_.front();
}

template <typename T>
T BlockDeque<T>::Back() const
{
    std::lock_guard<std::mutex> guard{mtx_};
    return deq_.back();
}

template <typename T>
bool BlockDeque<T>::Empty() const
{
    std::lock_guard<std::mutex> guard{mtx_};
    return deq_.empty();
}

template <typename T>
size_t BlockDeque<T>::Capacity() const
{
    std::lock_guard<std::mutex> guard{mtx_};
    return capacity_;
}

template <typename T>
size_t BlockDeque<T>::Size() const
{
    std::lock_guard<std::mutex> guard{mtx_};
    return deq_.size();
}

template <typename T>
bool BlockDeque<T>::Full() const
{
    std::lock_guard<std::mutex> guard{mtx_};
    return deq_.size() >= capacity_;
}

template <typename T>
void BlockDeque<T>::PushBack(const T &item)
{
    std::unique_lock<std::mutex> lock{mtx_};
    while (deq_.size() >= capacity_)
    {
        cond_producer_.wait(lock)
    }
    deq_.push_back(item);
    cond_consumer_.notify_one();
}

template <typename T>
void BlockDeque<T>::PushFront(const T &item)
{
    std::unique_lock<std::mutex> lock{mtx_};
    while (deq_.size() >= capacity_)
    {
        cond_producer_.wait(lock);
    }
    deq_.push_front(item);
    cond_consumer_.notify_one();
}

template <typename T>
bool BlockDeque<T>::PopFront(T &item)
{
    std::unique_lock<std::mutex> lock{mtx_};
    while (deq_.empty())
    {
        cond_consumer_.wait(lock);
        if (is_close_)
        {
            return false;
        }
    }
    item = deq_.front();
    deq_.pop_front();
    cond_producer_.notify_one();

    return true;
}

template <typename T>
bool BlockDeque<T>::PopFront(T &item, int timeout)
{
    std::unique_lock<std::mutex> lock{mtx_};
    while (deq_.empty())
    {
        if (cond_consumer_.wait_for(lock, std::chrono::seconds(timeout)) == std::cv_status::timeout)
        {
            return false;
        }
        if (is_close_)
        {
            return false;
        }
    }
    item = deq_.front();
    deq_.pop_front();
    cond_producer_.notify_one();

    return true;
}