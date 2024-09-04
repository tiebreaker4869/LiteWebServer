#pragma once

#include "buffer.hpp"
#include <sys/stat.h>
#include <unordered_map>

class HttpResponse
{
public:
    HttpResponse();
    ~HttpResponse();

    void Init(const std::string& src_dir, std::string& path, bool is_keep_alive = false, int code  =-1);
    void MakeResponse(Buffer& buff);
    void UnmapFile();
    char* File();
    void ErrorContent(Buffer& buff, std::string message);
    int Code() const {return code_;}
private:
    void AddStatusLine_(Buffer& buff);
    void AddHeader_(Buffer& buff);
    void AddContent_(Buffer& buff);

    void ErrorHtml_();
    std::string GetFileType_();

    int code_;
    bool is_keep_alive_;
    
    std::string path_;
    std::string src_dir_;

    char* file_;
    struct stat file_state_;

    static const std::unordered_map<std::string, std::string> kSuffixType;
    static const std::unordered_map<int, std::string> kCodeStatus;
    static const std::unordered_map<int, std::string> kCodePath;
};