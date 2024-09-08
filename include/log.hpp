#pragma once

#include <mutex>
#include <thread>
#include <string>
#include <iostream>
#include <memory>
#include <stdarg.h>

#include "buffer.hpp"
#include "block_deque.hpp"

/**
 * @class Log
 * @brief A singleton class for handling logging operations.
 *
 * This class provides functionality for initializing, writing, and managing log files.
 * It supports both synchronous and asynchronous logging, with the ability to set log levels
 * and customize log file paths.
 */
class Log
{
public:
    /**
     * @brief Initialize the log system.
     * @param level The initial log level.
     * @param path The directory path for log files (default: "./log").
     * @param suffix The suffix for log files (default: ".log").
     * @param max_queue_capacity The maximum capacity of the asynchronous logging queue (default: 1024).
     */
    void Init(int level, const char *path = "./log",
              const char *suffix = ".log", int max_queue_capacity = 1024);

    /**
     * @brief Get the singleton instance of the Log class.
     * @return Pointer to the Log instance.
     */
    static Log *GetInstance();

    /**
     * @brief Flush the log thread (used for asynchronous logging).
     */
    static void FlushLogThread();

    /**
     * @brief Write a log message.
     * @param level The log level of the message.
     * @param format The format string for the log message.
     * @param ... Variable arguments for the format string.
     */
    void Write(int level, const char *format, ...);

    /**
     * @brief Flush the log buffer to ensure all messages are written.
     */
    void Flush();

    /**
     * @brief Get the current log level.
     * @return The current log level.
     */
    int GetLevel();

    /**
     * @brief Set the log level.
     * @param level The new log level to set.
     */
    void SetLevel(int level);

    /**
     * @brief Check if the log system is open.
     * @return True if the log system is open, false otherwise.
     */
    bool IsOpen() { return is_open_; }

private:
    Log() = default;
    ~Log();

    /**
     * @brief Append the log level title to the log message.
     * @param level The log level.
     */
    void AppendLogLevelTitle_(int level);

    /**
     * @brief Perform asynchronous writing of log messages.
     */
    void AsyncWrite_();

    static const int kLogPathLen = 256; ///< Maximum length of the log file path
    static const int kLogNameLen = 256; ///< Maximum length of the log file name
    static const int kMaxLines = 50000; ///< Maximum number of lines per log file

    const char *path_;   ///< Path to the log directory
    const char *suffix_; ///< Suffix for log files

    int max_lines_;   ///< Maximum number of lines per log file
    int line_cnt_{0}; ///< Current line count in the log file
    int level_{0};    ///< Current log level
    int to_day_{0};   ///< Current day (for daily log rotation)

    FILE *fp_{nullptr}; ///< File pointer for the current log file
    Buffer buff_{};     ///< Buffer for log messages

    bool is_open_{false};  ///< Flag indicating if the log system is open
    bool is_async_{false}; ///< Flag indicating if asynchronous logging is enabled

    std::unique_ptr<BlockDeque<std::string>> deq_ptr_{nullptr}; ///< Queue for asynchronous logging
    std::unique_ptr<std::thread> write_thread_ptr_{nullptr};    ///< Thread for asynchronous writing
    std::mutex mtx_;                                            ///< Mutex for thread-safe operations
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