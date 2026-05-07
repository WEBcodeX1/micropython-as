# Architecture

This project requires 2 external static libraries and C++ header files.

## 1. HTTPParser Library

https://github.com/WEBcodeX1/http-1.2/tree/main/ports/arduino/esp32c3

> [!WARNING]
> Version `v0.2` (tag) should be used (tested)

## 2. Embedding MicroPython

https://github.com/clauspruefer/micropython

Until my proposed source changes are production ready (merged to official MicroPython), you must use my MicroPython fork.

It includes the following:

- Installable static cross-compiled MicroPython library (and C header file) for use in external CMake projects
- Extended embedded port, providing the possibility to call MicroPython functions `mp_embed_exec_string_function()` with 1 parameter and string return type

> [!WARNING]
> You must use `v1.26-release` branch (modified, extended branch), `main` branch is currently broken.
