#pragma once

#include <array>
#include <cstdint>
#include "config.h"

constexpr int32_t kOESSendCpu = 13;
constexpr int32_t kOESRecvCpu = 14;
constexpr int32_t kMDSBindCpu = 15;

constexpr int32_t kChannelCount = 12;
constexpr std::array<int32_t, kChannelCount> kChannelCpus = {
    1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
};

constexpr int32_t kWantSignLookAhead = 16;

constexpr int64_t kReportMoney = 50'0000'00;
