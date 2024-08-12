#include "log/log.h"
#include <memory>
#include <string>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <cstdarg>

#include <pthread.h>

#include "log/block_queue.h"

Log::Log()
{
    line_count_ = 0;
    is_async_ = false;
}

Log::~Log()
{
    if (fp_)
    {
        fclose(fp_);
    }

    if (buf_)
    {
        delete[] buf_;
    }
}

bool Log::Init(LogConfig config)
{
    // 如果设置了 max_queue_size 则设置为异步
    if (config.max_queue_size >= 1)
    {
        is_async_ = true;
        log_queue_ = std::make_unique<BlockQueue<std::string>>(config.max_queue_size);
        // 创建一个线程来异步写日志
        pthread_t tid;
        pthread_create(&tid, nullptr, FlushLogThread, nullptr);
    }

    close_log = config.close_log;
    log_buf_size_ = config.log_buf_size;
    buf_ = new char[log_buf_size_];
    memset(buf_, 0, log_buf_size_);
    split_lines_ = config.split_lines;

    time_t t = time(nullptr);
    struct tm *sys_tm = localtime(&t);
    struct tm my_time = *sys_tm;

    const char *p = strrchr(config.log_name.c_str(), '/');
    char log_full_name[256] = {0};

    if (p == nullptr)
    {
        snprintf(log_full_name, 255, "%d_%02d_%02d_%s", my_time.tm_year + 1900, my_time.tm_mon + 1, my_time.tm_mday, config.log_name.c_str());
    }
    else
    {
        strcpy(log_name_, p + 1);
        strncpy(dir_name_, config.log_name.c_str(), p - config.log_name.c_str() + 1);
        snprintf(log_full_name, 255, "%s%d_%02d_%02d_%s", dir_name_, my_time.tm_year + 1900, my_time.tm_mon + 1, my_time.tm_mday, log_name_);
    }

    date_ = my_time.tm_mday;

    fp_ = fopen(log_full_name, "a");

    if (fp_)
    {
        return true;
    }

    return false;
}

void Log::Flush()
{
    LockGuard lock(mutex_);
    fflush(fp_);
}

void Log::WriteLog(LogLevel level, const char *format, ...)
{
    struct timeval now = {0, 0};
    gettimeofday(&now, nullptr);
    time_t t = now.tv_sec;
    struct tm *sys_tm = localtime(&t);
    struct tm my_time = *sys_tm;
    char s[16] = {0};
    switch (level)
    {
    case DEBUG:
        strcpy(s, "[DEBUG]:");
        break;
    case INFO:
        strcpy(s, "[INFO]:");
        break;
    case WARN:
        strcpy(s, "[WARN]:");
        break;
    case ERROR:
        strcpy(s, "[ERROR]:");
        break;
    default:
        strcpy(s, "[INFO]:");
        break;
    }

    LockGuard lock(mutex_);
    ++line_count_;
    // 跨天或者达到最大行数
    if (date_ != my_time.tm_mday || line_count_ % split_lines_ == 0)
    {
        char new_log[256] = {0};
        fflush(fp_);
        fclose(fp_);
        char tail[16] = {0};
        snprintf(tail, 16, "%d_%02d_%02d_", my_time.tm_year + 1900, my_time.tm_mon + 1, my_time.tm_mday);
        if (date_ != my_time.tm_mday)
        {
            snprintf(new_log, 255, "%s%s%s", dir_name_, tail, log_name_);
            date_ = my_time.tm_mday;
            line_count_ = 0;
        }
        else
        {
            snprintf(new_log, 255, "%s%s%s.%lld", dir_name_, tail, log_name_, line_count_ / split_lines_);
        }
        fp_ = fopen(new_log, "a");
    }

    va_list valst;
    va_start(valst, format);

    int n = snprintf(buf_, 48, "%d-%02d-%02d %02d:%02d:%02d.%06ld %s ", my_time.tm_year + 1900, my_time.tm_mon + 1, my_time.tm_mday, my_time.tm_hour, my_time.tm_min, my_time.tm_sec, now.tv_usec, s);
    int m = vsnprintf(buf_ + n, log_buf_size_ - 1, format, valst);
    buf_[n + m] = '\n';
    buf_[n + m + 1] = '\0';

    if (is_async_ && !log_queue_->Full())
    {
        log_queue_->Push(std::string(buf_));
    }
    else
    {
        fputs(buf_, fp_);
    }

    va_end(valst);

    if (close_log)
    {
        fflush(fp_);
        fclose(fp_);
    }

    return;
}