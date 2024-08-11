#ifndef LWS_WEBSERVER_H_
#define LWS_WEBSERVER_H_

#include "config.h"
#include "constants.h"

class Webserver {
public:
    Webserver();

    ~Webserver();

    void Init(Config config);

    void Start();
private:
    void EventListen();

    void EventLoop();
};

#endif