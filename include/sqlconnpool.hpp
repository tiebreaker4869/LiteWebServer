#pragma once

#include <mysql/mysql.h>
#include <string>
#include <queue>
#include <mutex>
#include <semaphore.h>
#include <cassert>

class SqlConnPool
{
public:
    static SqlConnPool *GetInstance();
    // get connection from pool
    MYSQL *GetConn();
    void *FreeConn(MYSQL *conn);
    int GetFreeConnCount() const;

    void Init(const char *host, int port,
              const char *user, const char *pwd, const char *db_name,
              int conn_cnt);
    void ClosePool();

private:
    SqlConnPool() = default;
    ~SqlConnPool();

    int max_conn_;
    int used_cnt_{0};
    int free_cnt_{0};

    std::queue<MYSQL *> conn_que_; // ready queue
    mutable std::mutex mtx_;
    sem_t sem_id_;
};

class SqlConn
{
public:
    SqlConn(MYSQL **sql, SqlConnPool *conn_pool)
    {
        assert(conn_pool != nullptr);
        *sql = conn_pool->GetConn();
        sql_ = *sql;
        pool_ = conn_pool;
    }

    ~SqlConn()
    {
        if (sql_)
        {
            pool_->FreeConn(sql_);
        }
    }

private:
    MYSQL *sql_;
    SqlConnPool *pool_;
};