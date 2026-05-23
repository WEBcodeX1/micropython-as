#include "ClientHandler.hpp"
#include "constants.h"


using namespace std;

ClientHandler::ClientHandler()
{
    //- preserve maximum clients
    Clients.reserve(WIFI_MAX_STA_CONN);
}

ClientHandler::~ClientHandler()
{
}

void ClientHandler::addClient(const ClientFD_t ClientFD)
{
    //- set client connection non blocking
    Socket::makeNonblocking(ClientFD);

    ClientRef_t ClientObj = std::make_shared<Client>(
        ClientFD
    );

    Clients.emplace(
        ClientFD, move(ClientObj)
    );
}

void ClientHandler::processClients()
{
    //- receive data from all client filedescriptors
    for (auto const& ClientItem : Clients) {
        auto const ReadFD = ClientItem.first;
        if (Clients[ReadFD]->receiveData() == true) {
            Clients.erase(ReadFD);
            close(ReadFD);
        }
    }
}
