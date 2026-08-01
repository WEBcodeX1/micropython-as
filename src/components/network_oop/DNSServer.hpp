#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <cstddef>
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
    static constexpr size_t DNSQuestionOffset = 12;
    static constexpr unsigned char PongGameQueryName[] = {
        0x04, 'p', 'o', 'n', 'g',
        0x04, 'g', 'a', 'm', 'e',
        0x00
    };
    static constexpr size_t PongGameQueryNameLength = sizeof(PongGameQueryName);

    struct sockaddr_in ServerSocketAddr;
    struct sockaddr_in ClientSocketAddr;

    int ServerSocketFD;
    socklen_t ClientSocketLen;
    unsigned char RecvBuffer[128];

};
