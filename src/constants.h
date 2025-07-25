#pragma once

#include <array>
#include <cstdint>
#include "config.h"

constexpr int32_t kMDSBindCpu = 1;
constexpr int32_t kChannelCpuBegin = 2;
constexpr int32_t kChannelCount = 12;

#if REPLAY_REAL_TIME
#define TIME_SCALE * (1.0 / 15.0)
#else
#define TIME_SCALE
#endif

// constexpr std::array kStockCodesOriginal = {
// #if SH
//     600241, 605136, 603711, 600881, 600261, 601956, 600661, 603719, 600824, 603214, 605299, 600327, 603662, 600683, 600743, 600698, 603808, 605033, 601933, 603118, 603500, 600785, 603390, 601010, 603365, 603095, 600255,
// #endif
// #if SZ
//     2778, 2730, 2702, 2582, 2306, 2193, 981, 716, 619, 17,
// #endif
// };
