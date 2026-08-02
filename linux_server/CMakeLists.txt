cmake_minimum_required(VERSION 3.16)
project(server_linux CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Mark this as a Linux-native build so platform guards in the source files are activated.
add_compile_definitions(LINUX_BUILD)

# ---------------------------------------------------------------------------
# Build options
# ---------------------------------------------------------------------------
option(BUILD_HTTP_SERVER "Build the Linux HTTP server" ON)
option(BUILD_DNS_SERVER "Build the Linux DNS server" ON)

# ---------------------------------------------------------------------------
# Shared include directories
# ---------------------------------------------------------------------------
set(LINUX_COMMON_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/stubs
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/components/network_oop
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/main
)

set(LINUX_SERVER_INCLUDE_DIRS
    ${LINUX_COMMON_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/filesystem
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/components/filesystem
)

set(SANITIZER_TARGETS)

# ---------------------------------------------------------------------------
# HTTP server executable
# ---------------------------------------------------------------------------
if(BUILD_HTTP_SERVER)
    set(HTTP_SERVER_SOURCES
        main.cpp
        ../src/components/network_oop/Server.cpp
        ../src/components/network_oop/ClientHandler.cpp
        ../src/components/network_oop/Client.cpp
        ../src/components/network_oop/ASRequestHandler.cpp
        ../src/components/filesystem/Filesystem.cpp
    )

    # -----------------------------------------------------------------------
    # HTTP libraries – use the system-installed Linux FalconAS headers and
    # static libraries under /usr/local as documented in BUILD.md §10.
    # -----------------------------------------------------------------------
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

    add_executable(server_linux ${HTTP_SERVER_SOURCES})
    target_include_directories(server_linux PRIVATE
        ${LINUX_SERVER_INCLUDE_DIRS}
        ${HTTP_INCLUDE_DIR}
    )
    target_link_libraries(server_linux PRIVATE ${HTTPPARSER_LIBRARY})
    list(APPEND SANITIZER_TARGETS server_linux)
endif()

# ---------------------------------------------------------------------------
# DNS server executable
# ---------------------------------------------------------------------------
if(BUILD_DNS_SERVER)
    add_executable(dns_server_linux
        main_dns.cpp
        ../src/components/network_oop/DNSServer.cpp
    )
    target_include_directories(dns_server_linux PRIVATE ${LINUX_COMMON_INCLUDE_DIRS})
    list(APPEND SANITIZER_TARGETS dns_server_linux)
endif()

# ---------------------------------------------------------------------------
# Optional: enable AddressSanitizer for runtime crash/memory analysis.
# Activate by passing -DASAN=ON to cmake.
# ---------------------------------------------------------------------------
option(ASAN "Build with AddressSanitizer" OFF)
if(ASAN)
    foreach(tgt ${SANITIZER_TARGETS})
        target_compile_options(${tgt} PRIVATE -fsanitize=address -fno-omit-frame-pointer -g)
        target_link_options(${tgt} PRIVATE -fsanitize=address)
    endforeach()
endif()

# ---------------------------------------------------------------------------
# Optional: enable ThreadSanitizer for data-race detection.
# Activate by passing -DTSAN=ON to cmake.
# ---------------------------------------------------------------------------
option(TSAN "Build with ThreadSanitizer" OFF)
if(TSAN)
    foreach(tgt ${SANITIZER_TARGETS})
        target_compile_options(${tgt} PRIVATE -fsanitize=thread -fno-omit-frame-pointer -g)
        target_link_options(${tgt} PRIVATE -fsanitize=thread)
    endforeach()
endif()

# ---------------------------------------------------------------------------
# Optional: build the stability test suite (server_test + test_client).
# Activate by passing -DBUILD_TESTS=ON to cmake.
# See linux_server/test/README.md for details.
# ---------------------------------------------------------------------------
option(BUILD_TESTS "Build the stability test server and test client" ON)
set(LINUX_FILESYSTEM_OVERRIDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/filesystem)
set(LINUX_FILESYSTEM_OVERRIDE_HEADER ${LINUX_FILESYSTEM_OVERRIDE_DIR}/Filesystem.hpp)

if(BUILD_TESTS AND NOT BUILD_HTTP_SERVER)
    message(FATAL_ERROR "BUILD_TESTS requires BUILD_HTTP_SERVER=ON")
endif()

if(BUILD_TESTS)
    set(GENERATED_FS_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated_fs)
    set(TEST_FS_GENERATOR ${CMAKE_CURRENT_SOURCE_DIR}/generate_test_headers.py)
    set(SOURCE_FILESYSTEM_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/../src/components/filesystem/Filesystem.hpp)

    if(TARGET server_linux)
        target_include_directories(server_linux PRIVATE ${GENERATED_FS_DIR})
    endif()

    find_package(Python3 COMPONENTS Interpreter REQUIRED)

    execute_process(
        COMMAND ${CMAKE_COMMAND} -E make_directory ${GENERATED_FS_DIR}
    )

    file(MAKE_DIRECTORY ${LINUX_FILESYSTEM_OVERRIDE_DIR})
    file(READ ${SOURCE_FILESYSTEM_HEADER} FILESYSTEM_HEADER_CONTENT)
    string(REPLACE "#include \"filedata.h\"" "#include \"filedata-test.h\""
        FILESYSTEM_HEADER_CONTENT "${FILESYSTEM_HEADER_CONTENT}")
    string(REPLACE "#include \"filemetadata.h\"" "#include \"filemetadata-test.h\""
        FILESYSTEM_HEADER_CONTENT "${FILESYSTEM_HEADER_CONTENT}")
    file(WRITE ${LINUX_FILESYSTEM_OVERRIDE_HEADER} "${FILESYSTEM_HEADER_CONTENT}")

    execute_process(
        COMMAND
            ${Python3_EXECUTABLE}
            ${TEST_FS_GENERATOR}
            ${GENERATED_FS_DIR}
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
        COMMAND
            ${Python3_EXECUTABLE}
            ${TEST_FS_GENERATOR}
            ${GENERATED_FS_DIR}
        DEPENDS ${TEST_FS_GENERATOR}
        COMMENT "Generating Linux test filesystem headers"
        VERBATIM
    )

    add_subdirectory(test)
else()
    file(REMOVE ${LINUX_FILESYSTEM_OVERRIDE_HEADER})
endif()
