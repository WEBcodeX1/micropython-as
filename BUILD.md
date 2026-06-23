# Building / Requirements

This document describes the build process and requirements.

## 1. Operating System

We recommend using a standard Debian-based Linux distribution. I have tested compilation on Devuan (Debian-based) and on my current Ubuntu Hardened 25.10 system.

## 2. Cabling

Make sure you also have a working USB-C or USB-C-to-USB cable / adapter with power and data lines that supports:

- Serial communication for *firmware flashing*
- Serial communication for *JTAG / serial debugging over USB*

## 3. Requirements

The ESP32-IDF Development Framework (see [point 5](#5--install-esp-idf)), Python 3, pip3, and CMake must be installed before proceeding.

```bash
apt-get install python3 python3-pip cmake
```

### 3.1. External Includes

The following external static libraries and C++ header files—including a *patched* MicroPython—must be cross-compiled for the matching destination architecture. Proper instructions for the **ESP32-C3** and **ESP32-S3** boards are included.

The list below provides an overview of exactly what these libraries are used for. Detailed instructions can be found inside the `./lib/` folder and later in this document:

1. A **patched** MicroPython to execute MicroPython functions directly from C++ code (see [point 7](#7--micropython))
2. A HTTP parsing library from the FalconAS project to parse HTTP/1.1 messages (see [point 8](#8--http-libraries))
3. A HTTP message generator from the FalconAS project to generate HTTP/1.1 messages (see [point 8](#8--http-libraries))

## 4. ESP IoT Development Framework Features

A short overview of the framework features before continuing with the installation process.

- Cross-compiler for C **and** C++ (gcc, g++) with automatic language detection
- Integrated (extended) CMake framework / component management system
- Excellent component management for multiple boards / chip types
- Automated ELF-to-firmware image conversion / linking
- Command-line firmware flashing

## 5. Install ESP-IDF

The installation process is one of the easiest I have ever encountered. Programming, compiling, and external component integration are also very straightforward **without** losing any flexibility.

Clone and install ESP-IDF as a development user (non-root).

```bash
# clone git repository
git clone https://github.com/espressif/esp-idf.git

# install idf as non-root user
cd ./esp-idf
./install.sh
```

## 6. Activating the Build Environment

After installation, the build environment must be activated (sourced) from the current installation path.

```bash
. ./export.sh
```

If the installation process completed successfully, the following text should be displayed:

```txt
Done! You can now compile ESP-IDF projects.
```

## 7. MicroPython

The original MicroPython repository lacks direct, out-of-the-box support for cross-compiling libraries (shared or static) to embed the interpreter into external projects. Until my pending pull request is resolved, a static library building example using CMake with an external cross-compiler is available from my MicroPython GitHub Fork at https://github.com/clauspruefer/micropython/examples/embedding-staticlib.

It is *more important*: my fork also adds the `mp_embed_exec_string_function(char* function_name, char* function_param_value)` function. This enables direct C/C++ calls to MicroPython functions with a single string (JSON) parameter, which is used internally to pass JSON messages from the C++ application server layer to the running MicroPython interpreter / PONG structures.

Additionally, note that the ESP32-C3 (RISCV32) and ESP32-S3 (Xtensa) are completely different architectures and require specific *compiler adjustments* and `mpconfigport.h` settings. The `./lib/micropython/$architecture` directory includes settings and compile instructions for both architectures.

> [!WARNING]
> The ESP-IDF framework including crosscompilers for ESP32-C3 and ESP32-S3 (installed to your $HOME dir) is required to produce working binaries.

> [!NOTE]
> Be sure to a) clone the repository to ~/repos/micropython (default in `micropython_embed.mk`) **and** b) check out the correct MicroPython `v1.26-release` branch.

> [!NOTE]
> Cross-compiler settings for the relevant architecture are provided in `./lib/micropython/` (ESP32-C3 or ESP32-S3).

After executing the relevant architecture installer script, the static library will be installed to `/usr/local/lib/$architecture/libmicropython.a` and the MicroPython header file to `/usr/local/include/$architecture/micropython_embed.h`.

## 8. HTTP Libraries

For the HTTP/1.1 parser and application server, the parser and the message generator from the project https://github.com/WEBcodeX1/http-1.2 will be used.

Under `/ports/arduino/` installation instructions for board types ESP32-C3 and ESP32-S3 can be found.

This installs the static libraries to `/usr/local/lib/$architecture/` and the header files to `/usr/local/include/$architecture/`.

## 9. Compiling / Flashing

Change to the `micropython-as` repository, select your board type, compile it, and then flash it to the microcontroller.

> [!WARNING]
> The current default board, including settings, is **ESP32-S3**, not **ESP32-C3**. There are also **hardcoded** *GPIO* settings inside the C++ header files that you currently must change manually. Also note that the **ESP32-C3** MicroPython interpreter currently crashes on floating-point arithmetic.

```bash
cd ~/src/micropython-as/src
idf.py set-target esp32s3
idf.py build
idf.py flash
```

> [!NOTE]
> RAM (heap and stack) settings are already adusted for **ESP32-S3**, settings might be adjusted for **ESP32-C3**.
