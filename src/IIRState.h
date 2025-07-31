#pragma once


#include "FactorList.h"

struct alignas(64) IIRState
{
    // Previous cumulative volume (needed to compute diff).
    float prev_cum_volume{};

    // Exponential moving averages of the volume diff.
    float short_ema{};
    float long_ema{};

    // Cumulative energy (sum of squares) – updated incrementally.
    float band_energy{};
    float low_freq_energy{};
    float diff_energy{};

    // Window sums for efficient O(1) feature computation
    float windowBand100_sum{};  // Sum of band^2 for last 100 ticks
    float windowLow100_sum{};   // Sum of low_band^2 for last 100 ticks
    float windowDiff100_sum{};  // Sum of diff^2 for last 100 ticks
    float windowBand500_sum{};  // Sum of band^2 for last 500 ticks
    float windowDiff500_sum{};  // Sum of diff^2 for last 500 ticks

    // Ring buffer cursors & fill counters.
    int ring100_pos{};
    int ring500_pos{};
    int ring100_filled{};
    int ring500_filled{};

    // Misc state.
    int  tick_count{};

    // Fixed-size ring buffers for recent windows.
    float band2_ring100[100]{};
    float low_band2_ring100[100]{};
    float diff2_ring100[100]{};

    float band2_ring500[500]{};
    float low_band2_ring500[500]{};
    float diff2_ring500[500]{};

    void addVolumeTick(float lastVolume);
    void finalCompute(FactorList::Crowdind &factor);
};

