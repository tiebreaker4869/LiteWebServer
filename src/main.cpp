#include <string>

#include "config.h"
#include "webserver.h"


int main(int argc, char* argv[]) {

    Config config;

    Webserver server;

    server.Init(config);

    server.Start();

    return 0;
}