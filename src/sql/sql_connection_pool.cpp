#include "sql_connection_pool.h"
#include "lock/locker.h"
#include "log/log.h"

SQLConnPool::SQLConnPool()
{
    cur_connection_ = 0;
    free_connection_ = 0;
}

SQLConnPool *SQLConnPool::GetInstance()
{
    static SQLConnPool pool;
    return &pool;
}

void SQLConnPool::Init(SQLConfig config)
{
    this->url = config.url;
    this->db_name = config.db_name;
    this->username = config.username;
    this->password = config.password;
    this->port = config.port;
    this->close_log = config.close_log;
    this->max_connection_ = config.max_connection;

    for (int i = 0; i < max_connection_; i++)
    {
        MYSQL *conn = mysql_init(nullptr);

        if (!conn)
        {
            // log error info
            LOG_ERROR("MySQL init error");
            exit(1);
        }

        conn = mysql_real_connect(conn, url.c_str(), username.c_str(), password.c_str(), db_name.c_str(), port, nullptr, 0);

        if (!conn)
        {
            // log error info
            LOG_ERROR("MySQL connect error");
            exit(1);
        }

        pool_.push_back(conn);
    }

    free_connection_ = max_connection_;
    reserve_ = Semaphore(free_connection_);
}

// 从数据库连接池中拿出一个空闲连接
MYSQL *SQLConnPool::GetConnection()
{
    MYSQL *conn = nullptr;

    if (pool_.size() == 0)
    {
        return nullptr;
    }

    // 等待空闲连接
    reserve_.Wait();

    // 独占访问连接池
    LockGuard lock(mutex_);

    conn = pool_.front();
    pool_.pop_front();

    --free_connection_;
    ++cur_connection_;

    return conn;
}

// 归还连接到数据库连接池
bool SQLConnPool::ReleaseConnection(MYSQL *conn)
{
    if (!conn)
    {
        return false;
    }

    {
        LockGuard lock(mutex_);
        pool_.push_back(conn);
        ++free_connection_;
        --cur_connection_;
    }

    reserve_.Post();
    return true;
}

// 释放所有连接
void SQLConnPool::DestroyPool()
{
    LockGuard lock(mutex_);

    if (!pool_.empty())
    {
        for (auto it = pool_.begin(); it != pool_.end(); it++)
        {
            MYSQL *conn = *it;
            mysql_close(conn);
        }

        cur_connection_ = 0;
        free_connection_ = 0;
        pool_.clear();
    }
}

int SQLConnPool::GetNumFreeConn() const
{
    return free_connection_;
}

SQLConnPool::~SQLConnPool()
{
    DestroyPool();
}

ConnectionRAIIWrapper::ConnectionRAIIWrapper(MYSQL **conn, SQLConnPool *conn_pool)
{
    *conn = conn_pool->GetConnection();
    conn_ = *conn;
    pool_ = conn_pool;
}

ConnectionRAIIWrapper::~ConnectionRAIIWrapper()
{
    pool_->ReleaseConnection(conn_);
}