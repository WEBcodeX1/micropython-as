#pragma once

extern "C" {
#include "/usr/local/include/micropython_embed.h"
}

using namespace std;


class Micropython
{

public:

    Micropython();
    ~Micropython();

    void test();

private:
    char InterpreterHeap[8*1024];
    int InterpreterStackTop;

};
