cmake_minimum_required(VERSION 3.16)
project(server_test_suite CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_definitions(LINUX_BUILD)

# ---------------------------------------------------------------------------
# Paths
# ---------------------------------------------------------------------------
set(ROOT    ${CMAKE_CURRENT_SOURCE_DIR}/../..)
set(NET_OOP ${ROOT}/src/components/network_oop)
set(LS      ${CMAKE_CURRENT_SOURCE_DIR}/..)

# ---------------------------------------------------------------------------
# server_test binary
#
# Compiles the same server sources as server_linux, but uses the dynamic
# test filesystem (linux_server/test/filesystem/) instead of the production
# embedded-data filesystem from src/components/filesystem/.
#
# IMPORTANT: test/filesystem/ must appear BEFORE src/components/filesystem/
# in the include path so that #include "Filesystem.hpp" in ClientHandler.cpp
# resolves to the test override.
# ---------------------------------------------------------------------------
set(SERVER_SOURCES
    ${LS}/main.cpp
    ${NET_OOP}/Server.cpp
    ${NET_OOP}/ClientHandler.cpp
    ${NET_OOP}/Client.cpp
    ${NET_OOP}/ASRequestHandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filesystem/Filesystem.cpp
)

add_executable(server_test ${SERVER_SOURCES})

target_include_directories(server_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/filesystem  # test Filesystem.hpp — must be first
    ${LS}/stubs
    ${NET_OOP}
)

# ---------------------------------------------------------------------------
# HTTP library – built from source via the WEBcodeX1/http-1.2 submodule.
# Guard prevents redefinition when this file is included from the parent build.
# ---------------------------------------------------------------------------
if(NOT TARGET httpparser)
    add_subdirectory(${LS}/third_party/http-1.2/lib/http
                     ${CMAKE_CURRENT_BINARY_DIR}/http-1.2)
    target_include_directories(httpparser PUBLIC
        ${LS}/third_party/http-1.2/lib/http)
endif()

target_link_libraries(server_test PRIVATE httpparser)

# ---------------------------------------------------------------------------
# test_client binary
#
# Self-contained POSIX client; no FalconAS or ESP-IDF dependencies.
# ---------------------------------------------------------------------------
add_executable(test_client test_client.cpp)
target_link_libraries(test_client PRIVATE pthread)

# ---------------------------------------------------------------------------
# Optional sanitizer pass-through (inherit from parent or set explicitly)
# ---------------------------------------------------------------------------
option(ASAN "Build with AddressSanitizer" OFF)
if(ASAN)
    foreach(tgt server_test test_client)
        target_compile_options(${tgt} PRIVATE -fsanitize=address -fno-omit-frame-pointer -g)
        target_link_options(${tgt} PRIVATE -fsanitize=address)
    endforeach()
endif()

option(TSAN "Build with ThreadSanitizer" OFF)
if(TSAN)
    foreach(tgt server_test test_client)
        target_compile_options(${tgt} PRIVATE -fsanitize=thread -fno-omit-frame-pointer -g)
        target_link_options(${tgt} PRIVATE -fsanitize=thread)
    endforeach()
endif()
