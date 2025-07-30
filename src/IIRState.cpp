#include "IIRState.h"
#include "heatZone.h"
#include "FactorList.h"
#include <cmath>
#include <cstring>


HEAT_ZONE_SNAPSHOT void IIRState::addVolumeTick(double lastVolume)
{
    static constexpr int kCrowdShortSpan = 30; // 3 seconds window (in ticks)
    static constexpr int kCrowdLongSpan = 600; // 60 seconds window (in ticks)

    // --- IIR band-pass crowd factor update (O(1) per tick) ---


    // diff is simply the tick volume;
    double diff = (tick_count == 0 ? 0.0 : lastVolume);

    // Build/extend our own cumulative-volume counter so we never have to touch
    // the heavyweight order-book for historical data.
    double curr_cum_volume = prev_cum_volume + diff;

    // EMA coefficients
    const double alpha_short = 2.0 / (kCrowdShortSpan + 1);
    const double alpha_long  = 2.0 / (kCrowdLongSpan + 1);


    // Update EMAs
    if (tick_count == 0) {
        // Initialise both EMAs with the first observation
        short_ema = alpha_short * diff;     // pandas' first ewm value
        long_ema  = alpha_long  * diff;
    }
    else {
        short_ema = alpha_short * diff + (1.0 - alpha_short) * short_ema;
        long_ema  = alpha_long  * diff + (1.0 - alpha_long) * long_ema;
    }

    // Calculate band-pass and low-pass signals
    double high_band = short_ema - long_ema;  // High-frequency band-pass
    double low_band  = long_ema;              // Low-frequency component

    // Accumulate energies
    double diff_square = diff * diff;
    double high_band_square = high_band * high_band;
    double low_band_square = low_band * low_band;

    band_energy += high_band_square;              // High-frequency energy
    low_freq_energy += low_band_square;           // Low-frequency energy
    diff_energy += diff_square;                   // Total energy

    // Update ring buffers for windowed features (last 100 ticks)

    // Subtract old values from window sums before overwriting
    windowBand100_sum += high_band_square - band2_ring100[ring100_pos];
    windowLow100_sum += low_band_square - low_band2_ring100[ring100_pos];
    windowDiff100_sum += diff_square - diff2_ring100[ring100_pos];

    // Update ring buffers for 100-tick window
    band2_ring100[ring100_pos] = high_band_square;
    low_band2_ring100[ring100_pos] = low_band_square;
    diff2_ring100[ring100_pos] = diff_square;
    ring100_pos = (ring100_pos + 1) % 100;
    if (ring100_filled < 100) ++ring100_filled;

    // Subtract old values from 500-tick window sums before overwriting
    windowBand500_sum += high_band_square - band2_ring500[ring500_pos];
    windowDiff500_sum += diff_square - diff2_ring500[ring500_pos];

    // Update ring buffers for longer window (500 ticks)
    band2_ring500[ring500_pos] = high_band_square;
    low_band2_ring500[ring500_pos] = low_band_square;
    diff2_ring500[ring500_pos] = diff_square;
    ring500_pos = (ring500_pos + 1) % 500;
    if (ring500_filled < 500) ++ring500_filled;

    // Update prev_cum_volume and tick count
    prev_cum_volume = curr_cum_volume;
    ++tick_count;
}

HEAT_ZONE_COMPUTE void IIRState::finalCompute(FactorList::Crowdind &factor)
{
    constexpr double kCrowdEpsilon = 1e-12;

    // If not enough ticks – output NaNs and return.
    if (tick_count < 10) {
        std::memset(&factor, -1, sizeof(factor)); // -1 is same as NaN in bit-representation
        return;
    }

    // -------- Full-session indicators --------
    factor.crowdIndTop20    = band_energy      / (diff_energy + kCrowdEpsilon); // crowdind_20

    // -------- "last" (100-tick) low-frequency ratio  ---------
    factor.crowdIndBottom20 = windowLow100_sum / (diff_energy + kCrowdEpsilon); // crowdind_last_20

    // -------- h5 window (500 ticks) ----------------------------
    /* high-band ratio */
    factor.crowdIndLast500Top20 =                       // crowdind_h5_20
            ring500_filled
            ? windowBand500_sum / (windowDiff500_sum + kCrowdEpsilon)
            : NAN;
    /* total-energy ratio (h5_last): last-100 / last-500 */
    factor.crowdIndLast500Bottom20 =                    // crowdind_h5_last_20
            (ring500_filled && ring100_filled)
            ? windowDiff100_sum / (windowDiff500_sum + kCrowdEpsilon)
            : NAN;
}
