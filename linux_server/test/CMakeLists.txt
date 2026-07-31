cmake_minimum_required(VERSION 3.16)
project(server_test_suite CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_definitions(LINUX_BUILD)

if(NOT DEFINED GENERATED_FS_DIR OR NOT TARGET generate_test_filesystem_headers)
    message(FATAL_ERROR "Configure ../CMakeLists.txt with -DBUILD_TESTS=ON to build linux_server/test")
endif()

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

add_dependencies(server_test generate_test_filesystem_headers)

target_include_directories(server_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/filesystem  # test Filesystem.hpp — must be first
    ${GENERATED_FS_DIR}
    ${LS}/stubs
    ${NET_OOP}
)

# ---------------------------------------------------------------------------
# HTTP libraries – use the system-installed Linux FalconAS headers and static
# libraries under /usr/local as documented in BUILD.md §10.
# ---------------------------------------------------------------------------
set(HTTP_SYSTEM_PREFIX "/usr/local" CACHE PATH
    "Base path containing the Linux FalconAS HTTP headers and static libraries")

set(HTTP_INCLUDE_SEARCH_PATHS
    ${HTTP_SYSTEM_PREFIX}/include/linux
    ${HTTP_SYSTEM_PREFIX}/include
)

set(HTTP_LIBRARY_SEARCH_PATHS
    ${HTTP_SYSTEM_PREFIX}/lib/linux
    ${HTTP_SYSTEM_PREFIX}/lib
    ${HTTP_SYSTEM_PREFIX}/libs/linux
    ${HTTP_SYSTEM_PREFIX}/libs
)

find_path(HTTP_INCLUDE_DIR
    NAMES httpparser.hpp httpgenerator.hpp httpconstants.hpp
    PATHS ${HTTP_INCLUDE_SEARCH_PATHS}
    NO_DEFAULT_PATH
    REQUIRED)

find_library(HTTPPARSER_LIBRARY
    NAMES httpparser
    PATHS ${HTTP_LIBRARY_SEARCH_PATHS}
    NO_DEFAULT_PATH
    REQUIRED)

target_include_directories(server_test PRIVATE ${HTTP_INCLUDE_DIR})
target_link_libraries(server_test PRIVATE
    ${HTTPPARSER_LIBRARY}
)

# ---------------------------------------------------------------------------
# test_client binary
#
# Self-contained POSIX client; no FalconAS or ESP-IDF dependencies.
# ---------------------------------------------------------------------------
add_executable(test_client test_client.cpp)
target_link_libraries(test_client PRIVATE pthread)
target_compile_definitions(test_client PRIVATE
    TEST_NUM_FILES=${TEST_FS_NUM_FILES}
    TEST_MIN_SIZE=${TEST_FS_MIN_SIZE}
    TEST_MAX_SIZE=${TEST_FS_MAX_SIZE}
)

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
