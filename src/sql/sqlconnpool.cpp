#include "sqlconnpool.hpp"
#include "log.hpp"

#include <cassert>
#include <mutex>

SqlConnPool *SqlConnPool::GetInstance()
{
    static SqlConnPool conn_pool;
    return &conn_pool;
}

void SqlConnPool::Init(const char *host, int port,
                       const char *user, const char *pwd,
                       const char *db_name, int conn_cnt)
{
    assert(conn_cnt > 0);
    // create conn_cnt connections
    for (int i = 0; i < conn_cnt; i++)
    {
        MYSQL *sql = nullptr;
        sql = mysql_init(sql);
        if (!sql)
        {
            LOG_ERROR("MySql init error!");
            assert(sql);
        }

        if (!mysql_real_connect(sql, host, user, pwd, db_name, port, nullptr, 0))
        {
            LOG_ERROR("MySql Connect error!");
            LOG_ERROR("Error: %s", mysql_error(sql));
            continue;
        }
        LOG_INFO("MySQL conn: %d Connected!", i);
        conn_que_.push(sql);
    }
    max_conn_ = conn_cnt;
    sem_init(&sem_id_, 0, max_conn_);
}

MYSQL *SqlConnPool::GetConn()
{
    MYSQL *sql = nullptr;
    sem_wait(&sem_id_);
    {
        std::lock_guard<std::mutex> lock{mtx_};
        sql = conn_que_.front();
        conn_que_.pop();
    }
    return sql;
}

void SqlConnPool::FreeConn(MYSQL *sql)
{
    assert(sql);
    std::lock_guard<std::mutex> lock{mtx_};
    conn_que_.push(sql);
    sem_post(&sem_id_);
}

void SqlConnPool::ClosePool()
{
    std::lock_guard<std::mutex> lock{mtx_};
    // close all connections
    while (!conn_que_.empty())
    {
        MYSQL *sql = conn_que_.front();
        mysql_close(sql);
        conn_que_.pop();
    }
    mysql_library_end();
}

int SqlConnPool::GetFreeConnCount() const
{
    std::lock_guard<std::mutex> lock{mtx_};
    return conn_que_.size();
}

SqlConnPool::~SqlConnPool()
{
    ClosePool();
}