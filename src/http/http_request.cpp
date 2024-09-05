#include "http_request.hpp"
#include <regex>
#include <cassert>
#include <algorithm>

#include "log.hpp"
#include "sqlconnpool.hpp"

const std::unordered_set<std::string> HttpRequest::kDefaultHtml = {
    "/index",
    "/register",
    "/login",
    "/welcome",
    "/video",
    "/picture",
};

const std::unordered_map<std::string, int> HttpRequest::kDefaultHtmlTag = {
    {"/register.html", 0},
    {"/login.html", 1},
};

void HttpRequest::Init()
{
    method_ = "";
    path_ = "";
    version_ = "";
    body_ = "";
    state_ = HttpRequest::ParseState::kRequestLine;
    header_.clear();
    post_.clear();
}

bool HttpRequest::IsKeepAlive() const
{
    if (header_.find("Connection") != header_.end())
    {
        return header_.find("Connection")->second == "keep-alive" && version_ == "HTTP/1.1";
    }
    return false;
}

std::string &HttpRequest::Path()
{
    return path_;
}

std::string HttpRequest::Path() const
{
    return path_;
}

std::string HttpRequest::Method() const
{
    return method_;
}

std::string HttpRequest::Version() const
{
    return version_;
}

std::string HttpRequest::GetPostValueByKey(const std::string &key) const
{
    assert(key != "");
    if (post_.find(key) != post_.end())
    {
        return post_.find(key)->second;
    }
    return "";
}

bool HttpRequest::ParseRequestLine_(const std::string &line)
{
    LOG_DEBUG("%s", line.c_str());
    std::regex patten("^([^ ]*) ([^ ]*) HTTP/([^ ]*)$");
    std::smatch submatch;
    if (std::regex_match(line, submatch, patten))
    {
        method_ = submatch[1];
        path_ = submatch[2];
        version_ = submatch[3];
        state_ = ParseState::kHeader;
        return true;
    }
    LOG_ERROR("RequestLine Error");
    return false;
}

void HttpRequest::ParseHeader_(const std::string &line)
{
    std::regex patten("^([^:]*): ?(.*)$");
    std::smatch submatch;
    if (std::regex_match(line, submatch, patten))
    {
        header_[submatch[1]] = submatch[2];
    }
    else
    {
        state_ = ParseState::kBody;
    }
}

void HttpRequest::ParseBody_(const std::string &line)
{
    body_ = line;
    ParsePost_();
    state_ = ParseState::kFinish;
    LOG_DEBUG("Body:%s, len:%d", line.c_str(), line.size());
}

void HttpRequest::ParsePost_()
{
    if (method_ == "POST" && header_["Content-Type"] == "application/x-www-form-urlencoded")
    {
        ParseFromURLEncoded_();
        if (kDefaultHtmlTag.count(path_))
        {
            int tag = kDefaultHtmlTag.find(path_)->second;
            LOG_DEBUG("Tag:%d", tag);
            if (tag == 0 || tag == 1)
            {
                bool is_login = (tag == 1);
                if (UserVerify(post_["username"], post_["password"], is_login))
                {
                    path_ = "/welcome.html";
                }
                else
                {
                    path_ = "/error.html";
                }
            }
        }
    }
}

void HttpRequest::ParsePath_()
{
    if (path_ == "/")
    {
        path_ = "/index.html";
    }
    else if (kDefaultHtml.count(path_))
    {
        path_ += ".html";
    }
}

void HttpRequest::ParseKeyValue_(const std::string &line)
{
    size_t pos = line.find_first_of("=");
    post_[line.substr(0, pos)] = line.substr(pos + 1);
}

void HttpRequest::ParseFromURLEncoded_()
{
    // example: username=zhangsan&password=123456
    if (body_.size() == 0)
    {
        return;
    }
    size_t pre = 0;
    size_t found = body_.find_first_of("&");
    while (found != std::string::npos)
    {
        ParseKeyValue_(body_.substr(pre, found - pre));
        pre = found + 1;
        found = body_.find_first_of("&", pre);
    }
    ParseKeyValue_(body_.substr(pre, found - pre));
}

bool HttpRequest::UserVerify(const std::string &name, const std::string &pwd, bool is_login)
{
    if (name == "" || pwd == "")
    {
        return false;
    }
    LOG_INFO("Verify name:%s pwd:%s", name.c_str(), pwd.c_str());
    MYSQL *sql;
    SqlConn(&sql, SqlConnPool::GetInstance());

    char sql_query[256] = {0};
    MYSQL_RES *res = nullptr;
    MYSQL_FIELD *fields = nullptr;

    if (is_login)
    {
        sprintf(sql_query, "SELECT username, passwd FROM user WHERE username='%s' AND passwd='%s'", name.c_str(), pwd.c_str());
        // 查询数据库并验证
        if (mysql_query(sql, sql_query))
        {
            mysql_free_result(res);
            LOG_DEBUG("Query user error");
            return false;
        }
        res = mysql_store_result(sql);
        if (res && mysql_num_rows(res) > 0)
        {
            mysql_free_result(res);
            return true;
        }
    }
    else
    {
        // check if user exists first
        sprintf(sql_query, "SELECT * FROM user WHERE username='%s'", name.c_str());
        if (mysql_query(sql, sql_query))
        {
            mysql_free_result(res);
            LOG_DEBUG("Query user error");
            return false;
        }
        res = mysql_store_result(sql);
        if (res && mysql_num_rows(res) > 0)
        {
            mysql_free_result(res);
            LOG_DEBUG("User %s exists", name.c_str());
            return false;
        }
        // insert new user
        sprintf(sql_query, "INSERT INTO user(username, passwd) VALUES('%s', '%s')", name.c_str(), pwd.c_str());
        if (mysql_query(sql, sql_query))
        {
            mysql_free_result(res);
            LOG_DEBUG("Insert user error");
            return false;
        }
        mysql_free_result(res);
        return true;
    }

    mysql_free_result(res);
    mysql_close(sql);
    return false;
}