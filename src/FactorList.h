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
    using ScalarType = double;

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

    struct Volatility
    {
        double time; // vol_4_7_time
        double consecTime; // vol_4_7_consecutive_time
        double amountRatio; // vol_4_7_amaount_ratio
        double volumeRatio; // vol_4_7_volume_ratio
        double turnover; // vol_4_7_turnover
        double consecAmountRatio; // vol_4_7_consecutive_amount_ratios
        double consecVolumeRatio; // vol_4_7_consecutive_volume_ratios
        double consecTurnover; // vol_4_7_consecutive_turnovers
        double upVolume; // vol_4_7_up
        double upRatio; // vol_4_7_up_ratio
        double upStd; // vol_4_7_up_std
        double vwapSkew; // vol_4_7_vwap_skew
        double vwapKurt; // vol_4_7_vwap_kurt
        double consecVwapSkew; // vol_4_7_filter_vwap_skews
        double consecVwapKurt; // vol_4_7_filter_vwap_kurts
    };

    struct Kaiyuan
    {
        double quantile; // QUA
        double correlation; // TS
        double trimmedQuantile; // filter_QUA
        double trimmedCorrelation; // filter_TS
        double signalReturn10; // SR_10
        double signalReturn30; // SR_30
        double signalReturn50; // SR_50
    };

    union {
        struct {
            Momentum momentum[kMomentumDurations.size()];
            Volatility volatility[3];
            Kaiyuan kaiyuan;
        };

        struct {
            double rawFactors[FactorEnum::kMaxFactors];
        };
    };

    void dumpFactors(int32_t timestamp, int32_t stock) const;
};
