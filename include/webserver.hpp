#pragma once

#include <unordered_map>
#include <arpa/inet.h>

#include "httpconn.hpp"
#include "timer.hpp"
#include "epoll.hpp"
#include "thread_pool.hpp"
#include "sqlconnpool.hpp"

class WebServer
{
public:
    WebServer(int port,
              int trig_mode,
              int timeout_ms,
              bool opt_linger,
              int sql_port,
              const char *sql_user,
              const char *sql_pwd,
              const char *db_name,
              int conn_pool_num,
              int thread_num,
              bool open_log,
              int log_level,
              int log_queue_size);
    ~WebServer();
    void Run();
    void Stop();

private:
    bool InitSocket_();
    void InitEventMode_(int trig_mode);
    void AddClient_(int fd, sockaddr_in addr);

    void DealListen_();
    void DealWrite_(HttpConn *client);
    void DealRead_(HttpConn *client);

    void ExtendTime_(HttpConn *client);
    void CloseConn_(HttpConn *client);

    void OnRead_(HttpConn *client);
    void OnWrite_(HttpConn *client);
    void OnProcess_(HttpConn *client);

    static const int kMaxFd = 1 << 16;

    static int SetFdNonBlock_(int fd);

    int port_;
    bool open_linger_;
    int timeout_ms_;
    bool is_closed_;
    int listen_fd_;
    char *src_dir_;

    uint32_t listen_event_;
    uint32_t conn_event_;

    std::unique_ptr<Timer> timer_;
    std::unique_ptr<ThreadPool> thread_pool_;
    std::unique_ptr<Epoll> epoll_;
    std::unordered_map<int, HttpConn> users_;
};