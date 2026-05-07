# Architecture

This project requires 2 external static libraries and C/C++ header files.

## 1. HTTPParser Library

https://github.com/WEBcodeX1/http-1.2/tree/v0.2-release/ports/arduino/esp32c3

> [!WARNING]
> You must use `v0.2-release` branch (tested).

## 2. Embedding MicroPython

https://github.com/clauspruefer/micropython/tree/v1.26-release/examples/embedding-staticlib

Until my proposed source changes are production ready (merged to official MicroPython), you must use my MicroPython fork.

It includes the following:

- Installable static cross-compiled MicroPython library (and C header file) for use in external CMake projects
- Extended embedded port, providing the possibility to call MicroPython functions `mp_embed_exec_string_function()` with 1 parameter and string return type

> [!WARNING]
> You must use `v1.26-release` branch (modified, extended branch), the `main` branch is currently broken.
