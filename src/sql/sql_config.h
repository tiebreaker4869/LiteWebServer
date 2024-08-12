#ifndef LWS_SQL_SQL_CONFIG_H_
#define LWS_SQL_SQL_CONFIG_H_

#include <string>

/**
 * 数据库连接池配置项
 */
struct SQLConfig
{
    std::string url;

    std::string username;

    std::string password;

    std::string db_name;

    int port;

    int max_connection;

    int close_log; // 日志开关
};

#endif