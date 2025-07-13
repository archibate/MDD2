#pragma once


#include <cstdint>



namespace L2
{

inline int64_t timestampToMilliseconds(int32_t timestamp)
{
    int hours = timestamp / 10000000;
    int minutes = (timestamp / 100000) % 100;
    int seconds = (timestamp / 1000) % 100;
    int milliseconds = timestamp % 1000;

    return (hours * 3600 + minutes * 60 + seconds) * 1000 + milliseconds;
}

inline int32_t millisecondsToTimestamp(int64_t time)
{
    int milliseconds = time % 1000;
    time /= 1000;
    int seconds = time % 60;
    time /= 60;
    int minutes = time % 60;
    time /= 60;
    int hours = time % 24;
    return milliseconds + 1000 * (seconds + 100 * (minutes + 100 * hours));
}


inline int64_t timestampToAbsoluteMilliseconds(int32_t timestamp, int32_t interval)
{
    int64_t time = timestampToMilliseconds(timestamp);
    time -= timestampToMilliseconds(9'30'00'000);
    if (time < 0) {
        if (time > -5 * 60'000) {
            time = -interval;
        } else {
            time += 5 * 60'000 - interval;
        }
    } else if (time > 2 * 60 * 60'000) {
        if (time < (3 * 60 + 30) * 60'000) {
            time = 2 * 60 * 60'000;
        } else {
            time -= (1 * 60 + 30) * 60'000 - interval;
        }
    }
    return time;
}

inline int32_t absoluteMillisecondsToTimestamp(int64_t time, int32_t interval)
{
    if (time < 0) {
        time -= 5 * 60'000 - interval;
    }
    if (time > 2 * 60 * 60'000) {
        time += (1 * 60 + 30) * 60'000 - interval;
    }
    time += timestampToMilliseconds(9'30'00'000);
    return millisecondsToTimestamp(time);
}

inline int64_t timestampToPositiveAbsoluteMilliseconds(int32_t timestamp)
{
    int64_t time = timestampToMilliseconds(timestamp);
    time -= timestampToMilliseconds(9'30'00'000);
    if (time > 2 * 60 * 60'000) {
        if (time < (3 * 60 + 30) * 60'000) {
            time = 2 * 60 * 60'000;
        } else {
            time -= (1 * 60 + 30) * 60'000;
        }
    }
    return time;
}

inline int32_t positiveAbsoluteMillisecondsToTimestamp(int64_t time)
{
    if (time > 2 * 60 * 60'000) {
        time += (1 * 60 + 30) * 60'000;
    }
    time += timestampToMilliseconds(9'30'00'000);
    return millisecondsToTimestamp(time);
}

}
