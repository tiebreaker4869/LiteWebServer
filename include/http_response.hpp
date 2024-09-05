#pragma once

#include "buffer.hpp"
#include <sys/stat.h>
#include <unordered_map>

/**
 * @class HttpResponse
 * @brief Represents and handles an HTTP response.
 *
 * This class is responsible for generating and managing HTTP responses,
 * including status lines, headers, and content.
 */
class HttpResponse
{
public:
    /**
     * @brief Default constructor.
     */
    HttpResponse();

    /**
     * @brief Destructor.
     */
    ~HttpResponse();

    /**
     * @brief Initializes the HttpResponse object.
     * @param src_dir The source directory for files.
     * @param path The path of the requested resource.
     * @param is_keep_alive Whether to keep the connection alive.
     * @param code The HTTP status code.
     */
    void Init(const std::string &src_dir, std::string &path, bool is_keep_alive = false, int code = -1);

    /**
     * @brief Generates the complete HTTP response.
     * @param buff The buffer to store the response.
     */
    void MakeResponse(Buffer &buff);

    /**
     * @brief Unmaps the file from memory.
     */
    void UnmapFile();

    /**
     * @brief Gets the pointer to the file content.
     * @return Pointer to the file content.
     */
    char *File();

    /**
     * @brief Gets the length of the file.
     * @return Length of the file.
     */
    size_t FileLen() const;

    /**
     * @brief Generates error content for the response.
     * @param buff The buffer to store the error content.
     * @param message The error message.
     */
    void ErrorContent(Buffer &buff, std::string message);

    /**
     * @brief Gets the HTTP status code.
     * @return The HTTP status code.
     */
    int Code() const { return code_; }

private:
    /**
     * @brief Adds the status line to the response.
     * @param buff The buffer to store the status line.
     */
    void AddStatusLine_(Buffer &buff);

    /**
     * @brief Adds headers to the response.
     * @param buff The buffer to store the headers.
     */
    void AddHeader_(Buffer &buff);

    /**
     * @brief Adds content to the response.
     * @param buff The buffer to store the content.
     */
    void AddContent_(Buffer &buff);

    /**
     * @brief Generates HTML for error responses.
     */
    void ErrorHtml_();

    /**
     * @brief Determines the file type based on the file extension.
     * @return The MIME type of the file.
     */
    std::string GetFileType_();

    int code_;           ///< HTTP status code
    bool is_keep_alive_; ///< Whether to keep the connection alive

    std::string path_;    ///< Path of the requested resource
    std::string src_dir_; ///< Source directory for files

    char *file_;             ///< Pointer to the file content
    struct stat file_state_; ///< File status information

    static const std::unordered_map<std::string, std::string> kSuffixType; ///< Map of file extensions to MIME types
    static const std::unordered_map<int, std::string> kCodeStatus;         ///< Map of status codes to status messages
    static const std::unordered_map<int, std::string> kCodePath;           ///< Map of status codes to error page paths
};