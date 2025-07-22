#pragma once

#include <array>
#include <cstdint>
#include "config.h"

// md_bind=2
// api_recv=3
// api_send=4
// channels=5-14
// timer=15

constexpr int32_t kTimerCpu = 15;
constexpr int32_t kChannelCpuBegin = 5;
constexpr int32_t kChannelCount = 10;
constexpr int32_t kChannelPoolSize = 4096;

constexpr std::array kStockCodesOriginal = {
#if SH
    600241, 605136, 603711, 600881, 600261, 601956, 600661, 603719, 600824, 603214, 605299, 600327, 603662, 600683, 600743, 600698, 603808, 605033, 601933, 603118, 603500, 600785, 603390, 601010, 603365, 603095, 600255,
#endif
#if SZ
    2778, 2730, 2702, 2582, 2306, 2193, 981, 716, 619, 17,
#endif
};

#if REPLAY_REAL_TIME
#define TIME_SCALE * (1.0 / 100.0)
constexpr int32_t kBrustProcessMicroseconds = 6500;
#else
#define TIME_SCALE
constexpr int32_t kBrustProcessMicroseconds = 4500;
#endif
