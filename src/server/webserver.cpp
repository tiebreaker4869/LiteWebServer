#include "webserver.hpp"
#include "log.hpp"
#include <memory>
#include <unistd.h>
#include <cstring>
#include <sys/epoll.h>
#include <fcntl.h>

WebServer::WebServer(int port, int trig_mode,
                     int timeout_ms, bool opt_linger,
                     int sql_port, const char *sql_user,
                     const char *sql_pwd,
                     const char *db_name,
                     int conn_pool_num,
                     int thread_num,
                     bool open_log,
                     int log_level,
                     int log_queue_size) : port_(port),
                                           open_linger_(opt_linger),
                                           timeout_ms_(timeout_ms),
                                           is_closed_(false),
                                           timer_{std::make_unique<Timer>()},
                                           thread_pool_{std::make_unique<ThreadPool>(thread_num)},
                                           epoll_(std::make_unique<Epoll>())
{
    src_dir_ = getcwd(nullptr, 256);
    assert(src_dir_);
    strncat(src_dir_, "/../resources/", 16);
    HttpConn::user_count = 0;
    HttpConn::src_dir = src_dir_;

    // 初始化日志
    if (open_log)
    {
        Log::GetInstance()->Init(log_level, "./log", ".log", log_queue_size);
        if (is_closed_)
        {
            LOG_ERROR("========== Server init error!==========");
        }
        else
        {
            LOG_INFO("========== Server init ==========");
            LOG_INFO("Port: %d", port_);
            LOG_INFO("Listen Mode: %s", listen_event_ & EPOLLET ? "ET" : "LT");
            LOG_INFO("Connect Mode: %s", conn_event_ & EPOLLET ? "ET" : "LT");
            LOG_INFO("Log Level: %d", log_level);
            LOG_INFO("src dir: %s", src_dir_);
            LOG_INFO("Sql Connection Pool Number: %d", conn_pool_num);
            LOG_INFO("Thread Number: %d", thread_num);
        }

        InitEventMode_(trig_mode);
        if (!InitSocket_())
        {
            is_closed_ = true;
            LOG_ERROR("Init Socket Error!");
            exit(1);
        }
        LOG_INFO("========== Create Sql Connection Pool ==========");
        SqlConnPool::GetInstance()->Init("localhost", sql_port,
                                         sql_user, sql_pwd,
                                         db_name, conn_pool_num);
    }
}

void WebServer::Stop()
{
    close(listen_fd_);
    is_closed_ = true;
    free(src_dir_);
    SqlConnPool::GetInstance()->ClosePool();
}

WebServer::~WebServer() = default;

void WebServer::InitEventMode_(int trig_mode)
{
    // EVENTRDHUP can trigger an event when the peer is closed
    // EPOLLONESHOT disables checking after event notification is completed
    listen_event_ = EPOLLRDHUP;
    conn_event_ = EPOLLONESHOT | EPOLLRDHUP;

    // set trigger mode, EPOLLET is edge trigger, EPOLLLT is level trigger
    // default level trigger
    switch (trig_mode)
    {
    case 1:
        conn_event_ |= EPOLLET;
        break;
    case 2:
        listen_event_ |= EPOLLET;
        break;
    case 3:
        listen_event_ |= EPOLLET;
        conn_event_ |= EPOLLET;
        break;
    default:
        break;
    }
    HttpConn::isET = (conn_event_ & EPOLLET);
}

int WebServer::SetFdNonBlock_(int fd)
{
    return fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0) | O_NONBLOCK);
}

bool WebServer::InitSocket_()
{
    int ret;
    struct sockaddr_in addr;
    if (port_ > 65535 || port_ < 1024)
    {
        // why: 1024 ~ 65535 is the port range for user applications
        LOG_ERROR("Port: %d error!", port_);
        return false;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port_);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    struct linger opt_linger = {0};
    if (open_linger_)
    {
        // until the remaining data is sent or timeout
        opt_linger.l_onoff = 1;
        opt_linger.l_linger = 1;
    }
    // set TCP connection to be gracefully closed or rudely
    ret = setsockopt(listen_fd_, SOL_SOCKET, SO_LINGER, &opt_linger, sizeof(opt_linger));
    if (ret < 0)
    {
        close(listen_fd_);
        LOG_ERROR("Init setsockopt error!", port_);
        return false;
    }

    // port multiplexing
    // only the last socket will receive data normally
    int on = 1;
    ret = setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    if (ret < 0)
    {
        LOG_ERROR("Init setsockopt error!", port_);
        close(listen_fd_);
        return false;
    }

    ret = bind(listen_fd_, (struct sockaddr *)&addr, sizeof(addr));
    if (ret < 0)
    {
        LOG_ERROR("Bind Port: %d error!", port_);
        close(listen_fd_);
        return false;
    }

    ret = listen(listen_fd_, 6);
    if (ret < 0)
    {
        LOG_ERROR("Listen Port: %d error!", port_);
        close(listen_fd_);
        return false;
    }

    ret = epoll_->AddFd(listen_fd_, listen_event_ | EPOLLIN);
    if (ret == 0)
    {
        LOG_ERROR("Add listen error!");
        close(listen_fd_);
        return false;
    }
    SetFdNonBlock_(listen_fd_);
    LOG_INFO("Server init success!");

    return true;
}

void WebServer::Run()
{
    int time_ms = -1; // -1 means no timeout
    if (!is_closed_)
    {
        LOG_INFO("========== Server start ==========");
    }
    while (!is_closed_)
    {
        if (timeout_ms_ > 0)
        {
            time_ms = timer_->GetNextTick();
        }
        int event_cnt = epoll_->Wait(time_ms);
        for (int i = 0; i < event_cnt; i++)
        {
            {
                // deal with event
                int fd = epoll_->GetEventFd(i);
                uint32_t events = epoll_->GetEvents(i);
                if (fd == listen_fd_)
                {
                    DealListen_();
                }
                else if (events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR))
                {
                    CloseConn_(&users_[fd]);
                }
                else if (events & EPOLLIN)
                {
                    DealRead_(&users_[fd]);
                }
                else if (events & EPOLLOUT)
                {
                    DealWrite_(&users_[fd]);
                }
                else
                {
                    LOG_ERROR("Unexpected event");
                }
            }
        }
    }
}

void WebServer::CloseConn_(HttpConn *client)
{
    LOG_INFO("Client[%d] quit!", client->GetFd());
    epoll_->DeleteFd(client->GetFd());
    client->Close();
}

void WebServer::AddClient_(int fd, sockaddr_in addr)
{
    assert(fd > 0);
    users_[fd].Init(fd, addr);
    if (timeout_ms_ > 0)
    {
        timer_->AddTimer(fd, timeout_ms_,
                         std::bind(&WebServer::CloseConn_, this, &users_[fd]));
    }
    epoll_->AddFd(fd, EPOLLIN | conn_event_);
    SetFdNonBlock_(fd);
    LOG_INFO("Client[%d] in!", fd);
}

void WebServer::DealListen_()
{
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);

    do
    {
        int fd = accept(listen_fd_, (struct sockaddr *)&addr, &len);
        if (fd <= 0)
        {
            return;
        }
        else if (HttpConn::user_count >= kMaxFd)
        {
            LOG_WARN("Server is busy!");
            return;
        }
        AddClient_(fd, addr);
    } while (listen_event_ & EPOLLET);
}

void WebServer::DealRead_(HttpConn *client)
{
    assert(client);
    ExtendTime_(client);
    thread_pool_->AddTask(std::bind(&WebServer::OnRead_, this, client));
}

void WebServer::DealWrite_(HttpConn *client)
{
    assert(client);
    ExtendTime_(client);
    thread_pool_->AddTask(std::bind(&WebServer::OnWrite_, this, client));
}

void WebServer::ExtendTime_(HttpConn *client)
{
    if (timeout_ms_ > 0)
    {
        timer_->Adjust(client->GetFd(), timeout_ms_);
    }
}

void WebServer::OnRead_(HttpConn *client)
{
    assert(client);
    int ret = -1;
    int read_errno = 0;
    ret = client->Read(&read_errno);
    if (ret <= 0 && read_errno != EAGAIN)
    {
        CloseConn_(client);
        return;
    }
    OnProcess_(client);
}

void WebServer::OnWrite_(HttpConn *client)
{
    assert(client);
    int ret = -1;
    int write_errno = 0;
    ret = client->Write(&write_errno);
    if (client->ToWriteBytes() == 0)
    {
        // transfer complete
        if (client->IsKeepAlive())
        {
            OnProcess_(client);
            return;
        }
    }
    else if (ret < 0)
    {
        if (write_errno == EAGAIN)
        {
            // continue transfer
            epoll_->ModifyFd(client->GetFd(), conn_event_ | EPOLLOUT);
            return;
        }
    }
    CloseConn_(client);
}

void WebServer::OnProcess_(HttpConn *client)
{
    if (client->Handle())
    {
        epoll_->ModifyFd(client->GetFd(), conn_event_ | EPOLLOUT);
    }
    else
    {
        epoll_->ModifyFd(client->GetFd(), conn_event_ | EPOLLIN);
    }
}