#include "Server.hpp"
#include "Network.hpp"

#include "esp_log.h"

using namespace std;


Server::Server()
{
}

Server::~Server()
{
}

void Server::init()
{
    //- set listen address / port
    SocketListenAddress = Network::getIPAddr();
    SocketListenPort = 80;

    //- setup server socket
    setupSocket();

    //- setup server socket monitoring
    setupPoll();

    //- start server loop
    ServerLoop();
}

void Server::setupSocket()
{
    //- setup socket, parameter
    ServerSocketFD = socket(AF_INET, SOCK_STREAM, 0);

    ESP_LOGI("HTTPServer", "SocketFD:%d", ServerSocketFD);

    memset((char*)&SocketAddr, 0, sizeof(SocketAddr));

    SocketAddr.sin_family = AF_INET;
    SocketAddr.sin_port = htons(SocketListenPort);
    SocketAddr.sin_addr.s_addr = inet_addr(SocketListenAddress.c_str());

    //- set socket reuse address and reuse port flag
    int flags = fcntl(ServerSocketFD, F_GETFL, 0);

    fcntl(ServerSocketFD, F_SETFL, flags | SO_REUSEADDR | SO_REUSEPORT);

    const int sockopt = 1;

    //- set socket TCP_NODELAY
    setsockopt(ServerSocketFD, IPPROTO_TCP, TCP_NODELAY, &sockopt, sizeof(int));

    //- make server socket nonblocking
    Socket::makeNonblocking(ServerSocketFD);

    //- bind socket, start listen
    bind(ServerSocketFD, (struct sockaddr*)&SocketAddr, sizeof(SocketAddr));

    listen(ServerSocketFD, 0);
}

void Server::setupPoll()
{
    ServerConnFD[0].fd = ServerSocketFD;
    ServerConnFD[0].events = POLLIN;
}

void Server::ServerLoop()
{
    //- main server loop
    while(1) {

        //- poll server fd for incoming connections
        if (poll(ServerConnFD, 1, 0) > 0) {
            ESP_LOGI("HTTPServer", "accept()");

            //- check for incoming connection
            if (ServerConnFD[0].revents & POLLIN) {
                acceptClient();
            }
        }

        //- process clients
        auto sumMessages = processClients();

        //- idle wakeup delay
        if (sumMessages == 0) {
            vTaskDelay(10);
        }
        else if (sumMessages >= 0) {
            vTaskDelay(1);
        }
    }
}

void Server::acceptClient()
{
    socklen_t ClientSocketLen;
    ClientSocketLen = sizeof(ClientSocketAddr);

    ClientFD_t ClientFD = accept(
        ServerSocketFD,
        reinterpret_cast<struct sockaddr*>(&ClientSocketAddr),
        &ClientSocketLen
    );

    ESP_LOGI("HTTPServer", "ClientFD:%d", ClientFD);

    if (ClientFD > 0) {
        addClient(ClientFD);
    }
}
