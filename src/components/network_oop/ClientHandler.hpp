#pragma once

#include "Client.hpp"
#include "Network.hpp"

#include <string>
#include <memory>
#include <unordered_map>


using namespace std;

typedef std::shared_ptr<Client> ClientRef_t;
typedef unordered_map<ClientFD_t, ClientRef_t> ClientMap_t;

class ClientHandler
{

public:

    ClientHandler();
    ~ClientHandler();

    void addClient(const ClientFD_t);
    void processClients();

private:

    ClientMap_t Clients;
};
