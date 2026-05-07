#include "Micropython.hpp"

using namespace std;

MicroPython::MicroPython()
{
    mp_embed_init(&InterpreterHeap[0], sizeof(InterpreterHeap), &InterpreterStackTop);
}

MicroPython::~MicroPython()
{
    mp_embed_deinit();
}

bool MicroPython::callFunction(string& FunctionName, string& FunctionParam, string& Result)
{
    const char* CResultPointer = nullptr;

    CResultPointer = mp_embed_exec_string_function(
        FunctionName.c_str(),
        FunctionParam.c_str()
    );

    if (CResultPointer) {
        Result.assign(CResultPointer);
        return true;
    }

    return false;
}
