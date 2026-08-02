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

    DNSServer(uint16_t ListenPort = 53);
    ~DNSServer();

    void start();
    void ServerLoop();

private:

    uint16_t SocketListenPort;

    static constexpr size_t DNSQuestionOffset = 12;

    static constexpr unsigned char PongGameQueryName[] = {
        0x04, 'p', 'o', 'n', 'g',
        0x04, 'g', 'a', 'm', 'e',
        0x00
    };

    static constexpr size_t PongGameQueryNameLength = sizeof(PongGameQueryName);
    static constexpr size_t DNSQuestionTypeOffset = DNSQuestionOffset + PongGameQueryNameLength;
    static constexpr size_t PongGameQueryLength = DNSQuestionTypeOffset + 4;
    static constexpr size_t Edns0OptRecordOffset = PongGameQueryLength;
    static constexpr size_t Edns0OptTypeOffset = Edns0OptRecordOffset + 1;
    static constexpr size_t Edns0OptHeaderLength = 11;
    static constexpr size_t PongGameEdns0QueryLengthMinimum = PongGameQueryLength + Edns0OptHeaderLength;

    struct sockaddr_in ServerSocketAddr;
    struct sockaddr_in ClientSocketAddr;

    int ServerSocketFD;
    socklen_t ClientSocketLen;
    unsigned char RecvBuffer[128];

};
