#ifndef HTTP_HTTP_CONN_H_
#define HTTP_HTTP_CONN_H_

#include <string>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <map>

#include "sql/sql_connection_pool.h"
#include "constants.h"

enum Method
{
    kGet = 0,
    kPost,
    kHead,
    kPut,
    kDelete,
    kTrace,
    kOptions,
    kConnect,
    kPath
};

enum CheckState
{
    kCheckStateRequestLine = 0,
    kCheckStateHeader,
    kCheckStateContent
};

enum HttpCode
{
    kNoRequest = 0,
    kGetRequest,
    kBadRequest,
    kNoResource,
    kForbbidenRequest,
    kFileRequest,
    kInternalError,
    kClosedConnection
};

enum LineStatus
{
    kLineOk = 0,
    kLineBad,
    kLineOpen
};

struct HttpConfig
{
    int sockfd;
    const sockaddr_in addr;
    std::string root;
    int trig_mode;
    int close_log;
    std::string user;
    std::string password;
    std::string db_name;
};

class HttpConn
{
public:
    static constexpr int kFileNameLen = 200;
    static constexpr int kReadBufferSize = 2048;
    static constexpr int kWriteBufferSize = 1024;

    HttpConn() = default;
    ~HttpConn() = default;

    void Init(HttpConfig config);
    void CloseConn(bool real_close = true);
    void Process();
    bool ReadOnce();
    bool Write();
    sockaddr_in *GetAddress() { return &address_; }
    void InitMySQLResult(SQLConnPool *sql_conn_pool);
    bool timer_flag;
    bool progress;
    static int epollfd;
    static int user_count;
    static std::map<std::string, std::string> users;
    MYSQL *sql_conn;
    int close_log;
    Operation op; // 读为 0(kRead) 写为 1(kWrite)

private:
    void Init();
    HttpCode ProcessRead();
    bool ProcessWrite(HttpCode ret);
    HttpCode ParseRequestLine(char *text);
    HttpCode ParseHeaders(char *text);
    HttpCode ParseContent(char *text);
    HttpCode DoRequest();
    char *GetLine() { return read_buf_ + start_line_; }
    LineStatus ParseLine();
    void Unmap();
    bool AddResponse(const char *format, ...);
    bool AddContent(const char *content);
    bool AddStatusLine(int status, const char *title);
    bool AddHeaders(int content_length);
    bool AddContentType();
    bool AddContentLength(int content_length);
    bool AddLinger();
    bool AddBlankLine();

    int sockfd_;
    sockaddr_in address_;
    char read_buf_[kReadBufferSize];
    long read_idx_;
    long checked_idx_;
    int start_line_;
    char write_buf_[kWriteBufferSize];
    int write_idx_;
    CheckState check_state_;
    Method method_;
    char real_file_[kFileNameLen];
    char *url_;
    char *version_;
    char *host_;
    long content_length_;
    bool linger_;
    char *file_address_;
    struct stat file_stat_;
    struct iovec iv_[2];
    int iv_count_;
    int cgi_;
    char *str_; // 存储请求头数据
    int bytes_to_send_;
    int bytes_have_sent_;
    std::string doc_root_;

    int trig_mode_;

    std::string sql_user_;
    std::string sql_password_;
    std::string sql_db_name_;
};

#endif