#include "httpconn.hpp"

#include <cassert>
#include <unistd.h>

#include "log.hpp"

const char *HttpConn::src_dir;
std::atomic<int> HttpConn::user_count;
bool HttpConn::isET;

HttpConn::HttpConn()
{
    fd_ = -1;
    addr_ = {0};
    is_close_ = true;
}

HttpConn::~HttpConn()
{
    Close();
}

void HttpConn::Init(int fd, const sockaddr_in &addr)
{
    user_count++;
    addr_ = addr;
    fd_ = fd;
    is_close_ = false;
    read_buff_.InitPtr();
    write_buff_.InitPtr();
    LOG_INFO("Client[%d](%s:%d) in, userCount:%d", fd_, GetIP(), GetPort(), (int)user_count);
}

void HttpConn::Close()
{
    response_.UnmapFile();
    if (is_close_ == false)
    {
        is_close_ = true;
        user_count--;
        close(fd_);
        LOG_INFO("Client[%d](%s:%d) quit, UserCount:%d", fd_, GetIP(), GetPort(), (int)user_count);
    }
}

int HttpConn::GetFd() const
{
    return fd_;
}

sockaddr_in HttpConn::GetAddr() const
{
    return addr_;
}

const char *HttpConn::GetIP() const
{
    return inet_ntoa(addr_.sin_addr);
}

int HttpConn::GetPort() const
{
    return addr_.sin_port;
}

ssize_t HttpConn::Read(int *save_errno)
{
    ssize_t len = -1;

    do
    {
        len = read_buff_.ReadFd(fd_, save_errno);
        if (len <= 0)
        {
            break;
        }

    } while (isET); // 在 ET 模式下，需要循环读取，直到没有数据可读

    return len;
}

ssize_t HttpConn::Write(int *save_errno)
{
    ssize_t len = -1;

    do
    {
        len = writev(fd_, iov_, iov_cnt_);
        if (len <= 0)
        {
            break;
        }
        if (iov_[0].iov_len + iov_[1].iov_len == 0)
        {
            break;
        }
        else if (static_cast<size_t>(len) > iov_[0].iov_len)
        {
            iov_[1].iov_base = (uint8_t *)iov_[1].iov_base + (len - iov_[0].iov_len);
            iov_[1].iov_len -= (len - iov_[0].iov_len);
            if (iov_[0].iov_len)
            {
                write_buff_.InitPtr();
                iov_[0].iov_len = 0;
            }
        }
        else
        {
            iov_[0].iov_base = (uint8_t *)iov_[0].iov_base + len;
            iov_[0].iov_len -= len;
            write_buff_.UpdateReadPtr(len);
        }
    } while (isET || ToWriteBytes() > 10240); // 在 ET 模式下，需要循环写入，直到没有数据可写

    return len;
}

bool HttpConn::Handle()
{
    request_.Init();
    if (read_buff_.ReadableBytes() <= 0)
    {
        return false;
    }
    else if (request_.Parse(read_buff_))
    {
        LOG_DEBUG("%s", request_.Path().c_str());
        response_.Init(src_dir, request_.Path(), request_.IsKeepAlive(), 200);
    }
    else
    {
        response_.Init(src_dir, request_.Path(), false, 400);
    }

    response_.MakeResponse(write_buff_);

    // write write_buff_ to response
    iov_[0].iov_base = const_cast<char *>(write_buff_.ReadPtr());
    iov_[0].iov_len = write_buff_.ReadableBytes();
    iov_cnt_ = 1;

    // set response file
    if (response_.FileLen() > 0 && response_.File())
    {
        iov_[1].iov_base = response_.File();
        iov_[1].iov_len = response_.FileLen();
        iov_cnt_ = 2;
    }
    LOG_DEBUG("filesize: %d, %d  to %d", response_.FileLen(), iov_cnt_, ToWriteBytes());
    return true;
}