#pragma once

#include <chrono>
#include <functional>
#include <unordered_map>
#include <vector>

using TimeoutCallBack = std::function<void()>;
using Clock = std::chrono::high_resolution_clock;
using MS = std::chrono::milliseconds;
using TimeStamp = Clock::time_point;

struct TimerNode
{
    int id;
    TimeStamp expires;
    TimeoutCallBack cb;
    bool operator<(const TimerNode &t) { return expires < t.expires; }
};

class Timer {
public:
    Timer();
    ~Timer();

    void Adjust(int fd, int new_expires);
    void AddTimer(int fd, int timeout, const TimeoutCallBack& cb);
    void DoWork(int fd);
    void Clear();
    void Tick();
    void Pop();
    int GetNextTick();
private:
    std::vector<TimerNode> heap_;
    std::unordered_map<int, size_t> ref_;

    void Del_(size_t i);
    void SiftUp_(size_t i);
    void SiftDown_(size_t i);
    void SwapNode_(size_t i, size_t j);
};