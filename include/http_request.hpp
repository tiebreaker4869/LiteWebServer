#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>

#include "buffer.hpp"

class HttpRequest
{
public:
    enum ParseState
    {
        kRequestLine,
        kHeader,
        kBody,
        kFinish
    };
    enum HttpCode
    {
        kNoRequest,
        kGetRequest,
        kBadRequest,
        kNoResponse,
        kForbiddenedRequest,
        kFileRequest,
        kInternalError,
        kClosedConnection
    };

    HttpRequest() = default;
    ~HttpRequest() = default;

    void Init();
    bool Parse(Buffer &buffer);

    bool IsKeepAlive() const;
    std::string Path() const;
    std::string &Path();
    std::string Method() const;
    std::string Version() const;
    std::string GetPostValueByKey(const std::string &key) const;

private:
    bool ParseRequestLine_(const std::string &line);
    void ParseHeader_(const std::string &line);
    void ParseBody_(const std::string &line);

    void ParsePath_();
    void ParsePost_();
    void ParseFromURLEncoded_();
    void ParseKeyValue_(const std::string &line);

    static bool UserVerify(const std::string &name, const std::string &passwd, bool is_login);

    ParseState state_;
    std::string method_;
    std::string path_;
    std::string version_;
    std::string body_;

    std::unordered_map<std::string, std::string> header_;
    std::unordered_map<std::string, std::string> post_;

    static const std::unordered_set<std::string> kDefaultHTML;
    static const std::unordered_map<std::string, int> kDefaultHTMLTag;
    static int ConvertHex_(char ch);
};