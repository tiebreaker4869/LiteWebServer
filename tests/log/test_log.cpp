#include <gtest/gtest.h>
#include <fstream>
#include <thread>
#include "log/log.h"

// 测试 Log 类的初始化
TEST(LogTest, Init)
{
    LogConfig config = {"test_log", 1024, 5000, 0, 0};
    Log *log = Log::GetInstance();
    EXPECT_TRUE(log->Init(config));
}

// 测试同步写日志功能
TEST(LogTest, SyncWriteLog)
{
    LogConfig config = {"test_log_sync", 1024, 5000, 0, 0}; // max_queue_size 为 0 表示同步
    Log *log = Log::GetInstance();
    log->Init(config);

    log->WriteLog(INFO, "This is a sync test log: %d", 1);
    log->Flush();
}
