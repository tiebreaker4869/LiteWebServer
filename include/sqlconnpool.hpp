#pragma once

#include <mysql/mysql.h>
#include <string>
#include <queue>
#include <mutex>
#include <semaphore.h>
#include <cassert>

/**
 * @class SqlConnPool
 * @brief A singleton class that manages a pool of MySQL database connections.
 *
 * This class implements a connection pool to efficiently manage and reuse
 * MySQL database connections. It provides thread-safe methods for obtaining
 * and releasing connections, as well as initializing and closing the pool.
 */
class SqlConnPool
{
public:
    /**
     * @brief Get the singleton instance of SqlConnPool.
     * @return Pointer to the SqlConnPool instance.
     */
    static SqlConnPool *GetInstance();

    /**
     * @brief Get a connection from the pool.
     * @return Pointer to a MYSQL connection.
     */
    MYSQL *GetConn();

    /**
     * @brief Return a connection to the pool.
     * @param conn Pointer to the MYSQL connection to be returned.
     */
    void FreeConn(MYSQL *conn);

    /**
     * @brief Get the number of free connections in the pool.
     * @return The count of free connections.
     */
    int GetFreeConnCount() const;

    /**
     * @brief Initialize the connection pool.
     * @param host MySQL server host.
     * @param port MySQL server port.
     * @param user MySQL user name.
     * @param pwd MySQL user password.
     * @param db_name Database name.
     * @param conn_cnt Number of connections to create in the pool.
     */
    void Init(const char *host, int port,
              const char *user, const char *pwd, const char *db_name,
              int conn_cnt);

    /**
     * @brief Close all connections and destroy the pool.
     */
    void ClosePool();

private:
    SqlConnPool() = default;
    ~SqlConnPool();

    int max_conn_;    ///< Maximum number of connections in the pool
    int used_cnt_{0}; ///< Number of connections currently in use
    int free_cnt_{0}; ///< Number of free connections

    std::queue<MYSQL *> conn_que_; ///< Queue of available connections
    mutable std::mutex mtx_;       ///< Mutex for thread-safe operations
    sem_t sem_id_;                 ///< Semaphore for connection availability
};

/**
 * @class SqlConn
 * @brief RAII wrapper for SqlConnPool connections.
 *
 * This class provides a convenient way to automatically obtain a connection
 * from the pool upon construction and return it upon destruction, ensuring
 * proper resource management.
 */
class SqlConn
{
public:
    /**
     * @brief Constructor that obtains a connection from the pool.
     * @param sql Pointer to a MYSQL pointer that will receive the connection.
     * @param conn_pool Pointer to the SqlConnPool instance.
     */
    SqlConn(MYSQL **sql, SqlConnPool *conn_pool)
    {
        assert(conn_pool != nullptr);
        *sql = conn_pool->GetConn();
        sql_ = *sql;
        pool_ = conn_pool;
    }

    /**
     * @brief Destructor that returns the connection to the pool.
     */
    ~SqlConn()
    {
        if (sql_)
        {
            pool_->FreeConn(sql_);
        }
    }

private:
    MYSQL *sql_;        ///< The MYSQL connection
    SqlConnPool *pool_; ///< Pointer to the SqlConnPool instance
};