cmake_minimum_required(VERSION 3.16)
project(server_linux CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Mark this as a Linux-native build so platform guards in the source files are activated.
add_compile_definitions(LINUX_BUILD)

# ---------------------------------------------------------------------------
# Source files (server component only — no WiFi, MicroPython, display, LED)
# ---------------------------------------------------------------------------
set(SOURCES
    main.cpp
    ../src/components/network_oop/Server.cpp
    ../src/components/network_oop/ClientHandler.cpp
    ../src/components/network_oop/Client.cpp
    ../src/components/network_oop/ASRequestHandler.cpp
    ../src/components/filesystem/Filesystem.cpp
)

# ---------------------------------------------------------------------------
# Include paths
# Stubs directory comes first so it shadows ESP-IDF / FreeRTOS headers.
# ---------------------------------------------------------------------------
include_directories(
    stubs
    ../src/components/network_oop
    ../src/components/filesystem
    ../src/main
    /usr/local/include/linux
)

# ---------------------------------------------------------------------------
# Executable
# ---------------------------------------------------------------------------
add_executable(server_linux ${SOURCES})

# ---------------------------------------------------------------------------
# External HTTP libraries (Linux-native builds from the FalconAS http-1.2 port)
# Install instructions: see BUILD.md section "Linux Server Build".
# Expected locations after installation:
#   /usr/local/lib/linux/libhttpparser.a
#   /usr/local/lib/linux/libhttpgenerator.a
# ---------------------------------------------------------------------------
add_library(httpparser_lib STATIC IMPORTED)
set_property(TARGET httpparser_lib PROPERTY
    IMPORTED_LOCATION /usr/local/lib/linux/libhttpparser.a)

add_library(httpgenerator_lib STATIC IMPORTED)
set_property(TARGET httpgenerator_lib PROPERTY
    IMPORTED_LOCATION /usr/local/lib/linux/libhttpgenerator.a)

target_link_libraries(server_linux PRIVATE httpparser_lib httpgenerator_lib)

# ---------------------------------------------------------------------------
# Optional: enable AddressSanitizer for runtime crash/memory analysis.
# Activate by passing -DASAN=ON to cmake.
# ---------------------------------------------------------------------------
option(ASAN "Build with AddressSanitizer" OFF)
if(ASAN)
    target_compile_options(server_linux PRIVATE -fsanitize=address -fno-omit-frame-pointer -g)
    target_link_options(server_linux PRIVATE -fsanitize=address)
endif()

# ---------------------------------------------------------------------------
# Optional: enable ThreadSanitizer for data-race detection.
# Activate by passing -DTSAN=ON to cmake.
# ---------------------------------------------------------------------------
option(TSAN "Build with ThreadSanitizer" OFF)
if(TSAN)
    target_compile_options(server_linux PRIVATE -fsanitize=thread -fno-omit-frame-pointer -g)
    target_link_options(server_linux PRIVATE -fsanitize=thread)
endif()
