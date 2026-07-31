#pragma once

// Stub ESP logging API for Linux builds

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <ostream>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>

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

inline void appendLiteral(std::ostringstream &stream, const char *format)
{
    if(format == nullptr)
    {
        stream << "<null format>";
        return;
    }

    while(*format != '\0')
    {
        if((format[0] == '%') && (format[1] == '%'))
        {
            stream << '%';
            format += 2;
            continue;
        }

        stream << *format;
        ++format;
    }
}

template<typename TValue>
void appendValue(std::ostringstream &stream, const char specifier, TValue &&value)
{
    using DecayedValue = std::decay_t<TValue>;

    switch(specifier)
    {
        case 's':
            if constexpr(std::is_convertible_v<DecayedValue, const char *>)
            {
                const char *text = static_cast<const char *>(value);
                stream << (text != nullptr ? text : "<null>");
            }
            else
            {
                stream << std::forward<TValue>(value);
            }
            break;

        case 'c':
            if constexpr(std::is_integral_v<DecayedValue>)
            {
                stream << static_cast<char>(value);
            }
            else
            {
                stream << std::forward<TValue>(value);
            }
            break;

        case 'x':
        case 'X':
        {
            std::ostringstream hexStream;
            if(specifier == 'X')
            {
                hexStream.setf(std::ios::uppercase);
            }
            hexStream << std::hex;
            if constexpr(std::is_integral_v<DecayedValue> || std::is_enum_v<DecayedValue>)
            {
                hexStream << static_cast<unsigned long long>(value);
            }
            else if constexpr(std::is_pointer_v<DecayedValue>)
            {
                hexStream << reinterpret_cast<std::uintptr_t>(value);
            }
            else
            {
                hexStream << std::forward<TValue>(value);
            }
            stream << hexStream.str();
            break;
        }

        case 'p':
            if constexpr(std::is_pointer_v<DecayedValue> || std::is_null_pointer_v<DecayedValue>)
            {
                stream << static_cast<const void *>(value);
            }
            else
            {
                stream << std::forward<TValue>(value);
            }
            break;

        default:
            stream << std::forward<TValue>(value);
            break;
    }
}

inline const char *skipSpecifierPrefix(const char *format)
{
    while((*format == '-') || (*format == '+') || (*format == ' ') || (*format == '#') || (*format == '0'))
    {
        ++format;
    }

    while((*format >= '0') && (*format <= '9'))
    {
        ++format;
    }

    if(*format == '.')
    {
        ++format;
        while((*format >= '0') && (*format <= '9'))
        {
            ++format;
        }
    }

    while((*format == 'l') || (*format == 'h') || (*format == 'z') || (*format == 't') || (*format == 'j'))
    {
        ++format;
    }
    return format;
}

inline void appendFormat(std::ostringstream &stream, const char *format)
{
    appendLiteral(stream, format);
}

template<typename TValue, typename... TRest>
void appendFormat(std::ostringstream &stream, const char *format, TValue &&value, TRest&&... rest)
{
    if(format == nullptr)
    {
        stream << "<null format>";
        return;
    }

    while(*format != '\0')
    {
        if(*format != '%')
        {
            stream << *format;
            ++format;
            continue;
        }

        if(format[1] == '%')
        {
            stream << '%';
            format += 2;
            continue;
        }

        ++format;
        format = skipSpecifierPrefix(format);

        const char specifier = (*format != '\0') ? *format : 's';
        if(*format != '\0')
        {
            ++format;
        }

        appendValue(stream, specifier, std::forward<TValue>(value));
        appendFormat(stream, format, std::forward<TRest>(rest)...);
        return;
    }

    stream << " [extra args:";
    appendValue(stream, 's', std::forward<TValue>(value));
    ((stream << ' ', appendValue(stream, 's', std::forward<TRest>(rest))), ...);
    stream << ']';
}

template<typename... TArgs>
void writeMessage(std::ostream &stream, const int level, const char *tag, const char *format, TArgs&&... args)
{
    std::ostringstream message;
    message << '[' << levelName(level) << "][" << (tag != nullptr ? tag : "<null>") << "] ";
    appendFormat(message, format, std::forward<TArgs>(args)...);
    message << '\n';
    stream << message.str();
    stream.flush();
}

struct LogFunction
{
    int Level;

    template<typename... TArgs>
    void operator()(const char *tag, const char *format, TArgs&&... args) const
    {
        writeMessage((Level == ESP_LOG_ERROR) ? std::cerr : std::cout,
                     Level,
                     tag,
                     format,
                     std::forward<TArgs>(args)...);
    }
};

inline void writeHex(std::ostream &stream, const char *tag, const void *buffer, const std::size_t length, const int level)
{
    std::ostringstream message;
    message << '[' << levelName(level) << "][" << (tag != nullptr ? tag : "<null>") << ']';

    if(buffer == nullptr)
    {
        message << " <null>\n";
        stream << message.str();
        stream.flush();
        return;
    }

    const auto *bytes = static_cast<const std::uint8_t *>(buffer);
    message << std::hex;
    for(std::size_t index = 0; index < length; ++index)
    {
        message << ' ';
        if(bytes[index] < 0x10)
        {
            message << '0';
        }
        message << static_cast<unsigned int>(bytes[index]);
    }
    message << '\n';

    stream << message.str();
    stream.flush();
}

struct BufferHexFunction
{
    void operator()(const char *tag, const void *buffer, const std::size_t length, const int level) const
    {
        writeHex(std::cout, tag, buffer, length, level);
    }
};

struct BufferCharFunction
{
    constexpr void operator()(const char *, const void *, const std::size_t, const int) const
    {
    }
};

}

inline constexpr esp_log_stub::LogFunction ESP_LOGI{ESP_LOG_INFO};
inline constexpr esp_log_stub::LogFunction ESP_LOGW{ESP_LOG_WARN};
inline constexpr esp_log_stub::LogFunction ESP_LOGE{ESP_LOG_ERROR};
inline constexpr esp_log_stub::BufferHexFunction ESP_LOG_BUFFER_HEX_LEVEL{};
inline constexpr esp_log_stub::BufferCharFunction ESP_LOG_BUFFER_CHAR_LEVEL{};
