#ifndef ClientHandler_hpp
#define ClientHandler_hpp

#include <string>
#include <memory>
#include <unordered_map>

using namespace std;

typedef unordered_map<uint16_t, const std::string> ClientMap_t;


class ClientHandler
{

public:

    ClientHandler();
    ~ClientHandler();

    void addClient(const uint16_t);

private:

    ClientMap_t Clients;
    int16_t ProcessedClients;
};

#endif
