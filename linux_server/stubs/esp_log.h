#pragma once

// Stub ESP logging macros for Linux builds

#include <cstddef>
#include <cstdint>
#include <cstdio>

namespace esp_log_stub {

inline constexpr int ESP_LOG_ERROR = 1;
inline constexpr int ESP_LOG_WARN  = 2;
inline constexpr int ESP_LOG_INFO  = 3;

inline constexpr const char *levelName(const int level)
{
    switch(level)
    {
        case ESP_LOG_ERROR:
            return "ERR";
        case ESP_LOG_WARN:
            return "WARN";
        case ESP_LOG_INFO:
        default:
            return "INFO";
    }
}

inline void logBufferHexLevel(const char *tag, const void *buf, const std::size_t len, const int level)
{
    std::printf("[%s][%s]", levelName(level), tag);

    if(buf == nullptr)
    {
        std::printf(" <null>\n");
        return;
    }

    const auto *bytes = static_cast<const std::uint8_t *>(buf);
    for(std::size_t i = 0; i < len; ++i)
    {
        std::printf(" %02X", bytes[i]);
    }

    std::printf("\n");
}

}

#define ESP_LOG_ERROR esp_log_stub::ESP_LOG_ERROR
#define ESP_LOG_WARN esp_log_stub::ESP_LOG_WARN
#define ESP_LOG_INFO esp_log_stub::ESP_LOG_INFO

#define ESP_LOGI(tag, fmt, ...) \
    std::printf("[INFO][" tag "] " fmt "\n", ##__VA_ARGS__)

#define ESP_LOGW(tag, fmt, ...) \
    std::printf("[WARN][" tag "] " fmt "\n", ##__VA_ARGS__)

#define ESP_LOGE(tag, fmt, ...) \
    std::fprintf(stderr, "[ERR][" tag "] " fmt "\n", ##__VA_ARGS__)

#define ESP_LOG_BUFFER_HEX_LEVEL(tag, buf, len, level) \
    do { esp_log_stub::logBufferHexLevel(tag, buf, len, level); } while(0)

#define ESP_LOG_BUFFER_CHAR_LEVEL(tag, buf, len, level) \
    do {} while(0)
