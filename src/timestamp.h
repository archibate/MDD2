#pragma once


#include <cstdint>

[[gnu::always_inline]] inline constexpr int32_t timestampLinear(int32_t timestamp)
{
    int32_t hours = timestamp / 10000000;
    int32_t minutes = (timestamp / 100000) % 100;
    int32_t seconds = (timestamp / 1000) % 100;
    int32_t milliseconds = timestamp % 1000;

    return (hours * 3600 + minutes * 60 + seconds) * 1000 + milliseconds;
}

[[gnu::always_inline]] inline constexpr int32_t timestampDelinear(int32_t time)
{
    int32_t milliseconds = time % 1000;
    time /= 1000;
    int32_t seconds = time % 60;
    time /= 60;
    int32_t minutes = time % 60;
    time /= 60;
    int32_t hours = time % 24;
    return milliseconds + 1000 * (seconds + 100 * (minutes + 100 * hours));
}

[[gnu::always_inline]] inline constexpr int64_t timestampAbsLinear(int32_t timestamp)
{
    int64_t time = timestampLinear(timestamp);
    time -= timestampLinear(9'30'00'000);
    if (time > 2 * 60 * 60'000) {
        if (time < (3 * 60 + 30) * 60'000) {
            time = 2 * 60 * 60'000 + 100;
        } else {
            time -= (1 * 60 + 30) * 60'000 - 100;
        }
    }
    return time;
}

[[gnu::always_inline]] inline constexpr int32_t timestampAbsDelinear(int64_t time)
{
    if (time > 2 * 60 * 60'000) {
        time += (1 * 60 + 30) * 60'000 - 100;
    }
    time += timestampLinear(9'30'00'000);
    return timestampDelinear(time);
}

[[gnu::always_inline]] inline constexpr int32_t timestampAdvance(int32_t timestamp, int32_t ms)
{
    return timestampAbsDelinear(timestampAbsLinear(timestamp) + ms);
}

[[gnu::always_inline]] inline constexpr int32_t timestampAdvance100ms(int32_t timestamp)
{
    return timestampAbsDelinear(timestampAbsLinear(timestamp) + 100);
}

[[gnu::always_inline]] inline constexpr int32_t timestampDifference(int32_t t1, int32_t t2)
{
    return timestampAbsLinear(t1) - timestampAbsLinear(t2);
}
