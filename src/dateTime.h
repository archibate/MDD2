#pragma once

#include <ctime>
#include <cstdint>
#include <cstdlib>
#include <chrono>


inline int32_t getToday() noexcept
{
    std::time_t now = std::time(nullptr);
    std::tm *tm = std::localtime(&now);
    char buf[9];
    buf[std::strftime(buf, sizeof buf, "%Y%m%d", tm)] = 0;
    return std::atoi(buf);
}

inline int32_t getTimestamp() noexcept
{
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm *tm = std::localtime(&t);
    char buf[7];
    buf[std::strftime(buf, sizeof buf, "%H%M%S", tm)] = 0;
    int32_t timestamp = std::atoi(buf);
    timestamp = timestamp * 1000 + static_cast<int32_t>(
        duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()).count() % 1000);
    return timestamp;
}
