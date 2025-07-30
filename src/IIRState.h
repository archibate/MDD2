#pragma once


#include "FactorList.h"

struct IIRState
{
    // Previous cumulative volume (needed to compute diff).
    double prev_cum_volume{};

    // Exponential moving averages of the volume diff.
    double short_ema{};
    double long_ema{};

    // Cumulative energy (sum of squares) – updated incrementally.
    double band_energy{};
    double low_freq_energy{};
    double diff_energy{};

    // Window sums for efficient O(1) feature computation
    double windowBand100_sum{};  // Sum of band^2 for last 100 ticks
    double windowLow100_sum{};   // Sum of low_band^2 for last 100 ticks
    double windowDiff100_sum{};  // Sum of diff^2 for last 100 ticks
    double windowBand500_sum{};  // Sum of band^2 for last 500 ticks
    double windowDiff500_sum{};  // Sum of diff^2 for last 500 ticks

    // Ring buffer cursors & fill counters.
    int ring100_pos{};
    int ring500_pos{};
    int ring100_filled{};
    int ring500_filled{};

    // Misc state.
    int  tick_count{};

    // Fixed-size ring buffers for recent windows.
    double band2_ring100[100]{};
    double low_band2_ring100[100]{};
    double diff2_ring100[100]{};

    double band2_ring500[500]{};
    double low_band2_ring500[500]{};
    double diff2_ring500[500]{};

    void addVolumeTick(double lastVolume);
    void finalCompute(FactorList::Crowdind &factor);
};

