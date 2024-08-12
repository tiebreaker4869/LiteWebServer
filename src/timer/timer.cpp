#include "timer/timer.h"

#include <cstring>
#include <cassert>
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/signal.h>

UtilTimer::UtilTimer() : prev{nullptr}, next{nullptr}, cb_func{nullptr}, client_data{nullptr}
{
}

TimerManager::TimerManager()
{
    head_ = new UtilTimer();
    tail_ = new UtilTimer();
    head_->next = tail_;
    tail_->prev = head_;
}

TimerManager::~TimerManager()
{
    UtilTimer *tmp = head_;
    while (tmp)
    {
        UtilTimer *del = tmp;
        tmp = tmp->next;
        delete del;
    }
}

void TimerManager::AddTimer(UtilTimer *timer)
{
    auto current = head_->next;

    while (current != tail_ && current->expire < timer->expire)
    {
        current = current->next;
    }

    // insert timer before current
    timer->next = current;
    timer->prev = current->prev;
    current->prev->next = timer;
    current->prev = timer;
}

void TimerManager::AdjustTimer(UtilTimer *timer)
{
    auto current = timer->next;

    if (current == tail_ || current->expire > timer->expire)
    {
        return;
    }

    timer->prev->next = timer->next;
    timer->next->prev = timer->prev;

    while (current != tail_ && current->expire < timer->expire)
    {
        current = current->next;
    }

    // insert timer before current
    timer->next = current;
    timer->prev = current->prev;
    current->prev->next = timer;
    current->prev = timer;
}

void TimerManager::RemoveTimer(UtilTimer *timer)
{
    timer->prev->next = timer->next;
    timer->next->prev = timer->prev;
    delete timer;
}

void TimerManager::Tick()
{
    time_t cur = time(nullptr);
    UtilTimer *tmp = head_->next;

    while (tmp != tail_)
    {
        if (cur < tmp->expire)
        {
            break;
        }

        tmp->cb_func(tmp->client_data);
        head_->next = tmp->next;
        tmp->next->prev = head_;
        delete tmp;
        tmp = head_->next;
    }
}

void Utils::Init(int timeslot)
{
    this->timeslot = timeslot;
}

int Utils::SetNonBlocking(int fd)
{
    int old_options = fcntl(fd, F_GETFL);
    int new_options = old_options | O_NONBLOCK;
    fcntl(fd, new_options);
    return old_options;
}

void Utils::EPollRegisterFd(int epollfd, int fd, bool one_shot, int trig_mode)
{
    epoll_event event;
    event.data.fd = fd;

    event.events = EPOLLIN | EPOLLRDHUP;

    if (trig_mode == 1)
    {
        event.events |= EPOLLET;
    }

    if (one_shot)
    {
        event.events |= EPOLLONESHOT;
    }

    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    SetNonBlocking(fd);
}

void Utils::HandleSignal(int sig)
{
    // 为了保证可重入性，保留原来的 errno
    int save_errno = errno;
    int msg = sig;
    send(pipefd[1], (char *)&msg, 1, 0);
    errno = save_errno;
}

void Utils::RegisterSignalHandler(int sig, void (*handler)(int), bool restart = true)
{
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handler;
    if (restart)
    {
        sa.sa_flags |= SA_RESTART;
    }
    sigfillset(&sa.sa_mask);
    assert(sigaction(sig, &sa, nullptr) != -1);
}

// 处理定时任务，重新定时以不断周期性触发 SIGALARM 信号
void Utils::HandleTimerEvent()
{
    manager.Tick();
    alarm(timeslot);
}

void Utils::PrintError(int connfd, const char* info) {
    send(connfd, info, strlen(info), 0);
    close(connfd);
}