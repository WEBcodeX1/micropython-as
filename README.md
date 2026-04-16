# Embedding MicroPython in a C++ HTTP/1.1 Application Server

This project embeds MicroPython in Arduino-based ESP32 microcontrollers using the **Falcon-AS** C++ HTTP/1.1 web server.

Refer to [./BUILD.md](./BUILD.md) to learn how to build the project.

> [!NOTE]
> You might ask yourself: What exactly is this approach useful for, and how does it differ from the existing MicroPython ESP32 integration? We provide illustrative diagrams in the [10. Diagrams](#10-diagrams) section.

# 1. ESP32-C3 SoC

[Espressif Systems](https://www.espressif.com/) developed the **ESP32-C3** system-on-chip, based on a 32-bit single-core RISC-V CPU. Ready-to-use boards are available from *Seeed Studio* (XIAO ESP32-C3 mini board) at an affordable price point (5€ :moneybag::star2:).

## 1.1. Base Features

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

## 1.2. Detailed Hardware Specs

For detailed hardware information, start with the manufacturer's wiki: https://wiki.seeedstudio.com/XIAO_ESP32C3_Getting_Started/.

# 2. Variants

There are also several notable variants.

## 2.1. ESP32-S3

ESP32-S3R8 Xtensa LX7 dual-core 240 MHz system with optional external HAT extensions, for example a camera or audio add-on.

> [!NOTE]
> The current ESP-IDF integrates a *transparent* C++11 threading implementation that makes code execution across multiple CPU cores much easier :heart_eyes:.

## 2.2. ESP32-P4

A high-performance, highly secure board (**without** integrated Wi-Fi or Bluetooth), with attachable MicroSD card and PHY Ethernet (IP101GR).

# 3. Development Frameworks

With this amount of system memory, development should be straightforward. However, there are multiple pitfalls. This document discusses how to choose the **correct** SDK (software development kit) from the available options for your requirements.

The following SDKs are usable and working:

1. MicroPython - https://github.com/micropython/micropython
2. Arduino IDE - https://www.arduino.cc/en/software/
3. Native ESP-IDF - https://github.com/espressif/esp-idf

# 4. Operating System

Before continuing, here is a short remark about RTOSes (real-time operating systems). In **embedded** multi-core systems, it is advisable to use such a layer to separate, for example, the *Wi-Fi / network stack* from *application code* into **layers / controllable tasks** to improve stability and reduce programming mistakes.

> [!NOTE]
> In a single-core microcontroller system, such a design can be counterproductive.

The Espressif ESP-IDF framework integrates *FreeRTOS* into all boards, including the *ESP32-C3* **single-core** devices. This introduces moderate overhead. The implementation details, including advantages and disadvantages of the different SDK approaches, are discussed in the following sections.

> [!NOTE]
> Using an embedded Linux operating system is only advisable for **much larger** systems where significantly more high-speed peripherals (for example PCIe, multiple 100 GbE NICs, or multiple GPU / DisplayPort devices) must be coordinated.

# 5. Global Advantages / Disadvantages

Each of the following SDK variants has advantages and disadvantages and is aimed at programmers with different skill levels.

## 5.1. Advantages

First, here is a quick overview of useful features included in all SDK variants:

- lwIP (lightweight IP stack) including IPv6
- Network interface abstraction / IP routing between interfaces
- Berkeley sockets / abstraction layer for TCP and UDP
- Mbed TLS (Transport Layer Security and X.509 certificate handling)
- Memory region **partitioning** / minimal virtual filesystem integration
- Extensive and stable libraries for peripheral handling (I2C, GPIO, UART, PWM, SPI)

## 5.2. Disadvantages

There are quite a few global disadvantages. The first is the lack of *example networking code* at higher OSI layers, and the second is *exorbitant bloat* in default setups.

Many people start development with the native *Arduino IDE*, which is very easy to set up and can be ready for use within minutes after installing the correct board extensions. When compiling the first lines of a simple **sketch** (Arduino project identifier), the first impression is usually that compile times are slow and the resulting flash image is large.

I tested a simple Wi-Fi access point implementation with minimal TCP server processing, which resulted in a 1 MB flash image. This is sufficient for the ESP32-C3, but still heavily bloated (details follow below).

# 6. MicroPython

MicroPython is the choice for the least-experienced C / C++ developer. There is no need **at all** to write C / C++ code.

But how exactly is MicroPython integrated into the microcontroller? It is *not* **only** a Python interpreter compiled for microcontrollers. MicroPython is *integrated* into the ESP32 RTOS subsystem, bootloader, and main loop. In practice, this means that when the microcontroller boots, the Python interpreter and *additional* **control code** are loaded into the upper memory regions, initialized, started, and then run continuously.

Depending on the microcontroller used, this enables uploading, running, flashing, and compiling Python code to bytecode without reflashing the complete firmware. An interpreter console CLI is also accessible over serial to control the interpreter at runtime.

Regarding performance and firmware size optimization, MicroPython allows additional Python modules to be integrated as *pre-compiled bytecode* (virtual filesystem partition) or even *frozen bytecode* (directly into the firmware blob). This makes runtime compilation obsolete and improves performance. As a practical example, controlling a common I2C SSD1306 OLED display using the Python `machine` and `ssd1306` library will not make any noticeable difference compared to a C / C++ implementation.

## 6.1. Disadvantages

Upper-layer networking **server** libraries (including many RPC mechanisms) are **not** easy to implement for inexperienced developers and usually require a significant amount of code. In addition, many protocols and libraries are bloated with features that increase complexity and firmware size.

> [!NOTE]
> Our project modifies the MicroPython implementation by replacing MicroPython's control logic with a **very simplified** C++ HTTP/1.1 TLS-capable web server (with unnecessary HTTP features removed). On an HTTP POST request with a JSON payload, it executes a MicroPython script with that payload.

# 7. Arduino IDE

The Arduino IDE is a lightweight development environment focused on external libraries written in C++.
The developer can choose between two IDE versions: a newer *modern* AppImage (version 2) or a *traditional installable* version (version 1).

On my hardened Ubuntu 25.10 desktop, the AppImage (version 2) tends to be unusable. I also found it difficult to see how external libraries could be integrated efficiently with this AppImage-based approach. I tested version 1, which is written in Java, and it works fine.

In the first step, I only had to add the external Git URL to configure the correct "Additional Boards Manager URL" (https://jihulab.com/esp-mirror/espressif/arduino-esp32.git) in the IDE preferences and then install the `esp32` package from the Board Manager.

In the last step, select a) `XIAO_ESP32C3` as the board type, b) `/dev/ttyACM0` as the serial interface, and c) serial baud rate `115200` to communicate with the microcontroller.

With the following simple LED GPIO code, you can verify the IDE setup and confirm that the microcontroller executes your compiled binary correctly (note that the ESP32C3 does not include a programmable LED on the board, so to see the LED blink, an LED with the correct resistor must be soldered to the correct board pin). Even without a soldered LED, the program will execute and print debug output over USB serial when you start the IDE's serial monitor.

```c++
// define led GPIO
const int led = D10; // there is no LED_BUILTIN available for the XIAO ESP32C3.

void setup() {
  // initialize serial output
  Serial.begin(115200);
  Serial.println("Setup starting.");

  // initialize digital pin led as an output
  pinMode(led, OUTPUT);
}

void loop() {
  Serial.println("LED switch on.");
  digitalWrite(led, HIGH);   // turn the LED on
  delay(1000);               // wait for a second
  Serial.println("LED switch off.");
  digitalWrite(led, LOW);    // turn the LED off
  delay(1000);               // wait for a second
}
```

> [!NOTE]
> Under the **Sketch** menu, select **Verify/Compile** to compile the sketch and, after compiling, select **Upload** to flash **and** run the code. If no LED is soldered, the code will still run and output the `println()` messages through **Tools/Serial Monitor** (USB serial).

## 7.1. Advantages

The Arduino IDE is well suited for small projects with **no need** to integrate advanced external C / C++ libraries.

All provided libraries make a solid impression. The C++ abstractions / interfaces are well defined and follow modern C++ practices. It is also easy to include external libraries with the library manager (**Tools/Manage Libraries**).

The option to integrate your own libraries exists. I did not pursue it further because the Arduino IDE gave the impression that integrating a custom library is non-trivial. Several internet searches also suggest that direct use of the ESP-IDF (IoT Development Framework) is much more hardware-oriented.

## 7.2. Disadvantages

Including only the `WiFi.h` (Arduino IDE native C++ implementation) headers, without any further application code, will result in a 1 MB firmware image. There is **no** *easy* way to configure the libraries to omit features (for example disable IPv6 or TLS).

## 7.3. Repository

The ESP32 Arduino C++ library is maintained on GitHub at https://github.com/espressif/arduino-esp32 and is usable regardless of the Arduino IDE. You should consider this as a selection criterion for your SDK.

## 7.4. Library Builder Tool

Espressif provides a **library builder** to customize library functionality at https://github.com/espressif/esp32-arduino-lib-builder. This is a *complex* process compared to ESP-IDF integration and should also be considered carefully as an SDK selection criterion.

> [!NOTE]
> I was able to customize and compile with the library builder, but I was **not** able to integrate it correctly into the Arduino IDE. The compilation process still used the previous libraries, without firmware size improvements.

# 8. ESP-IDF

The IoT Development Framework (ESP-IDF) is more hardware-centric than the Arduino IDE. The application code—**setup** and **main loop**—must be implemented as *FreeRTOS* tasks, so it is more complex than the Arduino IDE.

Inside the IDF, many components are much better designed, and using the framework *feels* much nicer from a developer's point of view. Also, the customization process in version 6.1 is very good, giving the developer much better control over included functionality and allowing significantly smaller firmware images.

> [!NOTE]
> We chose ESP-IDF as the development environment, and the project code also demonstrates how to work with advanced ESP-IDF features.

## 8.1. Advantages

The ESP-IDF framework is much more flexible than the Arduino IDE. Including external C / C++ libraries and writing your own libraries both follow current development practices / standards.

### 8.1.1. Kconfig

The famous *Kconfig* framework, invented by the Linux kernel for customizing kernel builds, is used for multiple purposes:

1. customize **global** library functions (for example disable IPv6 in the lwIP stack)
2. customize **own** features (for example disable HTTPS)

This is done by mapping compiler preprocessor directives inside a custom `/main/Kconfig.projbuild` definition file.

By calling `idf.py menuconfig`, a classic ncurses-based configuration menu appears where settings can be selected and customized.

### 8.1.2. CMake Integration

To provide a very flexible component customization system, IDF uses an *enhanced* CMake-based build system controlled by the internal `idf.py` command. The firmware link and flash process is also controlled by the `idf.py` script, which makes customized automation possible.

## 8.2. Disadvantages

In contrast to the *arduino-esp32* C++ libraries, IDF only provides libraries written in pure C. Due to the IDF's CMake integration, you are free to choose between the following development approaches:

1. Exclusively use the IDF C libraries (providing suboptimal programming interfaces)
2. Link C++ libraries from *arduino-esp32* (often with unnecessary overhead)
3. Combine IDF C and *arduino-esp32* C++ libraries (which bloats firmware image size)
4. Write your own C++ libraries using the IDF C libraries (best choice for keeping firmware images small)

> [!NOTE]
> Because our project embeds MicroPython, our HTTPS web server code already contains working C++ classes, and optional web server content must also be packed into firmware: option 4 seems to be the right choice.

# 9. System Engineering

I am still learning the ESP32-C3 chip and memory layout, peripheral design, and RTOS internals. I am still not sure whether the **complete** RTOS code is integrated into the IDF and whether every firmware compilation also rebuilds the complete RTOS code.

The following projects provide a bit more insight into the microcontroller boot process and RISC-V assembler instruction set:

- https://github.com/espressif/esp32c3-direct-boot-example
- https://projectf.io/posts/riscv-cheat-sheet/

# 10. Diagrams

Diagrams will be added later.

# 11. Documentation / Examples

The Espressif ESP32 (Sphinx-generated) documentation and the examples section under `/examples` provide extensive development information.

> [!WARNING]
> Some higher-level networking examples (especially HTTP) are horrible and should be avoided, our web server implementation will use the lwIP Berkeley socket API and provide *clean code* (similar to the ESP-IDF's C++11 threading example, which is written at a high-quality programming level).

Nevertheless, we will provide an excerpt of the most important *ESP32-C3* links for you:

- [Get started](https://docs.espressif.com/projects/esp-idf/en/v6.0/esp32c3/get-started/index.html)
- [Hardware Reference](https://docs.espressif.com/projects/esp-idf/en/v6.0/esp32c3/hw-reference/index.html#hardware-reference)
- [Build System](https://docs.espressif.com/projects/esp-idf/en/v6.0/esp32c3/api-guides/build-system.html)
- [C++ support](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-guides/cplusplus.html)
