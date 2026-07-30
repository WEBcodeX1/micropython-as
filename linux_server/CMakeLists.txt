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
# HTTP library – built from source via the WEBcodeX1/http-1.2 submodule.
# The submodule lives at linux_server/third_party/http-1.2.
# Its lib/http/CMakeLists.txt creates a single static target named "httpparser"
# that contains both the parser and the generator compilation units.
# ---------------------------------------------------------------------------
add_subdirectory(third_party/http-1.2/lib/http)
target_include_directories(httpparser PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/http-1.2/lib/http)

# ---------------------------------------------------------------------------
# Include paths
# Stubs directory comes first so it shadows ESP-IDF / FreeRTOS headers.
# The http-1.2 headers are exposed via the httpparser target above.
# ---------------------------------------------------------------------------
include_directories(
    stubs
    ../src/components/network_oop
    ../src/components/filesystem
    ../src/main
)

# ---------------------------------------------------------------------------
# Executable
# ---------------------------------------------------------------------------
add_executable(server_linux ${SOURCES})

target_link_libraries(server_linux PRIVATE httpparser)

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

# ---------------------------------------------------------------------------
# Optional: build the stability test suite (server_test + test_client).
# Activate by passing -DBUILD_TESTS=ON to cmake.
# See linux_server/test/README.md for details.
# ---------------------------------------------------------------------------
option(BUILD_TESTS "Build the stability test server and test client" OFF)
if(BUILD_TESTS)
    add_subdirectory(test)
endif()
