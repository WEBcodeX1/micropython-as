#pragma once

#ifdef LINUX_BUILD
#include "/usr/local/include/linux/httpparser.hpp"
#else
#include "/usr/local/include/esp32s3/httpparser.hpp"
#endif

#include <cstdint>
#include <string>


using namespace std;

//- constants
static constexpr unsigned int AS_REQ_WAIT_IN = 1;
static constexpr unsigned int AS_REQ_PROCESSING = 2;
static constexpr unsigned int AS_REQ_PROCESSED = 3;

static constexpr unsigned int AS_REQ_GAME_START = 1;
static constexpr unsigned int AS_REQ_GAME_STOP = 2;
static constexpr unsigned int AS_REQ_PADDLE_UP = 3;
static constexpr unsigned int AS_REQ_PADDLE_DOWN = 4;


//- request struct definition
struct ASRequestDefinition_t {
    unsigned int ID;
    string URL;
    uint16_t HTTPMethod;
    string MicroPythonFunction;
};


class ASRequestHandler
{

public:

    ASRequestHandler();
    ~ASRequestHandler();

    void ASRequestInit(RequestProperties_t);

private:

};
