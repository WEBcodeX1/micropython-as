# Building / Requirements

This document describes the build process and requirements.

## 1. Operating System

We recommend using a standard Debian-based Linux distribution. I have tested compilation on Devuan (Debian-based) and on my current Ubuntu Hardened 25.10 system.

## 2. Cabling

Make sure you also have a working USB-C or USB-C-to-USB cable / adapter with power and data lines that supports:

- Battery charging (when soldered to the battery pins on the back side of the board)
- Serial communication for:
  - firmware flashing
  - JTAG / serial debugging over USB

## 3. Packages

Python 3, pip3, and CMake must be installed before proceeding.

```bash
apt-get install python3 python3-pip cmake
```

> [!WARNING]
> You will also need the `falcon-as` and `micropython` libraries at a later stage of the project. We will keep you informed and update this documentation.

## 4. ESP IoT Development Framework Features

A short overview of the framework features before continuing with the installation process.

- Cross-compiler for C **and** C++ (gcc, g++) with automatic language detection
- Integrated (extended) CMake framework / component management system
- Excellent component management for multiple boards / chip types
- Automated ELF-to-firmware image conversion / linking
- Command-line firmware flashing

## 5. Install the ESP IoT Development Framework

The installation process is one of the easiest I have ever encountered. Programming, compiling, and external component integration are also very straightforward **without** losing any flexibility.

```bash
# install packages as root user
sudo apt-get install python3 python3-pip cmake
```

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

The original MicroPython repository does not include support for building libraries (shared or static) for embedding the interpreter.
I have submitted a pull request with an example showing how to do this. Until the PR has been accepted (or declined), static library building can be obtained from my MicroPython fork.

> [!NOTE]
> Be sure to check out the correct `v1.26-release` branch.

> [!NOTE]
> Adjust the cross-compiler settings / paths in `riscv32-cross.cmake`.

```bash
git clone https://github.com/clauspruefer/micropython
git checkout v1.26-release
make -f micropython_embed.mk
cmake -DCMAKE_TOOLCHAIN_FILE=riscv32-cross.cmake
make
sudo make install
```

This installs the static library to `/usr/local/lib/esp32c3/libmicropython.a` and the MicroPython header file to `/usr/local/include/esp32c3/micropython_embed.h`.

## 8. HTTPParser Library

For the HTTP/1.1 parser, the parser from the project https://github.com/WEBcodeX1/http-1.2 will be used. The `arduino/esp32c3` port already includes support for building a static library, which can be included in our `micropython-as` project.

> [!NOTE]
> Adjust the cross-compiler settings / paths in `riscv32-cross.cmake`.

```bash
git clone https://github.com/WEBcodeX1/http-1.2
cd ./http-1.2/ports/arduino/esp32c3/
cmake -DCMAKE_TOOLCHAIN_FILE=riscv32-cross.cmake
make
sudo make install
```

This installs the static library to `/usr/local/lib/esp32c3/libhttpparser.a` and the HTTPParser header file to `/usr/local/include/esp32c3/httpparser.hpp`.

## 9. Compiling / Flashing

Change to the `micropython-as` repository, compile it, and then flash it to the microcontroller:

```bash
cd ~/src/micropython-as/src
idf.py build
idf.py flash
```

> [!NOTE]
> The flash and board RAM settings will probably need to be adjusted either through `idf.py menuconfig` or by using `esptool` with modified command-line parameters.
