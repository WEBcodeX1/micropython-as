# Building / Requirements

This document describes the build process and requirements.

## 1. Operating System

We recommend using a standard Debian-based Linux distribution. Compilation has been tested on *Devuan* (Debian-based) and on
an *Ubuntu Hardened 26.04 LTS* system.

## 2. Cabling

Make sure you also have a working USB-C or USB-C-to-USB cable and all peripherals connected correctly according to [PINOUT.md](PINOUT.md).

## 3. Requirements

The *ESP-IDF* Development Framework (see [section 5](#5-esp-idf-installation)), `Python 3`, `python3-venv`, `pip3`, and `CMake` must
be installed before proceeding.

```bash
apt-get install python3 python3-venv python3-pip cmake
```

### 3.1. External Includes

The following external static libraries and C++ header files—including a *patched* *MicroPython* **embed port**—must be cross-compiled
for the matching target architecture. Instructions for the **ESP32-C3** and **ESP32-S3** boards are linked below.

The list below provides an overview of exactly what these libraries are used for.

1. A *patched* *MicroPython* **embed port** (MicroPython sources remain untouched) to execute *MicroPython* functions directly from C++ code (see [section 7](#7-cross-compiling-micropython))
2. An HTTP/1.1 *parsing* and *message generation* library from the *NLAP/FalconAS* project to parse and generate HTTP/1.1 messages (see [section 8](#8-cross-compiling-http-libraries))

## 4. ESP-IDF Framework Features

A short overview of the framework features before continuing with the installation process.

- Cross-compiler for C **and** C++ (gcc, g++) with automatic language detection
- Integrated (extended) CMake framework / component management system
- Excellent component management for multiple boards / chip types
- Automated ELF-to-firmware image conversion / linking
- Command-line firmware flashing

## 5. ESP-IDF Installation

The installation process is one of the easiest I have ever encountered. Programming, compiling, and external component integration
are also very straightforward **without** losing any flexibility.

Clone and install *ESP-IDF* as a development user (non-root).

```bash
# clone repository
cd ~/src
git clone https://github.com/espressif/esp-idf.git

# install idf as non-root user
cd ~/src/esp-idf
./install.sh
```

## 6. ESP-IDF Build Environment

After installation, the build environment must be activated (sourced) from the current installation path.

```bash
. ./export.sh
```

If the installation process completed successfully, the following text should be displayed:

```txt
Done! You can now compile ESP-IDF projects.
```

## 7. Cross-Compiling MicroPython

The standard *MicroPython* repository does not support out-of-the-box cross-compilation to a static library for embedding into external projects.
A dedicated fork at [clauspruefer/micropython](https://github.com/clauspruefer/micropython/tree/embedding) (branch `embedding`) provides CMake-based
build examples that produce `libmicropython.a` for each *ESP32* target architecture using the *ESP-IDF* cross-compiler toolchain.

This fork also adds the `mp_embed_exec_string_function(char* function_name, char* function_param_value)` function, which enables direct C/C++ calls
to *MicroPython* functions with a single string (JSON) parameter. This is used internally to pass JSON messages from the C++ application server layer
to the running *MicroPython* interpreter / PONG structures.

The **ESP32-C3** (RISC-V RV32IMC) and **ESP32-S3** (Xtensa LX7) are completely different architectures and require separate build steps.
Detailed per-variant instructions are available at:

- **ESP32-S3**: https://github.com/clauspruefer/micropython/blob/embedding/examples/embedding/esp32/s3/README.md
- **ESP32-C3**: https://github.com/clauspruefer/micropython/blob/embedding/examples/embedding/esp32/c3/README.md

### 7.1. Clone Repository

It is required to clone the dedicated fork repository using the following commands:

```
cd ~/src
git clone https://github.com/clauspruefer/micropython.git
cd ./micropython
git checkout embedding
```

### 7.2. ESP32-S3 (Xtensa LX7)

```bash
# Activate ESP-IDF environment
cd ~/src/esp-idf
. ./export.sh

# Change to the ESP32-S3 embedding example
cd ~/src/micropython/examples/embedding/esp32/s3/

# Generate a CMake-compatible toolchain setup from the current user environment
./adjust-cross-build.sh

# Prepare / generate MicroPython sources
make -f micropython_embed.mk

# Configure with the Xtensa cross-compiler toolchain
cmake -DCMAKE_TOOLCHAIN_FILE=xtensa-cross.cmake

# Build and install
make -j2
sudo make install
```

After installation the static library and header are placed at:

- Library: `/usr/local/lib/esp32s3/libmicropython.a`
- Header: `/usr/local/include/esp32s3/micropython_embed.h`

### 7.3. ESP32-C3 (RISC-V)

```bash
# Activate ESP-IDF environment
cd src/esp-idf
. ./export.sh

# Change to the ESP32-C3 embedding example
cd src/micropython/examples/embedding/esp32/c3/

# Generate a CMake-compatible toolchain setup from the current user environment
./adjust-cross-build.sh

# Prepare / generate MicroPython sources
make -f micropython_embed.mk

# Configure with the RISC-V cross-compiler toolchain
cmake -DCMAKE_TOOLCHAIN_FILE=riscv32-cross.cmake

# Build and install
make -j2
sudo make install
```

After installation the static library and header are placed at:

- Library: `/usr/local/lib/esp32c3/libmicropython.a`
- Header: `/usr/local/include/esp32c3/micropython_embed.h`

## 8. Cross-Compiling HTTP Libraries

The HTTP/1.1 parser and message-generator libraries from the [WEBcodeX1/http-1.2](https://github.com/WEBcodeX1/http-1.2) project
must also be cross-compiled for the target *ESP32* architecture before building `micropython-as`.

Detailed instructions for each board type are available under the `ports/arduino/` subdirectory of that repository:

- **ESP32-S3**: https://github.com/WEBcodeX1/http-1.2/blob/main/ports/arduino/esp32s3/README.md
- **ESP32-C3**: https://github.com/WEBcodeX1/http-1.2/blob/main/ports/arduino/esp32c3/README.md

### 8.1. ESP32-S3 (Xtensa LX7)

```bash
# Activate ESP-IDF environment (if not already active)
cd ~/src/esp-idf
. ./export.sh

# Change to the ESP32-S3 Arduino port of http-1.2
cd ~/src/http-1.2/ports/arduino/esp32s3/

# Generate a CMake-compatible toolchain setup from the current user environment
./adjust-cross-build.sh

# Configure with the Xtensa cross-compiler toolchain
cmake -DCMAKE_TOOLCHAIN_FILE=xtensa-cross.cmake .

# Build and install
make
sudo make install
```

After installation the static library and headers are placed at:

- Library: `/usr/local/lib/esp32s3/libhttpparser.a`
- Headers: `/usr/local/include/esp32s3/` (`httpparser.hpp`, `httpgenerator.hpp`)

### 8.2. ESP32-C3 (RISC-V)

```bash
# Activate ESP-IDF environment (if not already active)
cd src/esp-idf
. ./export.sh

# Change to the ESP32-C3 Arduino port of http-1.2
cd src/http-1.2/ports/arduino/esp32c3/

# Generate a CMake-compatible toolchain setup from the current user environment
./adjust-cross-build.sh

# Configure with the RISC-V cross-compiler toolchain
cmake -DCMAKE_TOOLCHAIN_FILE=riscv32-cross.cmake .

# Build and install
make
sudo make install
```

After installation the static library and headers are placed at:

- Library: `/usr/local/lib/esp32c3/libhttpparser.a`
- Headers: `/usr/local/include/esp32c3/` (`httpparser.hpp`, `httpgenerator.hpp`)

## 9. Compiling / Flashing

Change to the `micropython-as` repository, select your board type, compile it, and then flash it to the microcontroller.

> [!WARNING]
> The current default board, including settings, is **ESP32-S3**, not **ESP32-C3**. There are also **hardcoded** *GPIO*
> settings inside the C++ header files that you currently must change manually. Also note that the **ESP32-C3** *MicroPython*
> interpreter currently crashes on floating-point arithmetic.

```bash
cd ~/src/micropython-as/src
idf.py set-target esp32s3
idf.py build
idf.py flash
```

> [!NOTE]
> RAM (heap and stack) settings are already adjusted for **ESP32-S3**, it may be necessary to adjust these for **ESP32-C3**.

## 10. Linux Server Build (Stability Testing)

The HTTP server component can be compiled and run as a **native Linux binary** for stability and crash analysis — without any
*ESP32* hardware, *ESP-IDF*, or *FreeRTOS*. This enables the use of tools such as `gdb`, Valgrind, and the Clang/GCC sanitizers
(AddressSanitizer, ThreadSanitizer) to pinpoint crashes and timeout-related bugs.

Only the following components are compiled:

- `Server` / `ClientHandler` / `Client` (TCP connection handling)
- `ASRequestHandler` / `ASRequestDef` (*MicroPython* request routing stubs)
- `Filesystem` (static file serving from embedded data)

WiFi, DNS, LED, display, and MicroPython execution are **not** included.

### 10.1. Prerequisites

The same HTTP parser and generator libraries from the FalconAS project are required, but built for the **host Linux architecture**
instead of a microcontroller cross-target.

Follow the instructions in the [WEBcodeX1/http-1.2](https://github.com/WEBcodeX1/http-1.2) repository under `ports/linux/` and install
the resulting static libraries and headers:

```
/usr/local/lib/linux/libhttpparser.a      or /usr/local/libs/libhttpparser.a
/usr/local/lib/linux/libhttpgenerator.a   or /usr/local/libs/libhttpgenerator.a
/usr/local/include/linux/httpparser.hpp   or /usr/local/include/httpparser.hpp
/usr/local/include/linux/httpgenerator.hpp or /usr/local/include/httpgenerator.hpp
/usr/local/include/linux/httpconstants.hpp or /usr/local/include/httpconstants.hpp
```

### 10.2. Build

```bash
cd ~/src/micropython-as/linux_server
cmake -B build
cmake --build build
```

This build produces `build/server_linux`, `build/dns_server_linux`, and the Linux test binaries together. `server_linux` listens on **port 8080** by default. An optional IPv4 listen address can be passed as the first argument (default: `0.0.0.0`):

```bash
./build/server_linux 127.0.0.1
```

To run the standalone DNS responder from a separate build directory:

```bash
cd ~/src/micropython-as/linux_server
cmake -B build-dns
cmake --build build-dns
./build-dns/dns_server_linux 127.0.0.1 53535
```

### 10.3. Background Start / Stop Script

For repeated stability tests, `linux_server/run_server.sh` can start the server
in the background, stop it again, and optionally capture stdout/stderr to a log
file:

```bash
cd ~/src/micropython-as/linux_server

# start in background without logging
./run_server.sh start

# start in background on loopback and save logs to a file
./run_server.sh start --host 127.0.0.1 --log --log-file ./build/server_linux.log

# check whether the server is running
./run_server.sh status

# stop the background server
./run_server.sh stop
```

By default, the script expects the binary at `linux_server/build/server_linux`
and stores the PID in `linux_server/build/server_linux.pid`.

### 10.4. Sanitizer Builds

For detailed crash and memory analysis, enable the AddressSanitizer or ThreadSanitizer at configure time:

```bash
# AddressSanitizer (detects buffer overflows, use-after-free, …)
cmake -B build -DASAN=ON
cmake --build build

# ThreadSanitizer (detects data races)
cmake -B build -DTSAN=ON
cmake --build build
```

### 10.5. Valgrind

```bash
valgrind --tool=memcheck --leak-check=full ./build/server_linux
```
