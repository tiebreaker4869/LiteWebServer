#pragma once

#include <arpa/inet.h>
#include <atomic>
#include <sys/uio.h>
#include "buffer.hpp"
#include "http_request.hpp"
#include "http_response.hpp"

class HttpConn
{
public:
    HttpConn();
    ~HttpConn();

    void Init(int sockfd, const sockaddr_in &addr);
    ssize_t Read(int *save_errno);
    ssize_t Write(int *save_errno);
    void Close();
    // Process HTTP connections, parse requests and generate responses
    bool Handle();
    // Interfaces to get connection information
    int GetFd() const;
    int GetPort() const;
    const char *GetIP() const;
    sockaddr_in GetAddr() const;

    // amounts of data need to be written
    int ToWriteBytes() const
    {
        return iov_[0].iov_len + iov_[1].iov_len;
    }

    bool IsKeepAlive() const
    {
        return request_.IsKeepAlive();
    }

    static bool isET;
    static const char *src_dir;
    static std::atomic<int> user_count; // number of all http connections
private:
    int fd_; // fd for the http conn
    bool is_close_;

    struct sockaddr_in addr_;

    int iov_cnt_;
    struct iovec iov_[2];

    Buffer read_buff_;
    Buffer write_buff_;

    HttpRequest request_;
    HttpResponse response_;
};