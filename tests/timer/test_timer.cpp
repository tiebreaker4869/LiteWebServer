#include <gtest/gtest.h>
#include "timer/timer.h"

// 测试 TimerManager 的 AddTimer 方法
TEST(TimerManagerTest, AddTimer)
{
    TimerManager manager;
    UtilTimer *timer = new UtilTimer();
    timer->expire = time(nullptr) + 5; // 设置过期时间为当前时间加5秒
    manager.AddTimer(timer);
}

// 测试 TimerManager 的 AdjustTimer 方法
TEST(TimerManagerTest, AdjustTimer)
{
    TimerManager manager;
    UtilTimer *timer = new UtilTimer();
    timer->expire = time(nullptr) + 5;
    manager.AddTimer(timer);

    // 调整定时器的过期时间
    timer->expire = time(nullptr) + 10;
    manager.AdjustTimer(timer);
}

// 测试 TimerManager 的 RemoveTimer 方法
TEST(TimerManagerTest, RemoveTimer)
{
    TimerManager manager;
    UtilTimer *timer = new UtilTimer();
    timer->expire = time(nullptr) + 5;
    manager.AddTimer(timer);
    manager.RemoveTimer(timer);
}

// 测试 TimerManager 的 Tick 方法
TEST(TimerManagerTest, Tick)
{
    TimerManager manager;
    UtilTimer *timer = new UtilTimer();
    timer->expire = time(nullptr) + 1;      // 设置过期时间为1秒后
    timer->cb_func = [](ClientData *cd) {}; // 设置回调函数
    manager.AddTimer(timer);

    // 等待1秒后调用 Tick
    sleep(1);
    manager.Tick();
}
