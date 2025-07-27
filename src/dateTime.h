#pragma once

#include <ctime>
#include <cstdint>
#include <cstdlib>


inline int32_t getToday() noexcept
{
    std::time_t now = std::time(nullptr);
    std::tm *tm = std::localtime(&now);
    char buf[9];
    buf[std::strftime(buf, 8, "%Y%m%d", tm)] = 0;
    return std::atoi(buf);
}
