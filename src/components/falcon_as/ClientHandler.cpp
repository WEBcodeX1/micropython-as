#include "ClientHandler.hpp"

using namespace std;

ClientHandler::ClientHandler() :
    ProcessedClients(0)
{
    //- init clients map
    Clients.reserve(10);
}

ClientHandler::~ClientHandler()
{
}

void ClientHandler::addClient(const uint16_t ClientFD)
{
}
