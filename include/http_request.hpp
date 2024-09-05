#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>

#include "buffer.hpp"

/**
 * @class HttpRequest
 * @brief Represents and handles an HTTP request.
 *
 * This class is responsible for parsing, storing, and providing access to
 * various components of an HTTP request, including the request line, headers,
 * and body.
 */
class HttpRequest
{
public:
    /**
     * @enum ParseState
     * @brief Represents the current state of request parsing.
     */
    enum ParseState
    {
        kRequestLine, ///< Parsing the request line
        kHeader,      ///< Parsing headers
        kBody,        ///< Parsing the body
        kFinish       ///< Parsing is complete
    };

    /**
     * @enum HttpCode
     * @brief Represents various HTTP response codes and states.
     */
    enum HttpCode
    {
        kNoRequest,          ///< No request to process
        kGetRequest,         ///< GET request
        kBadRequest,         ///< Bad request
        kNoResponse,         ///< No response
        kForbiddenedRequest, ///< Forbidden request
        kFileRequest,        ///< File request
        kInternalError,      ///< Internal server error
        kClosedConnection    ///< Connection closed
    };

    HttpRequest() = default;
    ~HttpRequest() = default;

    /**
     * @brief Initializes the HttpRequest object.
     */
    void Init();

    /**
     * @brief Parses the HTTP request from the given buffer.
     * @param buffer The buffer containing the HTTP request.
     * @return true if parsing was successful, false otherwise.
     */
    bool Parse(Buffer &buffer);

    /**
     * @brief Checks if the connection should be kept alive.
     * @return true if the connection should be kept alive, false otherwise.
     */
    bool IsKeepAlive() const;

    /**
     * @brief Gets the request path.
     * @return The request path as a string.
     */
    std::string Path() const;

    /**
     * @brief Gets a reference to the request path.
     * @return A reference to the request path.
     */
    std::string &Path();

    /**
     * @brief Gets the HTTP method.
     * @return The HTTP method as a string.
     */
    std::string Method() const;

    /**
     * @brief Gets the HTTP version.
     * @return The HTTP version as a string.
     */
    std::string Version() const;

    /**
     * @brief Gets a POST value by its key.
     * @param key The key to look up.
     * @return The value associated with the key.
     */
    std::string GetPostValueByKey(const std::string &key) const;

private:
    bool ParseRequestLine_(const std::string &line);
    void ParseHeader_(const std::string &line);
    void ParseBody_(const std::string &line);

    void ParsePath_();
    void ParsePost_();
    void ParseFromURLEncoded_();
    void ParseKeyValue_(const std::string &line);

    /**
     * @brief Verifies user credentials.
     * @param name The username.
     * @param passwd The password.
     * @param is_login Whether this is a login attempt.
     * @return true if verification succeeds, false otherwise.
     */
    static bool UserVerify(const std::string &name, const std::string &passwd, bool is_login);

    ParseState state_;
    std::string method_;
    std::string path_;
    std::string version_;
    std::string body_;

    std::unordered_map<std::string, std::string> header_;
    std::unordered_map<std::string, std::string> post_;

    static const std::unordered_set<std::string> kDefaultHtml;
    static const std::unordered_map<std::string, int> kDefaultHtmlTag;
};