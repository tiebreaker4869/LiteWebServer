#pragma once

#include <cstddef>
#include <string>
#include <vector>
#include <atomic>

class Buffer
{
public:
    Buffer(int buffer_size = 1024);
    ~Buffer() = default;

    size_t WritableBytes() const;
    size_t ReadableBytes() const;
    size_t ReadBytes() const;

    void InitPtr();
    const char *ReadPtr() const;
    const char *ConstWritePtr() const;
    char *WritePtr();
    void UpdateReadPtr(size_t len);
    void UpdateReadPtrUntilEnd(const char *end);
    std::string RetrieveAllToStr();

    void EnsureWritable(size_t len);
    void HasWritten(size_t len);
    void Append(const std::string &str);
    void Append(const char *str, size_t len);
    void Append(const void *data, size_t len);
    void Append(const Buffer &buffer);

    ssize_t ReadFd(int fd, int *save_errno);
    ssize_t WriteFd(int fd, int *save_errno);

private:
    char *BeginPtr_();
    const char *ConstBeginPtr_() const;
    void AllocSpace_(size_t len);

    std::vector<char> buffer_;
    std::atomic<std::size_t> read_pos_{0};
    std::atomic<std::size_t> write_pos_{0};
};