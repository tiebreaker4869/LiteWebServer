#pragma once

#include <cstddef>
#include <string>
#include <vector>
#include <atomic>

/**
 * @class Buffer
 * @brief A thread-safe buffer class for efficient reading and writing operations.
 *
 * This class provides a flexible buffer implementation with atomic read and write positions,
 * allowing for concurrent access in multi-threaded environments.
 */
class Buffer
{
public:
    /**
     * @brief Constructs a Buffer with a specified initial size.
     * @param buffer_size Initial size of the buffer (default: 1024 bytes).
     */
    explicit Buffer(int buffer_size = 1024);

    /**
     * @brief Default destructor.
     */
    ~Buffer() = default;

    /**
     * @brief Returns the number of bytes available for writing.
     * @return Size of writable space in bytes.
     */
    size_t WritableBytes() const;

    /**
     * @brief Returns the number of bytes available for reading.
     * @return Size of readable space in bytes.
     */
    size_t ReadableBytes() const;

    /**
     * @brief Returns the number of bytes that have been read.
     * @return Number of bytes read.
     */
    size_t ReadBytes() const;

    /**
     * @brief Initializes the read and write pointers.
     */
    void InitPtr();

    /**
     * @brief Returns a pointer to the current read position.
     * @return Const pointer to the current read position.
     */
    const char *ReadPtr() const;

    /**
     * @brief Returns a const pointer to the current write position.
     * @return Const pointer to the current write position.
     */
    const char *ConstWritePtr() const;

    /**
     * @brief Returns a pointer to the current write position.
     * @return Pointer to the current write position.
     */
    char *WritePtr();

    /**
     * @brief Updates the read pointer by a specified length.
     * @param len Number of bytes to move the read pointer.
     */
    void UpdateReadPtr(size_t len);

    /**
     * @brief Updates the read pointer to a specified end position.
     * @param end Pointer to the new end position for reading.
     */
    void UpdateReadPtrUntilEnd(const char *end);

    /**
     * @brief Retrieves all readable data as a string and resets the buffer.
     * @return String containing all readable data.
     */
    std::string RetrieveAllToStr();

    /**
     * @brief Ensures that the buffer has enough space for writing.
     * @param len Required writable length in bytes.
     */
    void EnsureWritable(size_t len);

    /**
     * @brief Updates the write pointer after data has been written.
     * @param len Number of bytes written.
     */
    void HasWritten(size_t len);

    /**
     * @brief Appends a string to the buffer.
     * @param str String to append.
     */
    void Append(const std::string &str);

    /**
     * @brief Appends a character array to the buffer.
     * @param str Pointer to the character array.
     * @param len Length of the array to append.
     */
    void Append(const char *str, size_t len);

    /**
     * @brief Appends raw data to the buffer.
     * @param data Pointer to the data.
     * @param len Length of the data to append.
     */
    void Append(const void *data, size_t len);

    /**
     * @brief Appends data from another Buffer object.
     * @param buffer Buffer object to append from.
     */
    void Append(const Buffer &buffer);

    /**
     * @brief Reads data from a file descriptor into the buffer.
     * @param fd File descriptor to read from.
     * @param save_errno Pointer to store the errno in case of an error.
     * @return Number of bytes read, or -1 on error.
     */
    ssize_t ReadFd(int fd, int *save_errno);

    /**
     * @brief Writes data from the buffer to a file descriptor.
     * @param fd File descriptor to write to.
     * @param save_errno Pointer to store the errno in case of an error.
     * @return Number of bytes written, or -1 on error.
     */
    ssize_t WriteFd(int fd, int *save_errno);

private:
    /**
     * @brief Returns a pointer to the beginning of the buffer.
     * @return Pointer to the beginning of the buffer.
     */
    char *BeginPtr_();

    /**
     * @brief Returns a const pointer to the beginning of the buffer.
     * @return Const pointer to the beginning of the buffer.
     */
    const char *ConstBeginPtr_() const;

    /**
     * @brief Allocates more space in the buffer.
     * @param len Additional length required.
     */
    void AllocSpace_(size_t len);

    std::vector<char> buffer_;              ///< Underlying storage for the buffer.
    std::atomic<std::size_t> read_pos_{0};  ///< Current read position (atomic for thread-safety).
    std::atomic<std::size_t> write_pos_{0}; ///< Current write position (atomic for thread-safety).
};