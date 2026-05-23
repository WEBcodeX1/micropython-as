#pragma once

#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include "ClientHandler.hpp"


class Server : private ClientHandler {

public:

    Server();
    ~Server();

    void init();
    void setupSocket();
    void setupPoll();
    void ServerLoop();
    void acceptClient();

private:

    string SocketListenAddress;
    uint SocketListenPort;

    struct sockaddr_in SocketAddr;
    struct sockaddr_in ClientSocketAddr;

    int ServerSocketFD;

    struct pollfd ServerConnFD[1];
};
