#pragma once

#include "/usr/local/include/esp32s3/httpparser.hpp"
#include "/usr/local/include/esp32s3/httpgenerator.hpp"

#include <cstdint>
#include <string>
#include <cerrno>

#include <unistd.h>

typedef int ClientFD_t;

class Client : public HTTPParser, public HTTPGenerator
{

public:

    Client(ClientFD_t);
    ~Client();

    bool receiveData(char*);
    ClientFD_t getClientFD();

protected:

    ClientFD_t _ClientFD;

private:

};
