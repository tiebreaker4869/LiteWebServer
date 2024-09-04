#pragma once

#include <mutex>
#include <thread>
#include <string>
#include <iostream>
#include <memory>

#include "buffer.hpp"
#include "block_deque.hpp"

class Log
{
public:
    void Init(int level, const char *path = "./log",
              const char *suffix = ".log", int max_queue_capacity = 1024);

    static Log *GetInstance();
    static void FlushLogThread();

    void Write(int level, const char *format, ...);
    void Flush();

    int GetLevel();
    void SetLevel(int level);
    bool IsOpen() { return is_open_; }

private:
    Log() = default;
    ~Log();
    void AppendLogLevelTitle_(int level);
    void AsyncWrite_();

    static const int kLogPathLen = 256;
    static const int kLogNameLen = 256;
    static const int kMaxLines = 50000;

    const char *path_;
    const char *suffix_;

    int max_lines_;
    int line_cnt_{0};
    int level_{0};
    int to_day_{0};

    FILE *fp_{nullptr};
    Buffer buff_{};

    bool is_open_{false};
    bool is_async_{false};

    std::unique_ptr<BlockDeque<std::string>> deq_ptr_{nullptr};
    std::unique_ptr<std::thread> write_thread_ptr_{nullptr};
    std::mutex mtx_;
};

#define LOG_BASE(level, format, ...)                   \
    do                                                 \
    {                                                  \
        Log *log = Log::GetInstance();                 \
        if (log->IsOpen() && log->GetLevel() <= level) \
        {                                              \
            log->Write(level, format, ##__VA_ARGS__);  \
            log->Flush();                              \
        }                                              \
    } while (0);

#define LOG_DEBUG(format, ...)             \
    do                                     \
    {                                      \
        LOG_BASE(0, format, ##__VA_ARGS__) \
    } while (0);
#define LOG_INFO(format, ...)              \
    do                                     \
    {                                      \
        LOG_BASE(1, format, ##__VA_ARGS__) \
    } while (0);
#define LOG_WARN(format, ...)              \
    do                                     \
    {                                      \
        LOG_BASE(2, format, ##__VA_ARGS__) \
    } while (0);
#define LOG_ERROR(format, ...)             \
    do                                     \
    {                                      \
        LOG_BASE(3, format, ##__VA_ARGS__) \
    } while (0);