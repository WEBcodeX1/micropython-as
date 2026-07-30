#pragma once

#include "/usr/local/include/esp32s3/httpconstants.hpp"

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
