#include "webserver.hpp"
#include "json.hpp"

#include <fstream>
#include <signal.h>
#include <memory>
#include <iostream>
#include <string>

WebServer *server;

void SignalHandler(int signal_no)
{
    if (signal_no == SIGINT)
    {
        std::cout << "\n STOP Server" << std::endl;
        server->Stop();
        delete server;
        exit(0);
    }
}

int main(int argc, char *argv[])
{
    std::ifstream config_file{"../config.json"};
    nlohmann::json config;
    config_file >> config;

    server = new WebServer(
        static_cast<int>(config["port"]),
        static_cast<int>(config["trig_mode"]),
        static_cast<int>(config["timeout_ms"]),
        static_cast<bool>(config["opt_linger"]),
        static_cast<int>(config["sql_port"]),
        static_cast<std::string>(config["sql_user"]).c_str(),
        static_cast<std::string>(config["sql_pwd"]).c_str(),
        static_cast<std::string>(config["db_name"]).c_str(),
        static_cast<int>(config["conn_pool_num"]),
        static_cast<int>(config["thread_num"]),
        static_cast<bool>(config["open_log"]),
        static_cast<int>(config["log_level"]),
        static_cast<int>(config["log_queue_size"]));

    struct sigaction sig_action;
    sig_action.sa_handler = SignalHandler;
    sigaction(SIGINT, &sig_action, nullptr);

    server->Run();
    return 0;
}