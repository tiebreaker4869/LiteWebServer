#include "http_response.hpp"

#include <unordered_map>
#include <sys/stat.h>
#include <cstddef>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
const std::unordered_map<std::string, std::string> HttpResponse::kSuffixType = {
    {".html", "text/html"},
    {".xml", "text/xml"},
    {".xhtml", "application/xhtml+xml"},
    {".txt", "text/plain"},
    {".rtf", "application/rtf"},
    {".pdf", "application/pdf"},
    {".word", "application/msword"},
    {".png", "image/png"},
    {".jpg", "image/jpeg"},
    {".jpeg", "image/jpeg"},
    {".au", "audio/basic"},
    {".mpeg", "video/mpeg"},
    {".mpg", "video/mpeg"},
    {".avi", "video/x-msvideo"},
    {".gz", "application/x-gzip"},
    {".tar", "application/x-tar"},
    {".css", "text/css"},
    {".js", "application/x-javascript"},
};

const std::unordered_map<int, std::string> HttpResponse::kCodeStatus = {
    {200, "OK"},
    {400, "Bad Request"},
    {403, "Forbidden"},
    {404, "Not Found"},
};

const std::unordered_map<int, std::string> HttpResponse::kCodePath = {
    {400, "/400.html"},
    {403, "/403.html"},
    {404, "/404.html"},
};

HttpResponse::HttpResponse()
{
    code_ = -1;
    path_ = src_dir_ = "";
    is_keep_alive_ = false;
    file_ = nullptr;
    file_state_ = {0};
}

HttpResponse::~HttpResponse()
{
    UnmapFile();
}

void HttpResponse::Init(const std::string &src_dir, std::string &path, bool is_keep_alive, int code)
{
    if (file_)
    {
        UnmapFile();
    }
    code_ = code;
    is_keep_alive_ = is_keep_alive;
    path_ = path;
    src_dir_ = src_dir;
    file_ = nullptr;
    file_state_ = {0};
}

void HttpResponse::MakeResponse(Buffer &buff)
{
    if (stat((src_dir_ + path_).data(), &file_state_) < 0 || S_ISDIR(file_state_.st_mode))
    {
        code_ = 404; // 文件不存在
    }
    else if (!(file_state_.st_mode & S_IROTH)) // 文件权限不足
    {
        code_ = 403;
    }
    else if (code_ == -1) // 正常
    {
        code_ = 200;
    }

    ErrorHtml_();
    AddStatusLine_(buff);
    AddHeader_(buff);
    AddContent_(buff);
}

char *HttpResponse::File()
{
    return file_;
}

size_t HttpResponse::FileLen() const
{
    return file_state_.st_size;
}

void HttpResponse::ErrorHtml_()
{
    if (kCodePath.count(code_) == 1)
    {
        path_ = kCodePath.find(code_)->second;
        stat((src_dir_ + path_).data(), &file_state_);
    }
}

void HttpResponse::AddStatusLine_(Buffer &buff)
{
    std::string status;
    if (kCodeStatus.count(code_) == 1)
    {
        status = kCodeStatus.find(code_)->second;
    }
    else
    {
        code_ = 400;
        status = kCodeStatus.find(400)->second;
    }
    buff.Append("HTTP/1.1 " + std::to_string(code_) + " " + status + "\r\n");
}

void HttpResponse::AddHeader_(Buffer &buff)
{
    buff.Append("Connection: ");
    if (is_keep_alive_)
    {
        buff.Append("keep-alive\r\n");
        buff.Append("keep-alive: max=6, timeout=120\r\n");
    }
    else
    {
        buff.Append("close\r\n");
    }
    buff.Append("Content-type: " + GetFileType_() + "\r\n");
}

void HttpResponse::AddContent_(Buffer &buff)
{
    int srcfd = open((src_dir_ + path_).data(), O_RDONLY);
    if (srcfd < 0)
    {
        ErrorContent(buff, "File Not Found!");
        return;
    }

    // 将文件映射到内存提高文件的访问速度
    file_ = static_cast<char *>(mmap(0, file_state_.st_size, PROT_READ, MAP_PRIVATE, srcfd, 0));
    close(srcfd);
    if (file_ == MAP_FAILED)
    {
        ErrorContent(buff, "File Not Found!");
        return;
    }
    buff.Append("Content-length: " + std::to_string(file_state_.st_size) + "\r\n");
}

void HttpResponse::UnmapFile()
{
    if (file_)
    {
        munmap(file_, file_state_.st_size);
        file_ = nullptr;
    }
}

std::string HttpResponse::GetFileType_()
{
    std::string::size_type idx = path_.find_last_of('.');
    if (idx == std::string::npos)
    {
        return "text/plain";
    }
    std::string suffix = path_.substr(idx);
    if (kSuffixType.count(suffix) == 1)
    {
        return kSuffixType.find(suffix)->second;
    }
    return "text/plain";
}

void HttpResponse::ErrorContent(Buffer &buff, std::string message)
{
    std::string body;
    std::string status;
    body += "<html><title>Error</title>";
    body += "<body bgcolor=\"ffffff\">";
    if (kCodeStatus.count(code_) == 1)
    {
        status = kCodeStatus.find(code_)->second;
    }
    else
    {
        status = "Bad Request";
    }
    body += std::to_string(code_) + " : " + status;
    body += "<p>" + message + "</p>";
    body += "<hr><em>LiteWebServer</em></body></html>";

    buff.Append("Content-length: " + std::to_string(body.size()) + "\r\n");
    buff.Append(body);
}