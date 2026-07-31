#pragma once

// Stub ESP logging API for Linux builds

#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <iomanip>
#include <iostream>

inline void ESP_LOGI(const char *tag, const char *format, ...)
{
    const char *safeTag = (tag != nullptr) ? tag : "<null>";
    const char *safeFormat = (format != nullptr) ? format : "<null format>";
    char buffer[2048];

    va_list args;
    va_start(args, format);
    std::vsnprintf(buffer, sizeof(buffer), safeFormat, args);
    va_end(args);

    std::cout << "[INFO][" << safeTag << "] " << buffer << '\n';
    std::cout.flush();
}

inline void ESP_LOGW(const char *tag, const char *format, ...)
{
    const char *safeTag = (tag != nullptr) ? tag : "<null>";
    const char *safeFormat = (format != nullptr) ? format : "<null format>";
    char buffer[2048];

    va_list args;
    va_start(args, format);
    std::vsnprintf(buffer, sizeof(buffer), safeFormat, args);
    va_end(args);

    std::cout << "[WARN][" << safeTag << "] " << buffer << '\n';
    std::cout.flush();
}

inline void ESP_LOGE(const char *tag, const char *format, ...)
{
    const char *safeTag = (tag != nullptr) ? tag : "<null>";
    const char *safeFormat = (format != nullptr) ? format : "<null format>";
    char buffer[2048];

    va_list args;
    va_start(args, format);
    std::vsnprintf(buffer, sizeof(buffer), safeFormat, args);
    va_end(args);

    std::cout << "[ERROR][" << safeTag << "] " << buffer << '\n';
    std::cout.flush();
}

inline void ESP_LOG_BUFFER_HEX_LEVEL(const char *tag, const void *buffer, const std::size_t length, int)
{
    const char *safeTag = (tag != nullptr) ? tag : "<null>";
    std::cout << "[HEX][" << safeTag << ']';

    if(buffer == nullptr)
    {
        std::cout << " <null>\n";
        std::cout.flush();
        return;
    }

    const auto *bytes = static_cast<const std::uint8_t *>(buffer);
    const std::ios_base::fmtflags oldFlags = std::cout.flags();
    const char oldFill = std::cout.fill();

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

inline void ESP_LOG_BUFFER_CHAR_LEVEL(const char *tag, const void *buffer, const std::size_t length, int)
{
    const char *safeTag = (tag != nullptr) ? tag : "<null>";
    std::cout << "[CHAR][" << safeTag << ']';

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
