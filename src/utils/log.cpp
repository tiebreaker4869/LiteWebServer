#include "log.hpp"

#include <mutex>
#include <ctime>
#include <sys/stat.h>

Log::~Log()
{
    if (write_thread_ptr_ && write_thread_ptr_->joinable())
    {
        while (!deq_ptr_->Empty())
        {
            deq_ptr_->Flush();
        }
        deq_ptr_->Close();
        write_thread_ptr_->join();
    }

    if (fp_)
    {
        std::lock_guard<std::mutex> lock{mtx_};
        Flush();
        fclose(fp_);
    }
}

int Log::GetLevel()
{
    std::lock_guard<std::mutex> lock{mtx_};
    return level_;
}

void Log::SetLevel(int level)
{
    std::lock_guard<std::mutex> lock{mtx_};
    level_ = level;
}

void Log::Init(int level, const char *path,
               const char *suffix, int max_queue_size)
{
    is_open_ = true;
    level_ = level;
    if (max_queue_size > 0)
    {
        deq_ptr_ = std::make_unique<BlockDeque<std::string>>(max_queue_size);
        write_thread_ptr_ = std::make_unique<std::thread>(FlushLogThread);
    }
    else
    {
        is_async_ = false;
    }

    line_cnt_ = 0;
    time_t timer = time(nullptr);
    struct tm *sys_tm = localtime(&timer);
    struct tm t = *sys_tm; // 将时间结构体复制给t
    path_ = path;
    suffix_ = suffix;
    char file_name[kLogNameLen] = {0};
    snprintf(file_name, kLogNameLen - 1, "%s/%04d_%02d_%02d%s.log", path_, t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, suffix_);
    to_day_ = t.tm_mday;

    {
        std::lock_guard<std::mutex> lock{mtx_};
        buff_.InitPtr();
        if (fp_)
        { // 如果文件指针已经打开，则刷新缓冲区并关闭文件
            Flush();
            fclose(fp_);
        }
        fp_ = fopen(file_name, "a");
        if (fp_ == nullptr)
        {
            mkdir(path_, 0777);
            if ((fp_ = fopen(file_name, "a")) == nullptr)
            {
                perror("fopen failed");
                exit(EXIT_FAILURE);
            }
        }
    }
}

void Log::Write(int level, const char *format, ...)
{
    struct timeval now = {0, 0};
    gettimeofday(&now, nullptr);
    time_t t_sec = now.tv_sec;
    struct tm *sys_tm = localtime(&t_sec);
    struct tm t = *sys_tm;
    va_list valst;
    if (to_day_ != t.tm_mday || (line_cnt_ && (line_cnt_ % kMaxLines == 0)))
    {
        // create new log file
        {
            std::lock_guard<std::mutex> lock{mtx_};

            char new_file[kLogNameLen];
            char tail[36] = {0};
            snprintf(tail, 36, "%04d_%02d_%02d", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday);
            ;

            if (to_day_ != t.tm_mday)
            {
                snprintf(new_file, kLogNameLen - 72, "%s/%s%s.log", path_, tail, suffix_);
                to_day_ = t.tm_mday;
                line_cnt_ = 0;
            }
            else
            {
                snprintf(new_file, kLogNameLen - 72, "%s/%s-%d%s.log", path_, tail, line_cnt_ / kMaxLines, suffix_);
            }
            Flush();
            fclose(fp_);

            if ((fp_ = fopen(new_file, "a")) == nullptr)
            {
                perror("fopen failed");
                exit(EXIT_FAILURE);
            }
        }
    }
    // write message to log
    {
        std::lock_guard<std::mutex> lock{mtx_};
        line_cnt_++;
        int n = snprintf(buff_.WritePtr(), 128, "%d-%02d-%02d %02d:%02d:%02d.%06ld",
                         t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec, now.tv_usec);
        buff_.HasWritten(n);
        AppendLogLevelTitle_(level);

        va_start(valst, format);
        int m = vsnprintf(buff_.WritePtr(), buff_.WritableBytes(), format, valst);
        va_end(valst);
        buff_.HasWritten(m);
        buff_.Append("\n\0", 2);

        if (is_async_ && deq_ptr_ && !deq_ptr_->Full())
        {
            deq_ptr_->PushBack(buff_.RetrieveAllToStr());
        }
        else
        {
            fputs(buff_.ReadPtr(), fp_);
        }
        buff_.InitPtr();
    }
}

void Log::AppendLogLevelTitle_(int level)
{
    switch (level)
    {
    case 0:
        buff_.Append("[debug]: ", 9);
        break;
    case 1:
        buff_.Append("[info]: ", 9);
        break;
    case 2:
        buff_.Append("[warn]: ", 9);
        break;
    case 3:
        buff_.Append("[error]: ", 9);
        break;
    default:
        buff_.Append("[info]: ", 9);
        break;
    }
}

void Log::Flush()
{
    std::lock_guard<std::mutex> lock{mtx_};
    if (is_async_)
    {
        deq_ptr_->Flush();
    }
    fflush(fp_);
}

void Log::AsyncWrite_()
{
    std::string str = "";
    while (deq_ptr_->PopFront(str))
    {
        std::lock_guard<std::mutex> lock{mtx_};
        fputs(str.c_str(), fp_);
    }
}

Log *Log::GetInstance()
{
    static Log instance;
    return &instance;
}

void Log::FlushLogThread()
{
    Log::GetInstance()->AsyncWrite_();
}