#pragma once

#include <chrono>
#include <string_view>

[[nodiscard]] constexpr std::chrono::sys_seconds get_build_timestamp() noexcept
{
    using namespace std::chrono_literals;

    constexpr std::string_view date_str{__DATE__}; // Format: "Mmm dd yyyy"
    constexpr std::string_view time_str{__TIME__}; // Format: "hh:mm:ss"

    constexpr int year = (date_str[7] - '0') * 1000 + (date_str[8] - '0') * 100 + (date_str[9] - '0') * 10 + (date_str[10] - '0');

    constexpr int day = (date_str[4] == ' ' ? 0 : date_str[4] - '0') * 10 + (date_str[5] - '0');

    constexpr auto get_month = [](std::string_view m) constexpr -> std::chrono::month {
        if (m.starts_with("Jan")) return std::chrono::January;
        if (m.starts_with("Feb")) return std::chrono::February;
        if (m.starts_with("Mar")) return std::chrono::March;
        if (m.starts_with("Apr")) return std::chrono::April;
        if (m.starts_with("May")) return std::chrono::May;
        if (m.starts_with("Jun")) return std::chrono::June;
        if (m.starts_with("Jul")) return std::chrono::July;
        if (m.starts_with("Aug")) return std::chrono::August;
        if (m.starts_with("Sep")) return std::chrono::September;
        if (m.starts_with("Oct")) return std::chrono::October;
        if (m.starts_with("Nov")) return std::chrono::November;
        return std::chrono::December;
    };

    constexpr auto month = get_month(date_str);

    constexpr int hour   = (time_str[0] - '0') * 10 + (time_str[1] - '0');
    constexpr int minute = (time_str[3] - '0') * 10 + (time_str[4] - '0');
    constexpr int second = (time_str[6] - '0') * 10 + (time_str[7] - '0');

    constexpr std::chrono::year_month_day ymd{std::chrono::year{year}, month, std::chrono::day{static_cast<unsigned>(day)}};

    return std::chrono::sys_days{ymd} + std::chrono::hours{hour} + std::chrono::minutes{minute} + std::chrono::seconds{second};
}

static void set_system_time()
{
    time_t build_time = std::chrono::system_clock::to_time_t(
        get_build_timestamp()
    );

    struct timeval tv = {
        .tv_sec = build_time,
        .tv_usec = 0
    };

    settimeofday(&tv, NULL);

    setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 1);

    tzset();
}
