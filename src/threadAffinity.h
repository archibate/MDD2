#pragma once

#include <chrono>
#include <thread>
#include <cstdint>
#include <sched.h>
#include <unistd.h>
#include <emmintrin.h>


[[gnu::always_inline]] inline int64_t steadyNow()
{
    return duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

[[gnu::always_inline]] inline void blockingSleepUntil(int64_t t)
{
    std::this_thread::sleep_until(std::chrono::steady_clock::time_point(std::chrono::nanoseconds(t)));
}

[[gnu::always_inline]] inline void spinSleepUntil(int64_t t)
{
    int64_t t0 = steadyNow();
    if (t <= t0) {
        return;
    }

    constexpr int64_t kEarlyDuration = 800;
    constexpr int64_t kPreHeatDuration = 200'000;
    t -= kEarlyDuration;
    if (t - t0 > kPreHeatDuration * 2) {
        blockingSleepUntil(t - kPreHeatDuration);
    }
    while (steadyNow() < t) {
        _mm_pause();
    }
}

[[gnu::noinline]] inline void setThisThreadAffinity(int32_t cpu)
{
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpu, &cpuset);
    sched_setaffinity(getpid(), sizeof(cpuset), &cpuset);
}
