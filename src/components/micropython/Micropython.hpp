#pragma once

#include <string>
#include "constants.h"

extern "C" {
#include "/usr/local/include/esp32c3/micropython_embed.h"
}

using namespace std;


class MicroPython
{

public:

    MicroPython();
    ~MicroPython();

    bool callFunction(string&, string&, string&);

private:

    char InterpreterHeap[MICROPYTHON_HEAP_SIZE];
    int InterpreterStackTop;

};
