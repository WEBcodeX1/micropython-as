#include "Micropython.hpp"

using namespace std;


MicroPython::MicroPython(char* InterpreterHeap, int InterpreterHeapSize, int* InterpreterStackTop)
{
    mp_embed_init(InterpreterHeap, InterpreterHeapSize, InterpreterStackTop);
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
