#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>
#include <sys/epoll.h>

class Epoll
{
public:
    explicit Epoll(int max_event = 4096);
    ~Epoll();

    bool AddFd(int fd, uint32_t events);
    bool ModifyFd(int fd, uint32_t events);
    bool DeleteFd(int fd, uint32_t events);

    int Wait(int timeout_ms = -1);
    int GetEventFd(size_t i) const;
    uint32_t GetEvents(size_t i) const;
private:
    int epoll_fd_;
    std::vector<struct epoll_event> events_;
};