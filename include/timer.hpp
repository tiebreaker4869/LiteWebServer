#pragma once

#include <chrono>
#include <functional>
#include <unordered_map>
#include <vector>

using TimeoutCallBack = std::function<void()>;
using Clock = std::chrono::high_resolution_clock;
using MS = std::chrono::milliseconds;
using TimeStamp = Clock::time_point;

/**
 * @struct TimerNode
 * @brief Represents a timer node with an ID, expiration time, and callback function.
 */
struct TimerNode
{
    int id;             ///< Unique identifier for the timer
    TimeStamp expires;  ///< Expiration time of the timer
    TimeoutCallBack cb; ///< Callback function to be executed when the timer expires
    bool operator<(const TimerNode &t) { return expires < t.expires; }
    TimerNode(int id, TimeStamp expires, TimeoutCallBack cb) : id(id), expires(expires), cb(cb) {}
};

/**
 * @class Timer
 * @brief Manages a collection of timers using a min-heap data structure.
 *
 * This class provides functionality to add, adjust, and manage timers
 * efficiently using a min-heap implementation.
 */
class Timer
{
public:
    /**
     * @brief Construct a new Timer object
     */
    Timer() { heap_.reserve(64); }

    /**
     * @brief Destroy the Timer object
     */
    ~Timer() { Clear(); }

    /**
     * @brief Adjust the expiration time of an existing timer
     * @param fd File descriptor associated with the timer
     * @param new_expires New expiration time in milliseconds
     */
    void Adjust(int fd, int new_expires);

    /**
     * @brief Add a new timer to the collection
     * @param fd File descriptor associated with the timer
     * @param timeout Timeout duration in milliseconds
     * @param cb Callback function to be executed when the timer expires
     */
    void AddTimer(int fd, int timeout, const TimeoutCallBack &cb);

    /**
     * @brief Clear all timers from the collection
     */
    void Clear();

    /**
     * @brief Process expired timers and update the heap
     */
    void Tick();

    /**
     * @brief Remove the top (earliest expiring) timer from the heap
     */
    void Pop();

    /**
     * @brief Get the time until the next timer expires
     * @return int Time in milliseconds until the next timer expires
     */
    int GetNextTick();

private:
    std::vector<TimerNode> heap_;         ///< Min-heap to store timer nodes
    std::unordered_map<int, size_t> ref_; ///< Map to store references to timer positions

    /**
     * @brief Delete a timer node from the heap
     * @param i Index of the node to delete
     */
    void Del_(size_t i);

    /**
     * @brief Move a node up the heap to maintain the heap property
     * @param i Index of the node to sift up
     */
    void SiftUp_(size_t i);

    /**
     * @brief Move a node down the heap to maintain the heap property
     * @param index Index of the node to sift down
     * @param n Size of the heap
     * @return true if the node is sifted down, false if the node is already in the correct position
     */
    bool SiftDown_(size_t index, size_t n);

    /**
     * @brief Swap two nodes in the heap
     * @param i Index of the first node
     * @param j Index of the second node
     */
    void SwapNode_(size_t i, size_t j);
};