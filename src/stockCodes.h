#pragma once


#include "daily.h"


// SZ: 000xxx 002xxx
// SH: 600xxx 601xxx 603xxx 605xxx

constexpr auto kStockCodesOriginalIndex = [] {
    std::array<int32_t, kStockCodesOriginal.size()> indices;
    int32_t j = 0;
    for (int32_t c = 0; c < kChannelCount; ++c) {
        for (int32_t i = 0; i < kStockCodesOriginal.size(); ++i) {
            int32_t origCode = kStockCodesOriginal[i];
            if (origCode % kChannelCount == c) {
                indices[j++] = i;
            }
        }
    }
    return indices;
} ();

constexpr auto kStockCodesOriginalChannelSizes = [] {
    std::array<int32_t, kChannelCount> sizes{};
    int32_t j = 0;
    for (int32_t c = 0; c < kChannelCount; ++c) {
        for (int32_t i = 0; i < kStockCodesOriginal.size(); ++i) {
            int32_t origCode = kStockCodesOriginal[i];
            if (origCode % kChannelCount == c) {
                ++sizes[c];
            }
        }
    }
    return sizes;
} ();

constexpr auto kStockCodesOriginalChannelStarts = [] {
    std::array<int32_t, kChannelCount> starts;
    for (int32_t c = 0; c < kChannelCount; ++c) {
        starts[c] = -1;
    }
    int32_t j = 0;
    for (int32_t c = 0; c < kChannelCount; ++c) {
        for (int32_t i = 0; i < kStockCodesOriginal.size(); ++i) {
            int32_t origCode = kStockCodesOriginal[i];
            if (origCode % kChannelCount == c) {
                if (starts[c] == -1) {
                    starts[c] = j;
                }
                ++j;
            }
        }
    }
    int32_t lastC = 0;
    for (int32_t c = 0; c < kChannelCount; ++c) {
        if (starts[c] == -1) {
            starts[c] = lastC;
        }
        lastC = starts[c];
    }
    return starts;
} ();

constexpr auto kStockCodes = [] {
    std::array<int32_t, kStockCodesOriginal.size()> codes;
    for (int32_t i = 0; i < codes.size(); ++i) {
        codes[i] = kStockCodesOriginal[kStockCodesOriginalIndex[i]];
    }
    return codes;
} ();

constexpr auto kStockIdLut = [] {
    std::array<int16_t, 0x7FFF> lut{};
    for (int32_t s = 0; s < lut.size(); ++s) {
        lut[s] = -1;
    }
    for (int32_t i = 0; i < kStockCodes.size(); ++i) {
        lut[static_cast<int16_t>(kStockCodes[i] & 0x7FFF)] = i;
    }
    return lut;
} ();
