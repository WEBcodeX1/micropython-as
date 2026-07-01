#pragma once

#include "Client.hpp"
#include "Network.hpp"

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>


using namespace std;

typedef Client* ClientRef_t;
typedef unordered_map<ClientFD_t, ClientRef_t> ClientMap_t;

class ClientHandler
{

public:

    ClientHandler();
    ~ClientHandler();

    void addClient(const ClientFD_t);
    uint8_t processClients();

private:

    ClientMap_t Clients;
    char receiveBuffer[1024];
};
