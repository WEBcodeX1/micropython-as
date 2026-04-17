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

1. NetworkHelper
2. ServerTCP
3. ServerUDP
4. ClientHandler

