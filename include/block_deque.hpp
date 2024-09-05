#pragma once

#include <cassert>
#include <mutex>
#include <deque>
#include <condition_variable>
#include <sys/time.h>

/**
 * @brief A thread-safe blocking deque implementation.
 *
 * This class provides a double-ended queue with blocking operations,
 * suitable for producer-consumer scenarios. It supports a maximum capacity
 * and provides synchronization mechanisms for thread-safe access.
 *
 * @tparam T The type of elements stored in the deque.
 */
template <typename T>
class BlockDeque
{
public:
    /**
     * @brief Construct a new Block Deque object.
     *
     * @param capacity The maximum number of elements the deque can hold (default: 1024).
     */
    explicit BlockDeque(size_t capacity = 1024);

    /**
     * @brief Destroy the Block Deque object and release any resources.
     */
    ~BlockDeque();

    /**
     * @brief Remove all elements from the deque.
     */
    void Clear();

    /**
     * @brief Close the deque, preventing further operations and notifying all waiting threads.
     */
    void Close();

    /**
     * @brief Notify all waiting consumer threads.
     */
    void Flush();

    /**
     * @brief Add an item to the back of the deque.
     *
     * If the deque is full, this operation will block until space becomes available.
     *
     * @param item The item to be added.
     */
    void PushBack(const T &item);

    /**
     * @brief Add an item to the front of the deque.
     *
     * If the deque is full, this operation will block until space becomes available.
     *
     * @param item The item to be added.
     */
    void PushFront(const T &item);

    /**
     * @brief Remove and return the front item from the deque.
     *
     * If the deque is empty, this operation will block until an item becomes available.
     *
     * @param item Reference to store the removed item.
     * @return true if an item was successfully removed, false if the deque is closed.
     */
    bool PopFront(T &item);

    /**
     * @brief Remove and return the front item from the deque with a timeout.
     *
     * If the deque is empty, this operation will block until an item becomes available
     * or the specified timeout is reached.
     *
     * @param item Reference to store the removed item.
     * @param timeout Maximum time to wait in seconds.
     * @return true if an item was successfully removed, false if timed out or the deque is closed.
     */
    bool PopFront(T &item, int timeout);

    /**
     * @brief Check if the deque is empty.
     *
     * @return true if the deque is empty, false otherwise.
     */
    bool Empty() const;

    /**
     * @brief Check if the deque is full.
     *
     * @return true if the deque is at capacity, false otherwise.
     */
    bool Full() const;

    /**
     * @brief Get the current number of elements in the deque.
     *
     * @return The number of elements in the deque.
     */
    size_t Size() const;

    /**
     * @brief Get the maximum capacity of the deque.
     *
     * @return The maximum number of elements the deque can hold.
     */
    size_t Capacity() const;

    /**
     * @brief Get the front element of the deque without removing it.
     *
     * @return The front element.
     */
    T Front() const;

    /**
     * @brief Get the back element of the deque without removing it.
     *
     * @return The back element.
     */
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