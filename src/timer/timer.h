#ifndef LWS_TIMER_TIMER_H_
#define LWS_TIMER_TIMER_H_
#include <sys/socket.h>
#include <arpa/inet.h>
#include <ctime>

#include "log/log.h"

class UtilTimer;

struct ClientData
{
    sockaddr_in address;
    int socketfd;
    UtilTimer *timer;
};

class UtilTimer
{
public:
    UtilTimer();
    time_t expire;
    void (*cb_func)(ClientData *);
    ClientData* client_data;
    UtilTimer *next;
    UtilTimer *prev;
};

class TimerManager
{
public:
    TimerManager();
    ~TimerManager();

    void AddTimer(UtilTimer *timer);
    void AdjustTimer(UtilTimer *timer);
    void RemoveTimer(UtilTimer *timer);
    void Tick();

private:
    UtilTimer *head_;
    UtilTimer *tail_;
};

class Utils
{
public:
    Utils() = default;
    ~Utils() = default;

    void Init(int timeslot);

    // 对文件设置非阻塞模式
    int SetNonBlocking(int fd);

    void EPollRegisterFd(int epollfd, int fd, bool one_shot, int trig_mode);

    void EPollRemove(int epollfd, int fd);

    void EPollMod(int epollfd, int fd, int ev, int trig_mode);

    static void HandleSignal(int sig);

    void HandleTimerEvent();

    void RegisterSignalHandler(int sig, void (*handler)(int), bool restart = true);

    void PrintError(int connfd, const char *info);

    static int *pipefd;
    TimerManager manager;
    static int epollfd;
    int timeslot;
};

#endif