#pragma once

#include <array>
#include <cstdint>

// md_bind=2
// api_recv=3
// api_send=4
// channels=5-14

constexpr int32_t kChannelCount = 10;
constexpr int32_t kChannelCpuBegin = 5;
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
