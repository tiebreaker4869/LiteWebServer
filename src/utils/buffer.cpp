#include "buffer.hpp"
#include <cassert>
#include <sys/uio.h>
#include <cstddef>
#include <string>
#include <unistd.h>

Buffer::Buffer(int capacity) : buffer_(capacity) {}

size_t Buffer::ReadableBytes() const
{
    return write_pos_ - read_pos_;
}

size_t Buffer::WritableBytes() const
{
    return buffer_.size() - write_pos_;
}

size_t Buffer::ReadBytes() const
{
    return read_pos_;
}

void Buffer::InitPtr()
{
    // fill buffer_ with 0
    std::fill(buffer_.begin(), buffer_.end(), 0);
    read_pos_ = 0;
    write_pos_ = 0;
}

char *Buffer::BeginPtr_()
{
    return buffer_.data();
}

const char *Buffer::ConstBeginPtr_() const
{
    return buffer_.data();
}

const char *Buffer::ReadPtr() const
{
    return ConstBeginPtr_() + read_pos_;
}

const char *Buffer::ConstWritePtr() const
{
    return ConstBeginPtr_() + write_pos_;
}

char *Buffer::WritePtr()
{
    return BeginPtr_() + write_pos_;
}

void Buffer::UpdateReadPtr(size_t len)
{
    assert(len <= ReadableBytes());
    read_pos_ += len;
}

void Buffer::UpdateReadPtrUntilEnd(const char *end)
{
    assert(ReadPtr() <= end);
    read_pos_ = end - ReadPtr();
}

std::string Buffer::RetrieveAllToStr()
{
    std::string result(ReadPtr(), ReadableBytes());
    InitPtr();
    return result;
}

void Buffer::EnsureWritable(size_t len)
{
    if (WritableBytes() < len)
    {
        AllocSpace_(len);
    }
    assert(WritableBytes() >= len);
}

void Buffer::HasWritten(size_t len)
{
    assert(len <= WritableBytes());
    write_pos_ += len;
}

ssize_t Buffer::ReadFd(int fd, int *save_errno)
{
    char extra_buf[65536];
    struct iovec vec[2];
    const size_t writable = WritableBytes();

    vec[0].iov_base = WritePtr();
    vec[0].iov_len = writable;
    vec[1].iov_base = extra_buf;
    vec[1].iov_len = sizeof(extra_buf);

    const ssize_t len = readv(fd, vec, 2);
    if (len < 0)
    {
        *save_errno = errno;
    }
    else if (static_cast<size_t>(len) <= writable)
    {
        HasWritten(len);
    }
    else
    {
        write_pos_ = buffer_.size();
        Append(extra_buf, len - writable);
    }
    return len;
}

ssize_t Buffer::WriteFd(int fd, int *save_errno)
{
    ssize_t read_size = ReadableBytes();
    ssize_t len = write(fd, ReadPtr(), read_size);
    if (len < 0)
    {
        *save_errno = errno;
        return len;
    }
    UpdateReadPtr(len);
    return len;
}

void Buffer::AllocSpace_(size_t len)
{
    if (WritableBytes() + ReadBytes() < len)
    {
        buffer_.resize(write_pos_ + len + 1);
    }
    else
    {
        std::copy(BeginPtr_() + read_pos_, BeginPtr_() + write_pos_, BeginPtr_());
        read_pos_ = 0;
        write_pos_ = read_pos_ + ReadableBytes();
        assert(ReadableBytes() == write_pos_ - read_pos_);
    }
}

void Buffer::Append(const std::string &str)
{
    Append(str.data(), str.size());
}

void Buffer::Append(const char *str, size_t len)
{
    assert(str && len > 0);
    EnsureWritable(len);
    std::copy(str, str + len, WritePtr());
    HasWritten(len);
}

void Buffer::Append(const void *data, size_t len)
{
    assert(data && len > 0);
    Append(static_cast<const char *>(data), len);
}

void Buffer::Append(const Buffer &buffer)
{
    Append(buffer.ReadPtr(), buffer.ReadableBytes());
}