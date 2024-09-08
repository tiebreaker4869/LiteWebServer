#pragma once

#include <unordered_map>
#include <arpa/inet.h>

#include "httpconn.hpp"
#include "timer.hpp"
#include "epoll.hpp"
#include "thread_pool.hpp"
#include "sqlconnpool.hpp"

/**
 * @class WebServer
 * @brief Represents a web server that handles HTTP connections and requests.
 *
 * This class encapsulates the functionality of a web server, including socket initialization,
 * client connection management, request processing, and response handling.
 */
class WebServer
{
public:
    /**
     * @brief Constructs a WebServer object.
     * @param port The port number to listen on.
     * @param trig_mode The trigger mode for events.
     * @param timeout_ms The timeout in milliseconds.
     * @param opt_linger Option for lingering on socket close.
     * @param sql_port The SQL server port.
     * @param sql_user The SQL user name.
     * @param sql_pwd The SQL password.
     * @param db_name The database name.
     * @param conn_pool_num The number of connections in the pool.
     * @param thread_num The number of threads in the thread pool.
     * @param open_log Whether to open logging.
     * @param log_level The logging level.
     * @param log_queue_size The size of the log queue.
     */
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

    /**
     * @brief Destructs the WebServer object.
     */
    ~WebServer();

    /**
     * @brief Starts the web server.
     */
    void Run();

    /**
     * @brief Stops the web server.
     */
    void Stop();

private:
    /**
     * @brief Initializes the server socket.
     * @return true if successful, false otherwise.
     */
    bool InitSocket_();

    /**
     * @brief Initializes the event mode.
     * @param trig_mode The trigger mode for events.
     */
    void InitEventMode_(int trig_mode);

    /**
     * @brief Adds a client to the server.
     * @param fd The file descriptor of the client socket.
     * @param addr The address of the client.
     */
    void AddClient_(int fd, sockaddr_in addr);

    /**
     * @brief Handles incoming connections.
     */
    void DealListen_();

    /**
     * @brief Handles writing to a client.
     * @param client Pointer to the HttpConn object representing the client.
     */
    void DealWrite_(HttpConn *client);

    /**
     * @brief Handles reading from a client.
     * @param client Pointer to the HttpConn object representing the client.
     */
    void DealRead_(HttpConn *client);

    /**
     * @brief Extends the timeout for a client.
     * @param client Pointer to the HttpConn object representing the client.
     */
    void ExtendTime_(HttpConn *client);

    /**
     * @brief Closes the connection to a client.
     * @param client Pointer to the HttpConn object representing the client.
     */
    void CloseConn_(HttpConn *client);

    /**
     * @brief Handles the read event for a client.
     * @param client Pointer to the HttpConn object representing the client.
     */
    void OnRead_(HttpConn *client);

    /**
     * @brief Handles the write event for a client.
     * @param client Pointer to the HttpConn object representing the client.
     */
    void OnWrite_(HttpConn *client);

    /**
     * @brief Processes a client's request.
     * @param client Pointer to the HttpConn object representing the client.
     */
    void OnProcess_(HttpConn *client);

    /**
     * @brief The maximum number of file descriptors.
     */
    static const int kMaxFd = 1 << 16;

    /**
     * @brief Sets a file descriptor to non-blocking mode.
     * @param fd The file descriptor to set.
     * @return The result of the operation.
     */
    static int SetFdNonBlock_(int fd);

    int port_;         /**< The port number the server is listening on. */
    bool open_linger_; /**< Whether to use linger option on socket close. */
    int timeout_ms_;   /**< The timeout in milliseconds. */
    bool is_closed_;   /**< Whether the server is closed. */
    int listen_fd_;    /**< The file descriptor for the listening socket. */
    char *src_dir_;    /**< The source directory for serving files. */

    uint32_t listen_event_; /**< The event type for the listening socket. */
    uint32_t conn_event_;   /**< The event type for client connections. */

    std::unique_ptr<Timer> timer_;            /**< Timer for managing timeouts. */
    std::unique_ptr<ThreadPool> thread_pool_; /**< Thread pool for handling requests. */
    std::unique_ptr<Epoll> epoll_;            /**< Epoll instance for event handling. */
    std::unordered_map<int, HttpConn> users_; /**< Map of file descriptors to HttpConn objects. */
};