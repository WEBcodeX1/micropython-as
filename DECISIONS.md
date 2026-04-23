# Decision History

This document should give insights about the **WHY** we did **WHAT** and **WHEN**.

## Multiprocessing

The FalconAS Linux port allows binding processes to CPU core IDs. The ESP32-C3 is a single core processor, and thus limited.
A *bigger* variant ESP32-S3 integrates 2 CPU cores, it could be a good idea to seperate the web server processing and the micropython code execution.

To produce clean code we should abstract these two layers for both a) single core and b) dual core processing in a generic way. These two possibilities exist:

- RTOS tasks
- C++11 threading library

Seperating the web server and the python code could be done using 2 RTOS tasks, on dual-core controllers the micropython task later on can be run on a different CPU core. I think we should start implementing 2 RTOS tasks on the ESP32-C3 first.

## Project Goal

The project goal is to provide a in-web-server playable PONG game on two connected wifi-clients (smartphones).
The ESP32-C3 acts as a Micropython Application Server, the game logic will be programmed in Python.

The ESP32-C3 has an attached OLED 1306 display which renders the complete game in realtime. The ESP32-C3 integrated web server will also host the application files, on smartphone browser connect the application will be transfered to the browser, started; following communication is abstracted via HTTP/1.1 POST with JSON payload.

## DNS

To connect the smartphones without using other peripherals, we also will integrate a micro-tiny DNS (UDP-only) server which will only react on a single *static* DNS IPv4 A query to the applications host *pong.game*.

The DNS server also will be *run* as RTOS task.

## Libraries / Networking

### Global Assumptions

- Networking sockets (lwIP) **type** is **BSD**
- Networking sockets (lwIP) **must** be implemented **non-blocking**

### Abstraction

We will provide ESP32 C++ higher level networking abstraction libraries with the following *logical* components:

1. Networking
2. ServerTCP
3. ServerUDP
4. ClientHandler

## Firmware Building

The resulting firmware binary should be small in size, include all required functionality and security features.

With the following settings, we achieved to build a firmware image of apx. 500 Kilobyte size:

- Wifi access point (only WPA2 personal, WPA3 and WPA3 enterprise disabled)
- Include mbedTLS library (for HTTPS server)
- Disable debug build, optimize for size (-Os compiler flag)
- Only IPv4 (no IPv6) lwIP stack
- Disable C++ exceptions and RTTI (real time type information)

## HTTP/1.1 Parser Library

To include / link the HTTP/1.1 parsing library as a static (.a file) into a ESP32-C3 IDF *component* it **must** be built with a gcc / g++ *RISC32V* toolchain. To integrate a whole CMake project into an IDF component can be huge amount of work, so it is much better to do it vice versa: tell CMake to use the already installed ESP IDF *RISC32V* toolchain.

> [!WARNING]
> Somehow using class inheritance in the static source library resulted in unresolved symbols in the IDFs component linking process. Using only the base class works fine, i will further investigate.

## MicroPython

We also will try to integrate MicroPython as a cross-compiled static linked library with the same approach as the HTTP parsing library.
