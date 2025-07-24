#pragma once

#include <chrono>
#include <thread>
#include <cstdint>
#include <sched.h>
#include <unistd.h>
#include <emmintrin.h>


inline void spinSleepUntil(std::chrono::steady_clock::time_point t)
{
    auto t0 = std::chrono::steady_clock::now();
    if (t <= t0) {
        return;
    }

    const std::chrono::nanoseconds kEarlyDuration(800);
    const std::chrono::microseconds kPreHeatDuration(200);
    t -= kEarlyDuration;
    if (t - t0 > kPreHeatDuration * 2) {
        std::this_thread::sleep_until(t - kPreHeatDuration);
    }
    while (std::chrono::steady_clock::now() < t) {
        _mm_pause();
    }
}


template <class Busy>
inline void spinSleepUntil(std::chrono::steady_clock::time_point t, Busy &&busy)
{
    auto t0 = std::chrono::steady_clock::now();
    if (t <= t0) {
        return;
    }

    const std::chrono::microseconds kPreHeatDuration(600);
    if (t - t0 > kPreHeatDuration * 2) {
        std::this_thread::sleep_until(t - kPreHeatDuration);
    }
    while (std::chrono::steady_clock::now() < t) {
        busy();
    }
}


// template <class Busy>
// inline void twoStageSpinSleepUntil(std::chrono::steady_clock::time_point t, Busy &&busy)
// {
//     auto t0 = std::chrono::steady_clock::now();
//     if (t <= t0) {
//         return;
//     }
//     const std::chrono::microseconds kPreHeatDuration(30'000);
//     const std::chrono::microseconds kFinalRushDuration(50);
//     if (t - t0 > kPreHeatDuration) {
//         std::this_thread::sleep_until(t - kPreHeatDuration);
//     }
//     auto t1 = t - kFinalRushDuration;
//     while (std::chrono::steady_clock::now() < t1) {
//         busy();
//     }
//     while (std::chrono::steady_clock::now() < t) {
//         _mm_pause();
//     }
// }


inline void setThisThreadAffinity(int32_t cpu)
{
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpu, &cpuset);
    sched_setaffinity(getpid(), sizeof(cpuset), &cpuset);
}
