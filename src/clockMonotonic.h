#pragma once

// #include <chrono>
// #include <thread>
#include <time.h>
#include <cstdint>
// #include <emmintrin.h>


inline int64_t monotonicTime()
{
    struct timespec tp;
    clock_gettime(CLOCK_MONOTONIC, &tp);
    return static_cast<uint64_t>(tp.tv_sec) * UINT64_C(1'000'000'000) + static_cast<uint64_t>(tp.tv_nsec);
    // return duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

inline void monotonicSleepUntil(int64_t t)
{
    struct timespec tp;
    uint64_t u = t;
    uint64_t s = u / UINT64_C(1'000'000'000);
    tp.tv_sec = s;
    tp.tv_nsec = t - s * UINT64_C(1'000'000'000);
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &tp, nullptr);
    // std::this_thread::sleep_until(std::chrono::steady_clock::time_point(std::chrono::nanoseconds(t)));
}

inline void monotonicSleepFor(int64_t t)
{
    struct timespec tp;
    uint64_t u = t;
    uint64_t s = u / UINT64_C(1'000'000'000);
    tp.tv_sec = s;
    tp.tv_nsec = t - s * UINT64_C(1'000'000'000);
    clock_nanosleep(CLOCK_MONOTONIC, 0, &tp, nullptr);
    // std::this_thread::sleep_until(std::chrono::steady_clock::time_point(std::chrono::nanoseconds(t)));
}

// inline void monotonicSpinSleep(int64_t t)
// {
//     int64_t t0 = monotonicTime();
//     if (t <= t0) {
//         return;
//     }
//
//     constexpr int64_t kEarlyDuration = 800;
//     constexpr int64_t kPreHeatDuration = 200'000;
//     t -= kEarlyDuration;
//     if (t - t0 > kPreHeatDuration * 2) {
//         monotonicSleep(t - kPreHeatDuration);
//     }
//     while (monotonicTime() < t) {
//         _mm_pause();
//     }
// }
