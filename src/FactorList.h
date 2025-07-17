#pragma once

#include <array>
#include <cstdint>


inline constexpr std::array kMomentumDurations = {
    // 0.1, 0.5, 1.0, 2.0, 3.0, 5.0, 10.0, 30.0,
    6'0, 30'0, 60'0, 120'0, 180'0, 300'0, 600'0, 1800'0,
};

inline constexpr std::array kFactorNames = {
    "momentum_h_0o1_m",
    "momentum_h_0o1_sd",
    "momentum_h_0o1_z",
    "momentum_o_0o1_m",
    "momentum_o_0o1_sd",
    "momentum_o_0o1_z",
    "momentum_o_h_diff_0o1",
    "momentum_o_h_z_diff_0o1",
    "momentum_h_0o5_m",
    "momentum_h_0o5_sd",
    "momentum_h_0o5_z",
    "momentum_o_0o5_m",
    "momentum_o_0o5_sd",
    "momentum_o_0o5_z",
    "momentum_o_h_diff_0o5",
    "momentum_o_h_z_diff_0o5",
    "momentum_h_1_m",
    "momentum_h_1_sd",
    "momentum_h_1_z",
    "momentum_o_1_m",
    "momentum_o_1_sd",
    "momentum_o_1_z",
    "momentum_o_h_diff_1",
    "momentum_o_h_z_diff_1",
    "momentum_h_2_m",
    "momentum_h_2_sd",
    "momentum_h_2_z",
    "momentum_o_2_m",
    "momentum_o_2_sd",
    "momentum_o_2_z",
    "momentum_o_h_diff_2",
    "momentum_o_h_z_diff_2",
    "momentum_h_3_m",
    "momentum_h_3_sd",
    "momentum_h_3_z",
    "momentum_o_3_m",
    "momentum_o_3_sd",
    "momentum_o_3_z",
    "momentum_o_h_diff_3",
    "momentum_o_h_z_diff_3",
    "momentum_h_5_m",
    "momentum_h_5_sd",
    "momentum_h_5_z",
    "momentum_o_5_m",
    "momentum_o_5_sd",
    "momentum_o_5_z",
    "momentum_o_h_diff_5",
    "momentum_o_h_z_diff_5",
    "momentum_h_10_m",
    "momentum_h_10_sd",
    "momentum_h_10_z",
    "momentum_o_10_m",
    "momentum_o_10_sd",
    "momentum_o_10_z",
    "momentum_o_h_diff_10",
    "momentum_o_h_z_diff_10",
    "momentum_h_30_m",
    "momentum_h_30_sd",
    "momentum_h_30_z",
    "momentum_o_30_m",
    "momentum_o_30_sd",
    "momentum_o_30_z",
    "momentum_o_h_diff_30",
    "momentum_o_h_z_diff_30",
};

struct FactorList
{
    struct Momentum
    {
        double highMean; // momentum_h_0.1_m
        double highStd; // momentum_h_0.1_sd
        double highZScore; // momentum_h_0.1_z
        double openMean; // momentum_o_0.1_m
        double openStd; // momentum_o_0.1_sd
        double openZScore; // momentum_o_0.1_z
        double diffMean; // momentum_o_h_diff_0.1
        double diffZScore; // momentum_o_h_z_diff_0.1
    };

    std::array<Momentum, kMomentumDurations.size()> momentum;

    void dumpFactors(int32_t timestamp, int32_t stock);
};
