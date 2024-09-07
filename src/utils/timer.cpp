#include "timer.hpp"
#include <iostream>
#include <cassert>

void Timer::SiftUp_(size_t i)
{
    assert(i >= 0 && i < heap_.size());
    size_t j = (i - 1) / 2;
    while (j >= 0)
    {
        if (heap_[j] < heap_[i])
        {
            break;
        }
        SwapNode_(i, j);
        i = j;
        j = (i - 1) / 2;
    }
}

void Timer::SwapNode_(size_t i, size_t j)
{
    assert(i >= 0 && i < heap_.size());
    assert(j >= 0 && j < heap_.size());
    std::swap(heap_[i], heap_[j]);
    ref_[heap_[i].id] = i;
    ref_[heap_[j].id] = j;
}

bool Timer::SiftDown_(size_t index, size_t n)
{
    assert(index >= 0 && index < heap_.size());
    assert(n >= 0 && n <= heap_.size());
    size_t i = index;
    size_t j = i * 2 + 1;
    while (j < n)
    {
        if (j + 1 < n && heap_[j + 1] < heap_[j])
        {
            j++;
        }
        if (heap_[i] < heap_[j])
        {
            break;
        }
        SwapNode_(i, j);
        i = j;
        j = i * 2 + 1;
    }
    return i > index;
}

void Timer::AddTimer(int id, int timeout, const TimeoutCallBack &cb)
{
    assert(id >= 0);
    size_t i;
    if (ref_.count(id) == 0)
    {
        i = heap_.size();
        ref_[id] = i;
        heap_.emplace_back(id, Clock::now() + MS(timeout), cb);
        SiftUp_(i);
    }
    else
    {
        i = ref_[id];
        heap_[i].expires = Clock::now() + MS(timeout);
        heap_[i].cb = cb;
        if (!SiftDown_(i, heap_.size()))
        {
            SiftUp_(i);
        }
    }
}

void Timer::Del_(size_t index)
{
    assert(!heap_.empty() && index >= 0 && index < heap_.size());
    size_t i = index;
    size_t n = heap_.size() - 1;
    assert(i <= n);
    if (i < n)
    {
        SwapNode_(i, n);
        if (!SiftDown_(i, n))
        {
            SiftUp_(i);
        }
    }
    ref_.erase(heap_.back().id);
    heap_.pop_back();
}

void Timer::Adjust(int fd, int timeout)
{
    assert(!heap_.empty() && ref_.count(fd) > 0);
    heap_[ref_[fd]].expires = Clock::now() + MS(timeout);
    SiftDown_(ref_[fd], heap_.size());
}

void Timer::Tick()
{
    if (heap_.empty())
    {
        return;
    }

    while (!heap_.empty())
    {
        TimerNode node = heap_.front();
        if (std::chrono::duration_cast<MS>(node.expires - Clock::now()).count() > 0)
        {
            // 最早的也还没 expire
            break;
        }
        node.cb();
        assert(heap_.size() > 0);
        Pop();
    }
}

void Timer::Pop()
{
    assert(!heap_.empty());
    Del_(0);
}

void Timer::Clear()
{
    ref_.clear();
    heap_.clear();
}

int Timer::GetNextTick()
{
    Tick();
    size_t res = -1;
    if (!heap_.empty())
    {
        res = std::max(0l, std::chrono::duration_cast<MS>(heap_.front().expires -
                                                          Clock::now())
                               .count());
    }

    return res;
}