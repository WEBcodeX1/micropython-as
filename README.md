# Embedding MicroPython in a C++ HTTP/1.1 Application Server (ESP32-S3, ESP32-C3)

This project embeds *MicroPython* in *ESP32* microcontrollers using the **Falcon-AS** C++ HTTP/1.1 parser library
and modified application server code adapted to the microcontroller architecture.

Refer to [./BUILD.md](./BUILD.md) to learn how to build the project.

I have added some videos showing the current project status / progress in [./video/](./video/) (gameplay and browser integration).

> [!WARNING]
> Currently, only the **ESP32-S3** is supported. **ESP32-C3** *MicroPython* crashes during floating-point math processing.

# 1. Architecture

The following diagrams show a comparison between the native *MicroPython* design and architecture and our HTTP application server approach.

## 1.1. MicroPython Architecture

![MicroPythonArchitecture](/diagram/micropython-architecture.png)

## 1.2. MicroPythonAS Architecture

![MicroPythonASArchitecture](/diagram/micropython-as-architecture.png)

# 2. Project Status

| Feature <img width=450px></img>                    | Status <img width=150px></img> | Flag               |
|----------------------------------------------------|--------------------------------|--------------------|
| Cross Compiling External Libraries                 | Stable, tested                 | :white_check_mark: |
| HTTP Parser / Static Library                       | Stable, tested                 | :white_check_mark: |
| Embedded MicroPython Interpreter / Static Library  | Stable, tested                 | :white_check_mark: |
| MicroPython PONG Game                              | Stable, tested                 | :white_check_mark: |
| Realtime Game Rendering / Title Cube Animation     | Stable, tested                 | :white_check_mark: |
| Peripherals (SSD1306 Display, 3 Color LED)         | Stable, tested                 | :white_check_mark: |
| Basic Networking / SoftAP                          | Stable, tested                 | :white_check_mark: |
| Posix Threads to RTOS Tasks Wrapper                | Stable, tested                 | :white_check_mark: |
| Basic DNS (UDP) Server / A Record / +EDNS0         | Stable, tested                 | :white_check_mark: |
| BSD-Socket Porting / FalconAS                      | Stable, tested                 | :white_check_mark: |
| Static Filesystem / FalconAS HTTP/1.1 GET          | Stable, tested                 | :white_check_mark: |
| Modified, working x0 Browser Framework             | Stable, tested                 | :white_check_mark: |
| FalconAS HTTP/1.1 GET / POST MicroPython Interface | Stable, tested                 | :white_check_mark: |
| Static HTTP/1.1 Server / OS DNS+HTTP Requests      | Stable, tested                 | :white_check_mark: |

## 2.1. External Links

- https://github.com/WEBcodeX1/http-1.2
- https://github.com/WEBcodeX1/x0
- https://github.com/clauspruefer/MicroPythonPong

# 3. ESP32-C3 SoC

[Espressif Systems](https://www.espressif.com/) developed the **ESP32-C3** system-on-chip, based on a 32-bit single-core RISC-V CPU. Ready-to-use boards are available from *Seeed Studio* (XIAO ESP32-C3 mini board) at an affordable price point (5€ :moneybag::star2:).

## 3.1. Base Features

- IEEE 802.11 b/g/n Wi-Fi
- Bluetooth 5 (BLE)
- 14 usable external pins
- Hardware cryptography (RSA, ECC, AES)
- Battery connector pins
- USB-C battery charging
- USB-C firmware / JTAG port
- USB-C serial debugging
- Internal temperature sensor
- RTC real-time clock
- Low-power / sleep-state control
- Secure boot / firmware signing
- 400 KB SRAM and 4 MB on-board flash memory

## 3.2. Detailed Hardware Specs

For detailed hardware information, start with the manufacturer's wiki: https://wiki.seeedstudio.com/XIAO_ESP32C3_Getting_Started/.

# 4. Variants

There are also several notable variants.

## 4.1. ESP32-S3

ESP32-S3R8 Xtensa LX7 dual-core 240 MHz system with optional external HAT extensions, for example a camera or audio add-on.

> [!NOTE]
> The current ESP-IDF integrates a *transparent* C++11 threading implementation that makes code execution across multiple CPU cores much easier :heart_eyes:.

## 4.2. ESP32-P4

A high-performance, highly secure board (**without** integrated Wi-Fi or Bluetooth), with attachable MicroSD card and PHY Ethernet (IP101GR).

# 5. Development Frameworks

With this amount of system memory, development should be straightforward. However, there are multiple pitfalls. This document discusses how to choose the **correct** SDK (software development kit) from the available options for your requirements.

The following SDKs are usable and working:

1. MicroPython - https://github.com/micropython/micropython
2. Arduino IDE - https://www.arduino.cc/en/software/
3. Native ESP-IDF - https://github.com/espressif/esp-idf

# 6. Operating System

Before continuing, here is a short remark about RTOSes (real-time operating systems). In **embedded** multi-core systems, it is advisable to use such a layer to separate, for example, the *Wi-Fi / network stack* from *application code* into **layers / controllable tasks** to improve stability and reduce programming mistakes.

> [!NOTE]
> In a single-core microcontroller system, such a design can be counterproductive.

The Espressif ESP-IDF framework integrates *FreeRTOS* into all boards, including the *ESP32-C3* **single-core** devices. This introduces moderate overhead. The implementation details, including advantages and disadvantages of the different SDK approaches, are discussed in the following sections.

> [!NOTE]
> Using an embedded Linux operating system is only advisable for **much larger** systems where significantly more high-speed peripherals (for example PCIe, multiple 100 GbE NICs, or multiple GPU / DisplayPort devices) must be coordinated.

# 7. Global Advantages / Disadvantages

Each of the following SDK variants has advantages and disadvantages and is aimed at programmers with different skill levels.

## 7.1. Advantages

First, here is a quick overview of useful features included in all SDK variants:

- lwIP (lightweight IP stack) including IPv6
- Network interface abstraction / IP routing between interfaces
- Berkeley sockets / abstraction layer for TCP and UDP
- Mbed TLS (Transport Layer Security and X.509 certificate handling)
- Memory region **partitioning** / minimal virtual filesystem integration
- Extensive and stable libraries for peripheral handling (I2C, GPIO, UART, PWM, SPI)

## 7.2. Disadvantages

There are quite a few global disadvantages. The first is the lack of *example networking code* at higher OSI layers, and the second is *exorbitant bloat* in default setups.

Many people start development with the native *Arduino IDE*, which is very easy to set up and can be ready for use within minutes after installing the correct board extensions. When compiling the first lines of a simple **sketch** (Arduino project identifier), the first impression is usually that compile times are slow and the resulting flash image is large.

I tested a simple Wi-Fi access point implementation with minimal TCP server processing, which resulted in a 1 MB flash image. This is sufficient for the ESP32-C3, but still heavily bloated (details follow below).

# 8. MicroPython

MicroPython is the choice for the least-experienced C / C++ developer. There is no need **at all** to write C / C++ code.

But how exactly is MicroPython integrated into the microcontroller? It is *not* **only** a Python interpreter compiled for microcontrollers. MicroPython is *integrated* into the ESP32 RTOS subsystem, bootloader, and main loop. In practice, this means that when the microcontroller boots, the Python interpreter and *additional* **control code** are loaded into the upper memory regions, initialized, started, and then run continuously.

Depending on the microcontroller used, this enables uploading, running, flashing, and compiling Python code to bytecode without reflashing the complete firmware. An interpreter console CLI is also accessible over serial to control the interpreter at runtime.

Regarding performance and firmware size optimization, MicroPython allows additional Python modules to be integrated as *pre-compiled bytecode* (virtual filesystem partition) or even *frozen bytecode* (directly into the firmware blob). This makes runtime compilation obsolete and improves performance. As a practical example, controlling a common I2C SSD1306 OLED display using the Python `machine` and `ssd1306` library will not make any noticeable difference compared to a C / C++ implementation.

## 8.1. Disadvantages

Upper-layer networking **server** libraries (including many RPC mechanisms) are **not** easy to implement for inexperienced developers and usually require a significant amount of code. In addition, many protocols and libraries are bloated with features that increase complexity and firmware size.

> [!NOTE]
> Our project modifies the MicroPython implementation by replacing MicroPython's control logic with a **very simplified** C++ HTTP/1.1 TLS-capable web server (with unnecessary HTTP features removed). On an HTTP POST request with a JSON payload, it executes a MicroPython script with that payload.

# 9. ESP-IDF

The IoT Development Framework (ESP-IDF) is more hardware-centric than the Arduino IDE. The application code—**setup** and **main loop**—must be implemented as *FreeRTOS* tasks, so it is more complex than the Arduino IDE.

Inside the IDF, many components are much better designed, and using the framework *feels* much nicer from a developer's point of view. Also, the customization process in version 6.1 is very good, giving the developer much better control over included functionality and allowing significantly smaller firmware images.

> [!NOTE]
> We chose ESP-IDF as the development environment, and the project code also demonstrates how to work with advanced ESP-IDF features.

## 9.1. Advantages

The ESP-IDF framework is much more flexible than the Arduino IDE. Including external C / C++ libraries and writing your own libraries both follow current development practices / standards.

### 9.1.1. Kconfig

The famous *Kconfig* framework, invented by the Linux kernel for customizing kernel builds, is used for multiple purposes:

1. customize **global** library functions (for example disable IPv6 in the lwIP stack)
2. customize **own** features (for example disable HTTPS)

This is done by mapping compiler preprocessor directives inside a custom `/main/Kconfig.projbuild` definition file.

By calling `idf.py menuconfig`, a classic ncurses-based configuration menu appears where settings can be selected and customized.

### 9.1.2. CMake Integration

To provide a very flexible component customization system, IDF uses an *enhanced* CMake-based build system controlled by the internal `idf.py` command. The firmware link and flash process is also controlled by the `idf.py` script, which makes customized automation possible.

## 9.2. Disadvantages

In contrast to the *arduino-esp32* C++ libraries, IDF only provides libraries written in pure C. Due to the IDF's CMake integration, you are free to choose between the following development approaches:

1. Exclusively use the IDF C libraries (providing suboptimal programming interfaces)
2. Link C++ libraries from *arduino-esp32* (often with unnecessary overhead)
3. Combine IDF C and *arduino-esp32* C++ libraries (which bloats firmware image size)
4. Write your own C++ libraries using the IDF C libraries (best choice for keeping firmware images small)

> [!NOTE]
> Because our project embeds MicroPython, our HTTPS web server code already contains working C++ classes, and optional web server content must also be packed into firmware: option 4 seems to be the right choice.

# 10. Documentation / Examples

The Espressif *ESP32* (Sphinx-generated) documentation and the examples section under `/examples` provide extensive development information.

> [!WARNING]
> Some higher-level networking examples (especially HTTP) are horrible and should be avoided, our web server implementation will use the lwIP Berkeley socket API and provide *clean code* (similar to the ESP-IDF's C++11 threading example, which is written at a high-quality programming level).

Nevertheless, we will provide an excerpt of the most important *ESP32-C3* links for you:

- [Get started](https://docs.espressif.com/projects/esp-idf/en/v6.0/esp32c3/get-started/index.html)
- [Hardware Reference](https://docs.espressif.com/projects/esp-idf/en/v6.0/esp32c3/hw-reference/index.html#hardware-reference)
- [Build System](https://docs.espressif.com/projects/esp-idf/en/v6.0/esp32c3/api-guides/build-system.html)
- [C++ support](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-guides/cplusplus.html)
