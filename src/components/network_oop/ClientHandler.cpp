#include "ClientHandler.hpp"
#include "/usr/local/include/httpparser.hpp"
#include "constants.h"


using namespace std;

ClientHandler::ClientHandler() :
    ProcessedClients(0)
{
    //- preserve maximum clients
    Clients.reserve(WIFI_MAX_STA_CONN);
}

ClientHandler::~ClientHandler()
{
}

void ClientHandler::addClient(const ClientFD_t ClientFD)
{
    HTTPParser testparser = HTTPParser();
}
