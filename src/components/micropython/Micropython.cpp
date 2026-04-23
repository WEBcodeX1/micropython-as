#include "Micropython.hpp"

using namespace std;

Micropython::Micropython()
{
    mp_embed_init(&InterpreterHeap[0], sizeof(InterpreterHeap), &InterpreterStackTop);
}

Micropython::~Micropython()
{
}

void Micropython::test()
{
}
