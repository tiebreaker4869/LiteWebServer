#ifndef LWS_SQL_SQL_CONNECTION_POOL_H
#define LWS_SQL_SQL_CONNECTION_POOL_H

#include <list>
#include <mysql/mysql.h>
#include <string>

#include "lock/locker.h"
#include "sql/sql_config.h"

class SQLConnPool
{
public:
    MYSQL *GetConnection();

    bool ReleaseConnection(MYSQL *conn);

    int GetNumFreeConn() const;

    void DestroyPool();

    void Init(SQLConfig sql_config);

    static SQLConnPool *GetInstance();

    std::string url; // 主机地址
    std::string db_name;
    std::string username;
    std::string password;
    int port;

    int close_log; // 日志开关
private:
    SQLConnPool();
    ~SQLConnPool();

    int max_connection_;  // 最大连接数
    int cur_connection_;  // 已经使用的连接数
    int free_connection_; // 空闲连接数

    std::list<MYSQL *> pool_; // 连接池
    Mutex mutex_;
    Semaphore reserve_;
};

class ConnectionRAIIWrapper
{
public:
    ConnectionRAIIWrapper(MYSQL **conn, SQLConnPool *conn_pool);
    ~ConnectionRAIIWrapper();

private:
    MYSQL *conn_;
    SQLConnPool *pool_;
};

#endif