#pragma once

#include <cstdint>

struct DailyHeader
{
    int32_t fileVersion; // 250722
    int32_t today; // 20250102
    int32_t marketID; // SH=1, SZ=2
    int32_t stockCount;
    int32_t prevLimitUpCount;
    int32_t factorCount; // FactorEnum::kMaxFactors
    int32_t factorDtypeSize; // sizeof(double)
};
