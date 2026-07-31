cmake_minimum_required(VERSION 3.16)
project(server_test_suite CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_definitions(LINUX_BUILD)


# ---------------------------------------------------------------------------
# Paths
# ---------------------------------------------------------------------------
set(ROOT    ${CMAKE_CURRENT_SOURCE_DIR}/../..)
set(LS      ${CMAKE_CURRENT_SOURCE_DIR}/..)

# ---------------------------------------------------------------------------
# test_client binary
#
# Self-contained POSIX client; no FalconAS or ESP-IDF dependencies.
# ---------------------------------------------------------------------------
add_executable(test_client test_client.cpp)
target_include_directories(test_client PRIVATE ${GENERATED_FS_DIR})
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
