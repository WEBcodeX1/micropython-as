#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <vector>
#include <string>


class DNSServer {

public:

    DNSServer();
    ~DNSServer();

    void start();
    void ServerLoop();

private:

    uint16_t SocketListenPort = 53;

    struct sockaddr_in ServerSocketAddr;
    struct sockaddr_in ClientSocketAddr;

    int ServerSocketFD;
    socklen_t ClientSocketLen;
    unsigned char RecvBuffer[128];

};
