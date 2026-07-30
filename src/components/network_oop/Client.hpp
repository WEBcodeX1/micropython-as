#pragma once

#include "ASRequestHandler.hpp"

#ifdef LINUX_BUILD
#include "/usr/local/include/linux/httpparser.hpp"
#include "/usr/local/include/linux/httpgenerator.hpp"
#else
#include "/usr/local/include/esp32s3/httpparser.hpp"
#include "/usr/local/include/esp32s3/httpgenerator.hpp"
#endif

#include <cstdint>
#include <string>
#include <cerrno>

#include <unistd.h>

typedef int ClientFD_t;

class Client : public HTTPParser, public HTTPGenerator, public ASRequestHandler
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
