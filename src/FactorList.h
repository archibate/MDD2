#pragma once

#include <array>
#include <cstdint>
#include "FactorEnum.h"


inline constexpr std::array kMomentumDurations = {
    // 0.1, 0.5, 1.0, 2.0, 3.0, 5.0, 10.0, 30.0,
    6'0, 30'0, 60'0, 120'0, 180'0, 300'0, 600'0, 1800'0,
};

struct FactorList
{
    struct Momentum
    {
        double openMean; // momentum_o_0.1_m
        double openStd; // momentum_o_0.1_sd
        double openZScore; // momentum_o_0.1_z
        double highMean; // momentum_h_0.1_m
        double highStd; // momentum_h_0.1_sd
        double highZScore; // momentum_h_0.1_z
        double diffMean; // momentum_o_h_diff_0.1
        double diffZScore; // momentum_o_h_z_diff_0.1
    };

    union {
        struct {
            Momentum momentum[kMomentumDurations.size()];
        };

        struct {
            double rawFactors[FactorEnum::kMaxFactors];
        };
    };

    void dumpFactors(int32_t timestamp, int32_t stock);
};
