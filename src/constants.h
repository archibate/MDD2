#pragma once

#include <array>
#include <cstdint>
#include "config.h"

#if REPLAY
constexpr int32_t kOESSendCpu = 13;
constexpr int32_t kOESRecvCpu = 14;
constexpr int32_t kMDSBindCpu = 15;

constexpr int32_t kChannelCount = 12;
constexpr std::array<int32_t, kChannelCount> kChannelCpus = {
    1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
};
#endif
#if XC
constexpr int32_t kOESSendCpu = 1;
constexpr int32_t kOESRecvCpu = -1;
constexpr int32_t kMDSBindCpu = 2;
constexpr int32_t kMDSCacheCpu = 3;
constexpr int32_t kMDSBackupCpu = 12;

constexpr int32_t kChannelCount = 9;
constexpr std::array<int32_t, kChannelCount> kChannelCpus = {
    3, 4, 5, 6, 7, 8, 9, 10, 11,
};
#endif
#if NE
constexpr int32_t kOESSendCpu = 13;
constexpr int32_t kOESRecvCpu = 14;
constexpr int32_t kMDSBindCpu = 15;

constexpr int32_t kChannelCount = 12;
constexpr std::array<int32_t, kChannelCount> kChannelCpus = {
    1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
};
#endif
#if OST
constexpr int32_t kOESBindCpu = 13;
constexpr int32_t kMDSBindCpu = 14;

constexpr int32_t kChannelCount = 12;
constexpr std::array<int32_t, kChannelCount> kChannelCpus = {
    1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
};
#endif

constexpr int32_t kWantSignLookAhead = 10;

constexpr int64_t kReportMoney = 49'0000'00;
