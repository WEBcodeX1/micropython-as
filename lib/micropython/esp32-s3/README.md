# Install Instructions ESP32-S3

1. Clone patched MicroPython repository
2. Generate *Embed Port* subdir
3. Adjust cross-compiler settings
4. Build static MicroPython library

*Requirements*

- Current git
- CMake >= 3.16
- ESP-IDF framework cross-compilers (gcc, g++, binutils)

## 1. Clone MicroPython Repository

Clone the *patched* MicroPython library from GitHub which adds code / functionality to execute MicroPython functions from C++ directly.

```bash
mkdir -p ~/repos && cd ~/repos
git clone https://github.com/clauspruefer/micropython.git
```

> [!WARNING]
> The repository **must** be cloned into `~/repos/micropython`, otherwise you must adjust the path in `micropython_embed.mk`.

## 2. Generate Embed Port

After cloning, the first step is to generate *compilable* source code adapted for the given architecture.

> [!WARNING]
> The MicroPython repository base dir must point to a valid location in `micropython_embed.mk`, correct this before continuing.

> [!NOTE]
> Configuration for the architecture is defined in `./mpconfigport.h`.

The following command will generate the subdirs `micropython_embed` and `build-embed`, afterwards the source code is ready to be compiled.

```bash
make -f ./mpconfigport.h
```

## 3. Adjust Compiler Settings

In `xtensa-cross.cmake` the cmake cross-compiler setttings will be defined.

> [!WARNING]
> You **must** adjust the compiler settings in `xtensa-cross.cmake`; `CMAKE_C_COMPILER`, `CMAKE_CXX_COMPILER` and `CMAKE_FIND_ROOT_PATH` must point to valid paths and working compiler executables.

After adjusting the settings to the correct paths (ESP-IDF must be already installed to `~/.espressif` correctly), you are ready to compile the source code.

## 4. Build / Install Library

Compile the source code with the following cmake command:

```bash
cmake -DCMAKE_TOOLCHAIN_FILE=xtensa-cross.cmake .
make
```

Install the static library (`/usr/local/lib/esp32s3/libmicropython.a`) and header file (`/usr/local/include/esp32s3/micropython_embed.h`) as root user.

```bash
sudo make install
```
