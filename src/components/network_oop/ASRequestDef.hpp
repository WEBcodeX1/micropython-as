#pragma once

#ifdef LINUX_BUILD
#include "/usr/local/include/linux/httpconstants.hpp"
#else
#include "/usr/local/include/esp32s3/httpconstants.hpp"
#endif

#include "ASRequestHandler.hpp"

#include <array>


static const ASRequestDefinition_t r1 = {
    AS_REQ_GAME_START,
    "/python/startgame",
    HTTP_METHOD_GET,
    ""
};

static const ASRequestDefinition_t r2 = {
    AS_REQ_GAME_STOP,
    "/python/stopgame",
    HTTP_METHOD_GET,
    ""
};

static const ASRequestDefinition_t r3 = {
    AS_REQ_PADDLE_UP,
    "/python/paddleup",
    HTTP_METHOD_POST,
    ""
};

static const ASRequestDefinition_t r4 = {
    AS_REQ_PADDLE_DOWN,
    "/python/paddledown",
    HTTP_METHOD_POST,
    ""
};

static const std::array<ASRequestDefinition_t, 4> ASRequestDefinitions = {
    r1,
    r2,
    r3,
    r4
};
