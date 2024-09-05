#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>
#include <sys/epoll.h>

/**
 * @class Epoll
 * @brief A wrapper class for the Linux epoll API, providing event-driven I/O functionality.
 *
 * This class encapsulates the epoll system calls and provides a simple interface
 * for managing file descriptors and handling I/O events.
 */
class Epoll
{
public:
    /**
     * @brief Constructs an Epoll object with a specified maximum number of events.
     * @param max_event The maximum number of events to handle (default: 4096).
     */
    explicit Epoll(int max_event = 4096);

    /**
     * @brief Destructor that closes the epoll file descriptor.
     */
    ~Epoll();

    /**
     * @brief Adds a file descriptor to the epoll instance.
     * @param fd The file descriptor to add.
     * @param events The events to monitor for this file descriptor.
     * @return true if the operation was successful, false otherwise.
     */
    bool AddFd(int fd, uint32_t events);

    /**
     * @brief Modifies the events associated with a file descriptor in the epoll instance.
     * @param fd The file descriptor to modify.
     * @param events The new set of events to monitor for this file descriptor.
     * @return true if the operation was successful, false otherwise.
     */
    bool ModifyFd(int fd, uint32_t events);

    /**
     * @brief Removes a file descriptor from the epoll instance.
     * @param fd The file descriptor to remove.
     * @param events The events that were being monitored (unused in most implementations).
     * @return true if the operation was successful, false otherwise.
     */
    bool DeleteFd(int fd, uint32_t events);

    /**
     * @brief Waits for events on the epoll instance.
     * @param timeout_ms The maximum time to wait in milliseconds (-1 for infinite).
     * @return The number of file descriptors ready for the requested I/O, or -1 on error.
     */
    int Wait(int timeout_ms = -1);

    /**
     * @brief Retrieves the file descriptor for a ready event.
     * @param i The index of the event in the internal events array.
     * @return The file descriptor associated with the event.
     */
    int GetEventFd(size_t i) const;

    /**
     * @brief Retrieves the events that occurred for a ready file descriptor.
     * @param i The index of the event in the internal events array.
     * @return The events that occurred for the file descriptor.
     */
    uint32_t GetEvents(size_t i) const;

private:
    int epoll_fd_;                           ///< The file descriptor for the epoll instance.
    std::vector<struct epoll_event> events_; ///< Vector to store epoll events.
};