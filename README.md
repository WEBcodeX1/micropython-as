# Embedded Micropython into a C++ HTTP/1.1 Application Server

This project embeds Micropython into Arduino ESP32 based microcontrollers using the Falcon-AS C++ HTTP/1.1 Web-Server.

# ESP32-C3 SoC

[Espressive Systems](https://www.espressif.com/) recently developed a masterpiece of hardware: the **ESP32-C3** System on Chip, based on a 32-bit RISC-V (single core) CPU. Ready to use boards are distributed by *Seedstudio* (XIAO ESP32-C3 MINI Board) for an amazing price of apx. 5€ :moneybag::star2:.

**Base Features**

- IEEE 802.11 b/g/n WiFi
- Bluetooth 5 (BLE)
- 14 external usable PINs
- Hardware Cryptography (RSA, ECC, AES)
- Battery connector PINs
- USB-C **Protected** Battery Loading
- USB-C Firmware / JTAG port
- USB-C Serial Debugging 
- Internal Temperature Sensor
- Low-Power / Sleep-State Control
- 400KB SRAM and 4MB on-board flash memory

To get detailed (hardware) information, you should take a first look at the manufacturers wiki: https://wiki.seeedstudio.com/XIAO_ESP32C3_Getting_Started/.

# Variants

Also some remarkable variants get manufactured.

## ESP32-S3

ESP32-S3R8 Xtensa LX7 dual-core 240 MHz system with optional external HAT extension, e.g. camera and microphone / audio.

## ESP32-P4

A high-performant, high secure (**without** integrated Wifi and Bluetooth) board, Micro-SD Card and PHY Ethernet (IP101GR) attachable.

# Developing Frameworks

With such a masterpiece of hardware including *lots of system memory* developing should be fun you might think. **But** there are multiple pitfalls we will discuss and dive a bit deeper how to chose the **correct** SDK (software development kit) among **multiple** provided SDKs for your specific requirements.

Following, an overview of usable and working SDKs:

1. Micropython - https://github.com/micropython/micropython
2. Arduino IDE - https://www.arduino.cc/en/software/
3. Native ESP-IDF - https://github.com/espressif/esp-idf

# Operating System

Before continuing, a very short remark about RTOs (Real Time Operating Systems). In **embedded** multi-core systems it is advisable to use such an entity to seperate e.g. the *WiFi / Network Stack* from *Application Code* into **Layers / Controlable Tasks** to guarantee stability / prevent programming mistakes by the developer.

> [!NOTE]
> In a single-core microcontroller system such a design can be contra-productive.

The Espressif ESP-IDF framework integrates *FreeRTOS* into all boards also for the *ESP32-C3* **single-core**. This comes with a moderate overhead, we will discuss implementation details (advantages / disadvantages) between the different SDK approaches in the next chapters.

> [!NOTE]
> Using an embedded Linux operating system is only advisable for **much bigger** microcontrollers where much more high-speed peripherals (e.g. PCIe, Multi 100GbE NICs, Multi GPU/Display Port) must be coordinated. 

# Advantages / Disadvantages

Each of the following SDK variant has its advantages and disadvantages / is aimed for programmers with different skills.

## Global Advantages

First of all a quick overview of useful features included in all SDK variants:

- lwIP (lightweight IP stack) including IPv6
- Networking interface abstraction / IP routing between interfaces
- Berkley networking sockets / abstraction layer for TCP and UDP
- Mbed TLS (Transport Layer Security and X.509 certificate handling)
- Memory region **partitioning** / minimal virtual filesystem integration
- Extensive and stable libraries for peripheral handling (I2C, GPIO, UART, PWM, SPI)

## Global Disadvantages

There are a quite few global disadvantages, the first is *example networking code* in higher OSI layers and the second one is *extra-orbitant high bloating* in default setups.

Many people start developing using the native *Arduino IDE* which is very easy to set up and ready to use in minutes after installing the correct board extensions. When compiling the first lines of simple **sketch** (Arduino project identifier) code the first impression is: compile times are slow and the resulting binary flash image is large.

I tested a simple WiFi access point code including minimalistic tcp server processing which resulted in a 1MB flash image. Sufficient for the ESP32-C3 but heavily bloated (details following).

## Micropython

Micropython is the choice for the least-experienced C / C++ developer. There is no need **at all** using a single line of C / C++ code.

But how exactly is Micropython integrated into the microcontroller? It is *not* **only** a Python interpreter compiled for microcontrollers. Micropython will be *integrated* into the ESP32 RTOS subsystem / bootloader / main loop. This exactly means when the microcontroller is starting (booting) the Python interpreter and *additional* **control code** will be loaded into the upper memory regions, initialized, started and permanently *looped*.

Depending of the microcontroller used, this enables uploading / running / flashing / compiling Python code to bytecode without the need of re-flashing the complete firmware. Also an interpreter console CLI is accessible via serial to control the interpreter in runtime.

Regarding performance and firmware size optimization: Micropython allows additional Python modules to be integrated as *pre-compiled bytecode* (virtual filesystem partition) or even *frozen bytecode* (directly into firmware blob) which makes runtime compilation obsolete and thus improves performance. As practical example: controlling a common I2C OLED 1306 display using the Python `machine` and `ssd1306` library will not make any noticeable difference compared to a C / C++ implementation.

**Disadvantages**

Upper layer networking **server** libraries (many RPC mechanisms) are **not** easy to implement for unskilled developers and need quite a lot of implementation code. Also many protocols and libraries are bloated with features which increase complexity and resulting firmware size. 

> [!NOTE]
> Our project modifies the Micropython implementation by replacing the Micropythons control logic with a **very simplifyed** C++ HTTP/1.1 TLS capable webserver (all unneccessary HTTP features removed) which on HTTP POST request with a JSON payload will execute a Micropython script with the given JSON payload. 

# Chip Engineering
