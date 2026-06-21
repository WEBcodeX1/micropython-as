#pragma once

#include "/usr/local/include/esp32s3/httpparser.hpp"

#include <cstdint>
#include <string>
#include <cerrno>

#include <unistd.h>

typedef int ClientFD_t;

class Client : public HTTPParser
{

public:

    Client(ClientFD_t);
    ~Client();

    bool receiveData();

protected:

    ClientFD_t _ClientFD;

private:

    char* _ReceiveBuffer;

};
