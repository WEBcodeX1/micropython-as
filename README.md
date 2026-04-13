# Embedded Micropython into a C++ HTTP/1.1 Application Server

This project embeds Micropython into Arduino ESP32 based microcontrollers using the Falcon-AS C++ HTTP/1.1 Web-Server.

# ESP32-C3 SoC

[Espressive Systems](https://www.espressif.com/) recently developed a masterpiece of hardware: the **ESP32-C3** System on Chip, based on a 32-bit RISC-V (single core) CPU. Ready to use boards are distributed by *Seedstudio* (XIAO ESP32-C3 MINI Board) for an amazing price of apx. 5€ :moneybag::star2:.

**Base Features**

- IEEE 802.11 b/g/n WiFi
- Bluetooth 5 (BLE)
- 14 external usable PINs
- Battery connector PINs
- USB-C **Protected** Battery Loading
- USB-C Firmware / JTAG port
- USB-C Serial Debugging 
- Internal Temperature Sensor
- Low-Power / Sleep-State Control
- 4 Megabyte Memory

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

1. Micropython
2. Arduino IDE
3. Native ESP-IDF

# RTO

Before continuing, a very short remark about RTOs (Real Time Operating Systems). In **embedded** multi-core systems it is advisable to use such an entity to seperate e.g. the *WiFi / Network Stack* from *Application Code* into **Layers / Controlable Tasks** to guarantee stability / prevent programming mistakes by the programmer.

> [!NOTE]
> In a single-core system such a design can be contra-productive.

The Espressif ESP-IDF framework integrates *FreeRTOS* into all boards also for the *ESP32-C3* **single-core**. This comes with a moderate overhead, we will discuss implementation details (advantages / disadvantages) between the different SDK approaches in the next chapters.

# Advantages / Disadvantages

# Chip Engineering
