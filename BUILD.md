# Building / Requirements

The following document describes the building process and requirements.

## 1. Operating System

We recommend a working standard Linux distribution, i tested compiling on Devuan (Debian based) and on my current Ubuntu Hardened 25.10 LTE system.

## 2. Cabling

Make sure that you also have a working USB-C or USB-C to USB cable / adapter with **loading** and **communication** pins which enables:

- Battery loading (when soldered to battery pins on board back-side)
- Serial communication a) firmware flashing b) JTAG / serial debugging over USB

## 3. Packages / Libraries

Python3, PIP3 and CMake must be installed before further proceeding.

```bash
apt-get install python3 python3-pip cmake
```

> [!WARNING]
> You will also need the falcon-as libraries in a later stage of the project, we will keep you informed / update this documentation.

## 4. ESP IoT Development Framework Features

A short overview over the framework features before continuing with the installation process.

- Cross compiler for C **and** C++ (gcc, g++) with automatic language detection
- Integrated (extended) CMake framework / component management system
- Extraordinary good component management for multiple boards / chip types
- Automated ELF to firmware image conversion / linkage
- Command line firmware flashing

## 5. Install ESP IoT Development Framework

The installation process is the easiest installation i have ever encountered, also programming / compiling and external component integration is very handsome **without** losing any flexibility.

```bash
# install packages as root user
sudo apt-get install python3 python3-pip cmake
```

Clone and install the ESP-IDF as development user (non-root).

```bash
# clone git repository
git clone https://github.com/espressif/esp-idf.git

# install idf as non-root user
cd ./esp-idf
./install.sh
```

## 6. Activating Build Environment

After installing, the build environment must be activated (sourced) from the current installation path.

```bash
. ./export.sh
```

If the installation process went fine, the following text should be displayed:

```txt
Done! You can now compile ESP-IDF projects.
```

## 8. Compiling / Flashing

Change to the micropython-as repository, compile and afterwards flash to the microcontroller:

```bash
cd ~/src/micropython-as/src
idf.py build
idf.py flash
```

> [!NOTE]
> The flash / board RAM settings probably must be adusted by either `idf.py menuconfig` or using the flash `esptool` with modified command line parameters.
