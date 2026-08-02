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
    static constexpr unsigned char Edns0OptRecord[] = {
        0x00, 0x00, 0x29, 0x10, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00
    };
    static constexpr size_t Edns0OptRecordLength = sizeof(Edns0OptRecord);
    static constexpr size_t PongGameEdns0QueryLength = PongGameQueryLength + Edns0OptRecordLength;

    struct sockaddr_in ServerSocketAddr;
    struct sockaddr_in ClientSocketAddr;

    int ServerSocketFD;
    socklen_t ClientSocketLen;
    unsigned char RecvBuffer[128];

};
