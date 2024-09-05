#pragma once

#include <arpa/inet.h>
#include <atomic>
#include <sys/uio.h>
#include "buffer.hpp"
#include "http_request.hpp"
#include "http_response.hpp"

/**
 * @class HttpConn
 * @brief Represents an HTTP connection and handles its lifecycle.
 *
 * This class manages the state and operations of an individual HTTP connection,
 * including reading requests, writing responses, and maintaining connection information.
 */
class HttpConn
{
public:
    /**
     * @brief Default constructor.
     */
    HttpConn();

    /**
     * @brief Destructor.
     */
    ~HttpConn();

    /**
     * @brief Initializes the HTTP connection with a socket file descriptor and address.
     * @param sockfd The socket file descriptor.
     * @param addr The socket address.
     */
    void Init(int sockfd, const sockaddr_in &addr);

    /**
     * @brief Reads data from the connection.
     * @param save_errno Pointer to store the error number if an error occurs.
     * @return The number of bytes read, or -1 on error.
     */
    ssize_t Read(int *save_errno);

    /**
     * @brief Writes data to the connection.
     * @param save_errno Pointer to store the error number if an error occurs.
     * @return The number of bytes written, or -1 on error.
     */
    ssize_t Write(int *save_errno);

    /**
     * @brief Closes the connection.
     */
    void Close();

    /**
     * @brief Processes the HTTP connection, parsing requests and generating responses.
     * @return true if the connection should be kept alive, false otherwise.
     */
    bool Handle();

    /**
     * @brief Gets the file descriptor of the connection.
     * @return The file descriptor.
     */
    int GetFd() const;

    /**
     * @brief Gets the port number of the connection.
     * @return The port number.
     */
    int GetPort() const;

    /**
     * @brief Gets the IP address of the connection.
     * @return The IP address as a string.
     */
    const char *GetIP() const;

    /**
     * @brief Gets the socket address of the connection.
     * @return The socket address.
     */
    sockaddr_in GetAddr() const;

    /**
     * @brief Calculates the total number of bytes that need to be written.
     * @return The total number of bytes to write.
     */
    int ToWriteBytes() const
    {
        return iov_[0].iov_len + iov_[1].iov_len;
    }

    /**
     * @brief Checks if the connection should be kept alive.
     * @return true if the connection should be kept alive, false otherwise.
     */
    bool IsKeepAlive() const
    {
        return request_.IsKeepAlive();
    }

    /** @brief Flag indicating whether Edge Triggered mode is used. */
    static bool isET;

    /** @brief The source directory for serving files. */
    static const char *src_dir;

    /** @brief The total number of active HTTP connections. */
    static std::atomic<int> user_count;

private:
    int fd_;              /**< The file descriptor for this HTTP connection. */
    bool is_close_;       /**< Flag indicating whether the connection is closed. */
    struct sockaddr_in addr_; /**< The socket address of the connection. */
    int iov_cnt_;         /**< The count of I/O vectors. */
    struct iovec iov_[2]; /**< I/O vectors for scatter/gather I/O. */
    Buffer read_buff_;    /**< Buffer for reading data. */
    Buffer write_buff_;   /**< Buffer for writing data. */
    HttpRequest request_; /**< The HTTP request object. */
    HttpResponse response_; /**< The HTTP response object. */
};