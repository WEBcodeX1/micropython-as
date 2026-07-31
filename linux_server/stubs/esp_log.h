#pragma once

// Stub ESP logging API for Linux builds

#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <iomanip>
#include <iostream>

inline constexpr int ESP_LOG_ERROR = 1;
inline constexpr int ESP_LOG_WARN  = 2;
inline constexpr int ESP_LOG_INFO  = 3;

namespace esp_log_stub {

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

inline void writeFormatted(const int level, const char *tag, const char *format, va_list args)
{
    char buffer[2048];
    const char *safeTag = (tag != nullptr) ? tag : "<null>";
    const char *safeFormat = (format != nullptr) ? format : "<null format>";

    va_list argsCopy;
    va_copy(argsCopy, args);
    std::vsnprintf(buffer, sizeof(buffer), safeFormat, argsCopy);
    va_end(argsCopy);

    std::cout << '[' << levelName(level) << "][" << safeTag << "] " << buffer << '\n';
    std::cout.flush();
}

inline void logBufferHex(const char *tag, const void *buffer, const std::size_t length, const int level)
{
    const char *safeTag = (tag != nullptr) ? tag : "<null>";
    std::cout << '[' << levelName(level) << "][" << safeTag << ']';

    if(buffer == nullptr)
    {
        std::cout << " <null>\n";
        std::cout.flush();
        return;
    }

    const auto *bytes = static_cast<const std::uint8_t *>(buffer);
    std::ios_base::fmtflags oldFlags = std::cout.flags();
    char oldFill = std::cout.fill();

    for(std::size_t index = 0; index < length; ++index)
    {
        std::cout << ' ' << std::hex << std::setw(2) << std::setfill('0')
                  << static_cast<unsigned int>(bytes[index]);
    }

    std::cout.flags(oldFlags);
    std::cout.fill(oldFill);
    std::cout << '\n';
    std::cout.flush();
}

inline void logBufferChar(const char *tag, const void *buffer, const std::size_t length, const int level)
{
    const char *safeTag = (tag != nullptr) ? tag : "<null>";
    std::cout << '[' << levelName(level) << "][" << safeTag << ']';

    if(buffer == nullptr)
    {
        std::cout << " <null>\n";
        std::cout.flush();
        return;
    }

    const auto *chars = static_cast<const char *>(buffer);
    for(std::size_t index = 0; index < length; ++index)
    {
        std::cout << chars[index];
    }

    std::cout << '\n';
    std::cout.flush();
}

struct LogCallable
{
    int Level;

    void operator()(const char *tag, const char *format, ...) const
    {
        va_list args;
        va_start(args, format);
        writeFormatted(Level, tag, format, args);
        va_end(args);
    }
};

struct BufferHexCallable
{
    void operator()(const char *tag, const void *buffer, const std::size_t length, const int level) const
    {
        logBufferHex(tag, buffer, length, level);
    }
};

struct BufferCharCallable
{
    void operator()(const char *tag, const void *buffer, const std::size_t length, const int level) const
    {
        logBufferChar(tag, buffer, length, level);
    }
};

}

inline constexpr esp_log_stub::LogCallable ESP_LOGI{ESP_LOG_INFO};
inline constexpr esp_log_stub::LogCallable ESP_LOGW{ESP_LOG_WARN};
inline constexpr esp_log_stub::LogCallable ESP_LOGE{ESP_LOG_ERROR};
inline constexpr esp_log_stub::BufferHexCallable ESP_LOG_BUFFER_HEX_LEVEL{};
inline constexpr esp_log_stub::BufferCharCallable ESP_LOG_BUFFER_CHAR_LEVEL{};
