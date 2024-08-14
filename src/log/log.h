#ifndef LWS_LOG_LOG_H_
#define LWS_LOG_LOG_H_

#include <cstdio>
#include <string>
#include <memory>

#include "lock/locker.h"
#include "log/block_queue.h"

enum LogLevel
{
    DEBUG = 0,
    INFO,
    WARN,
    ERROR,
};

// 可选择的参数有日志文件、日志缓冲区大小、最大行数以及最长日志条队列
struct LogConfig
{
    std::string log_name;
    int log_buf_size = 8192;
    int split_lines = 5000000;
    int max_queue_size = 0;
    int close_log;
};

class Log
{
public:
    static Log *GetInstance()
    {
        static Log instance;
        return &instance;
    }

    static void *FlushLogThread(void *args)
    {
        Log::GetInstance()->AsyncWriteLog();
        return nullptr;
    }

    bool Init(LogConfig config);

    void WriteLog(LogLevel level, const char *format, ...);

    void Flush();

private:
    Log();

    ~Log();

    void AsyncWriteLog()
    {
        std::string single_log;

        while (log_queue_->Pop(single_log))
        {
            LockGuard lock(mutex_);
            fputs(single_log.c_str(), fp_);
        }
    }

    char dir_name_[128];   // 路径名
    char log_name_[128];   // log 文件名
    int split_lines_;      // 日志最大行数
    int log_buf_size_;     // 日志缓冲区打下
    long long line_count_; // 日志当前行数
    int date_;             // 因为要按天分类，记录当前哪一天
    FILE *fp_;             // 打开 log 文件的 ptr
    char *buf_;
    std::unique_ptr<BlockQueue<std::string>> log_queue_;
    bool is_async_; // 是否异步写日志
    Mutex mutex_;
    int close_log; // 日志开关
};

#define LOG_BASE(level, format, ...)                                \
    if (close_log == 0)                                             \
    {                                                               \
        Log::GetInstance()->WriteLog(level, format, ##__VA_ARGS__); \
        Log::GetInstance()->Flush();                                \
    }

#define LOG_DEBUG(format, ...) LOG_BASE(DEBUG, format, ##__VA_ARGS__)

#define LOG_INFO(format, ...) LOG_BASE(INFO, format, ##__VA_ARGS__)

#define LOG_WARN(format, ...) LOG_BASE(WARN, format, ##__VA_ARGS__)

#define LOG_ERROR(format, ...) LOG_BASE(ERROR, format, ##__VA_ARGS__)

#endif