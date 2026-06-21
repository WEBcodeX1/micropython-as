#pragma once

#include <string>
#include "constants.h"

extern "C" {
#include "/usr/local/include/esp32s3/micropython_embed.h"
}


using namespace std;

class MicroPython
{

public:

    MicroPython(char*, int, int*);
    ~MicroPython();

    bool callFunction(string&, string&, string&);

private:

};
