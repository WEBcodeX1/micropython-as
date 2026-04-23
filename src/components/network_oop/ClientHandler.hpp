#pragma once

#include <string>
#include <memory>
#include <unordered_map>

using namespace std;

typedef unordered_map<uint16_t, const string> ClientMap_t;
typedef uint16_t ClientFD_t;

class ClientHandler
{

public:

    ClientHandler();
    ~ClientHandler();

    void addClient(const ClientFD_t);

private:

    ClientMap_t Clients;
    int16_t ProcessedClients;
};
