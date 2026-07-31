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

# ---------------------------------------------------------------------------
# Include paths
# Stubs directory comes first so it shadows ESP-IDF / FreeRTOS headers.
# The FalconAS headers are provided by the system include directory above.
# ---------------------------------------------------------------------------
include_directories(
    stubs
    ../src/components/network_oop
    ../src/components/filesystem
    ../src/main
    ${HTTP_INCLUDE_DIR}
)

# ---------------------------------------------------------------------------
# Executable
# ---------------------------------------------------------------------------
add_executable(server_linux ${SOURCES})

target_link_libraries(server_linux PRIVATE
    ${HTTPPARSER_LIBRARY}
)

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
    set(TEST_FS_NUM_FILES 200 CACHE STRING "Number of generated Linux test files")
    set(TEST_FS_MIN_SIZE 10240 CACHE STRING "Minimum generated Linux test file size in bytes")
    set(TEST_FS_MAX_SIZE 1048576 CACHE STRING "Maximum generated Linux test file size in bytes")
    set(GENERATED_FS_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated_fs)
    set(TEST_FS_GENERATOR ${CMAKE_CURRENT_SOURCE_DIR}/test/filesystem/generate_test_headers.py)

    find_package(Python3 COMPONENTS Interpreter REQUIRED)

    execute_process(
        COMMAND ${CMAKE_COMMAND} -E make_directory ${GENERATED_FS_DIR}
    )

    execute_process(
        COMMAND
            ${Python3_EXECUTABLE}
            ${TEST_FS_GENERATOR}
            ${GENERATED_FS_DIR}
            --count ${TEST_FS_NUM_FILES}
            --min-size ${TEST_FS_MIN_SIZE}
            --max-size ${TEST_FS_MAX_SIZE}
        RESULT_VARIABLE GENERATE_TEST_HEADERS_RESULT
    )

    if(NOT GENERATE_TEST_HEADERS_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to generate Linux test filesystem headers")
    endif()

    add_custom_command(
        OUTPUT
            ${GENERATED_FS_DIR}/filedata-test.h
            ${GENERATED_FS_DIR}/filemetadata-test.h
        COMMAND ${CMAKE_COMMAND} -E make_directory ${GENERATED_FS_DIR}
        COMMAND ${Python3_EXECUTABLE} ${TEST_FS_GENERATOR} ${GENERATED_FS_DIR} --count ${TEST_FS_NUM_FILES} --min-size ${TEST_FS_MIN_SIZE} --max-size ${TEST_FS_MAX_SIZE}
        DEPENDS ${TEST_FS_GENERATOR}
        COMMENT "Generating Linux test filesystem headers"
        VERBATIM
    )

    add_custom_target(generate_test_filesystem_headers
        DEPENDS
            ${GENERATED_FS_DIR}/filedata-test.h
            ${GENERATED_FS_DIR}/filemetadata-test.h
    )

    add_subdirectory(test)
endif()
