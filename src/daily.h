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
    605299,
    605033,
    603215,
    603068,
    601010,
    600683,
    600322,
};

#if REPLAY_REAL_TIME
#define TIME_SCALE * (1.0 / 50.0)
constexpr int32_t kBrustProcessMicroseconds = 6500;
#else
#define TIME_SCALE
constexpr int32_t kBrustProcessMicroseconds = 4500;
#endif
