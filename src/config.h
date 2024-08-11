#ifndef LWS_CONFIG_H_
#define LWS_CONFIG_H_

#include <string>

class Config {
public:
    Config();
    
    void parse_args(int argc, char* argv[]);

    int port;

    int log_write_mode;

    int trig_mode;

    int listen_trig_mode;

    int conn_trig_mode;

    int opt_linger;

    int thread_num;

    int close_log;

    std::string root_dir;
    
    int sql_conn_num;
    
    std::string db_username;
    
    std::string db_password;
    
    std::string db_name;
};

#endif
