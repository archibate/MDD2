#pragma once

#include <magic_enum/magic_enum.hpp>

enum FactorEnum
{
    momentum_o_0o1_m,
    momentum_o_0o1_sd,
    momentum_o_0o1_z,
    momentum_h_0o1_m,
    momentum_h_0o1_sd,
    momentum_h_0o1_z,
    momentum_o_h_diff_0o1,
    momentum_o_h_z_diff_0o1,

    momentum_o_0o5_m,
    momentum_o_0o5_sd,
    momentum_o_0o5_z,
    momentum_h_0o5_m,
    momentum_h_0o5_sd,
    momentum_h_0o5_z,
    momentum_o_h_diff_0o5,
    momentum_o_h_z_diff_0o5,

    momentum_o_1_m,
    momentum_o_1_sd,
    momentum_o_1_z,
    momentum_h_1_m,
    momentum_h_1_sd,
    momentum_h_1_z,
    momentum_o_h_diff_1,
    momentum_o_h_z_diff_1,

    momentum_o_2_m,
    momentum_o_2_sd,
    momentum_o_2_z,
    momentum_h_2_m,
    momentum_h_2_sd,
    momentum_h_2_z,
    momentum_o_h_diff_2,
    momentum_o_h_z_diff_2,

    momentum_o_3_m,
    momentum_o_3_sd,
    momentum_o_3_z,
    momentum_h_3_m,
    momentum_h_3_sd,
    momentum_h_3_z,
    momentum_o_h_diff_3,
    momentum_o_h_z_diff_3,

    momentum_o_5_m,
    momentum_o_5_sd,
    momentum_o_5_z,
    momentum_h_5_m,
    momentum_h_5_sd,
    momentum_h_5_z,
    momentum_o_h_diff_5,
    momentum_o_h_z_diff_5,

    momentum_o_10_m,
    momentum_o_10_sd,
    momentum_o_10_z,
    momentum_h_10_m,
    momentum_h_10_sd,
    momentum_h_10_z,
    momentum_o_h_diff_10,
    momentum_o_h_z_diff_10,

    momentum_o_30_m,
    momentum_o_30_sd,
    momentum_o_30_z,
    momentum_h_30_m,
    momentum_h_30_sd,
    momentum_h_30_z,
    momentum_o_h_diff_30,
    momentum_o_h_z_diff_30,

    vol_4_7_time,
    vol_4_7_consecutive_time,
    vol_4_7_amaount_ratio,
    vol_4_7_volume_ratio,
    vol_4_7_turnover,
    vol_4_7_consecutive_amount_ratios,
    vol_4_7_consecutive_volume_ratios,
    vol_4_7_consecutive_turnovers,
    vol_4_7_up,
    vol_4_7_up_ratio,
    vol_4_7_up_std,
    vol_4_7_vwap_skew,
    vol_4_7_vwap_kurt,
    vol_4_7_filter_vwap_skews,
    vol_4_7_filter_vwap_kurts,

    vol_5_8_time,
    vol_5_8_consecutive_time,
    vol_5_8_amaount_ratio,
    vol_5_8_volume_ratio,
    vol_5_8_turnover,
    vol_5_8_consecutive_amount_ratios,
    vol_5_8_consecutive_volume_ratios,
    vol_5_8_consecutive_turnovers,
    vol_5_8_up,
    vol_5_8_up_ratio,
    vol_5_8_up_std,
    vol_5_8_vwap_skew,
    vol_5_8_vwap_kurt,
    vol_5_8_filter_vwap_skews,
    vol_5_8_filter_vwap_kurts,

    vol_6_9_time,
    vol_6_9_consecutive_time,
    vol_6_9_amaount_ratio,
    vol_6_9_volume_ratio,
    vol_6_9_turnover,
    vol_6_9_consecutive_amount_ratios,
    vol_6_9_consecutive_volume_ratios,
    vol_6_9_consecutive_turnovers,
    vol_6_9_up,
    vol_6_9_up_ratio,
    vol_6_9_up_std,
    vol_6_9_vwap_skew,
    vol_6_9_vwap_kurt,
    vol_6_9_filter_vwap_skews,
    vol_6_9_filter_vwap_kurts,

    QUA,
    TS,
    filter_QUA,
    filter_TS,
    SR_10,
    SR_30,
    SR_50,

    crowdind_20,
    crowdind_h5_20,
    crowdind_last_20,
    crowdind_h5_last_20,

    prev_U_mean_return,
    circ_mv,

    EMA_6_normalized,
    MIDPOINT_14_normalized,
    T3_5_normalized,
    ATR_14_vol_adj,
    NATR_14_vol_adj,
    AD_normalized,
    ADOSC_normalized,
    ADX_14_vol_adj,
    RSI_14,
    RSI_6,
    MACD_macd,
    MACD_signal,
    MACD_hist,
    VWAP_normalized,
    close_ma_5_normalized,
    return_std_5_vol_adj,
    close_ma_10_normalized,
    return_std_10_vol_adj,
    close_ma_20_normalized,
    return_std_20_vol_adj,
    total_amount_quantile,
    stock_character,
    kMaxFactors
};

template <>
struct magic_enum::customize::enum_range<FactorEnum>
{
    static constexpr int min = 0;
    static constexpr int max = FactorEnum::kMaxFactors - 1;
};
