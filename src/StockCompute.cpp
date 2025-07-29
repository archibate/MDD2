#include "StockCompute.h"
#include "StockState.h"
#include "MDD.h"
#include "timestamp.h"
#include "heatZone.h"
#include <spdlog/spdlog.h>
#include <cstring>

namespace
{

COLD_ZONE void logLimitUp(int32_t stockIndex, int32_t tickTimestamp, TickCache::Intent intent)
{
    auto &tickCache = MDD::g_tickCaches[stockIndex];
    int32_t linearTimestamp = (timestampLinear(tickTimestamp) + 90) / 100 * 100;
    int32_t minLinearTimestamp = tickCache.wantBuyTimestamp[0].load(std::memory_order_relaxed);
    int32_t minDt = std::abs(minLinearTimestamp - linearTimestamp);
    for (int32_t i = 1; i < tickCache.wantBuyTimestamp.size(); ++i) {
        int32_t wantTimestamp = tickCache.wantBuyTimestamp[i].load(std::memory_order_relaxed);
        int32_t dt = std::abs(wantTimestamp - linearTimestamp);
        if (dt < minDt) {
            minDt = dt;
            minLinearTimestamp = wantTimestamp;
        }
    }
    minLinearTimestamp += minLinearTimestamp & 1;

    std::atomic_thread_fence(std::memory_order_seq_cst);
    auto &stockCompute = MDD::g_stockComputes[stockIndex];
    auto &stockState = MDD::g_stockStates[stockIndex];

    // todo: find the correct factor list time and dump:
    stockCompute.dumpFactors(timestampDelinear(minLinearTimestamp));

    SPDLOG_INFO("limit up model status: stock={} tickTimestamp={} roundTimestamp={} minTimestamp={} minDt={} toleranceDt={} intent={}", stockState.stockCode, tickTimestamp, timestampLinear((timestampDelinear(tickTimestamp) + 90) / 100 * 100), timestampDelinear(minLinearTimestamp), minDt, kWantBuyTimeTolerance, magic_enum::enum_name(intent));
    SPDLOG_TRACE("limit up detected: stock={} timestamp={} intent={}", stockState.stockCode, tickTimestamp, magic_enum::enum_name(intent));
}

}


void StockCompute::start()
{
    upperLimitPriceApproach = static_cast<int32_t>(std::floor(stockState().upperLimitPrice / 1.02)) - 1;
    // upperLimitPriceApproach = static_cast<int32_t>(std::floor(stockState().upperLimitPrice * 0.996)) - 2;
    fState.currSnapshot.lastPrice = stockState().preClosePrice;
    fState.currSnapshot.numTrades = 0;
    fState.currSnapshot.volume = 0;
    fState.currSnapshot.amount = 0;
    futureTimestamp = fState.nextTickTimestamp = 9'30'00'000;

#if REPLAY
    factorListCache = std::make_unique<FactorList[]>(std::tuple_size_v<decltype(std::declval<TickCache>().wantBuyTimestamp)>);
#endif
}

COLD_ZONE void StockCompute::stop()
{
    alive = false;
    approachingLimitUp = false;
}

COLD_ZONE void StockCompute::dumpFactors(int32_t timestamp) const
{
    auto &tickCache = MDD::g_tickCaches[stockIndex()];
    for (int32_t i = 0; i < tickCache.wantBuyTimestamp.size(); ++i) {
        if (timestamp == tickCache.wantBuyTimestamp[i].load(std::memory_order_relaxed)) {
            factorListCache[i].dumpFactors(timestamp, stockState().stockCode);
            return;
        }
    }

    factorList.dumpFactors(timestampAdvance(futureTimestamp, -100), stockState().stockCode);
}

int64_t StockCompute::upSellOrderAmount() const
{
    int64_t amount = 0;
    for (auto const &[sellOrderId, sell]: upSellOrders) {
        int64_t q64 = sell.quantity;
        amount += sell.price * q64;
    }
    return amount;
}

HEAT_ZONE_ORDBOOK void StockCompute::onTick(MDS::Tick &tick)
{
    if (tick.stock == 0) [[unlikely]] {
        logLimitUp(stockIndex(), tick.timestamp / 10 * 10,
                   static_cast<TickCache::Intent>(tick.timestamp % 10));
        stop();
        return;
    }

    if (tick.isOrder()) {
        if (tick.isOrderCancel()) {
            onCancel(tick);
        } else {
            onOrder(tick);
        }

    } else if (tick.isTrade()) {
        onTrade(tick);
    }
}

HEAT_ZONE_ORDBOOK void StockCompute::onTimer()
{
    if (!alive) [[unlikely]] {
        return;
    }
    auto &tickCache = MDD::g_tickCaches[stockIndex()];
    if (!tickCache.tryObtainReadCopy()) {
        return;
    }
    auto &ticks = tickCache.getReadCopy();
    if (ticks.empty()) [[unlikely]] {
        return;
    }

    approachingLimitUp = false;
    for (auto &tick: ticks) {
        onTick(tick);
    }

    if (approachingLimitUp) {
        // 603662
        futureTimestamp = fState.nextTickTimestamp;
        firstCompute = true;
    }
}

HEAT_ZONE_COMPUTE void StockCompute::onPostTimer()
{
    if (approachingLimitUp) {
        computeFutureWantBuy();
        futureTimestamp = timestampAdvance100ms(futureTimestamp);
        if (firstCompute) {
            firstCompute = false;
            computeFutureWantBuy();
            futureTimestamp = timestampAdvance100ms(futureTimestamp);
        }
    }
}

HEAT_ZONE_COMPUTE void StockCompute::onBusy()
{
    bool wantBuy = computeModel();
    asm volatile ("" :: "r" (wantBuy));
}

HEAT_ZONE_ORDBOOK void StockCompute::onOrder(MDS::Tick &tick)
{
    if (tick.price >= upperLimitPriceApproach && tick.isSellOrder()) {
        UpSell sell;
        sell.price = tick.price;
        sell.quantity = tick.quantity;
        upSellOrders.insert({tick.orderNo(), sell});
    }
}

HEAT_ZONE_ORDBOOK void StockCompute::onCancel(MDS::Tick &tick)
{
    if (tick.isSellOrder()) {
        auto it = upSellOrders.find(tick.orderNo());
        if (it != upSellOrders.end()) {
            upSellOrders.erase(it);
        }
    }
}

HEAT_ZONE_ORDBOOK void StockCompute::onTrade(MDS::Tick &tick)
{
    auto it = upSellOrders.find(tick.sellOrderNo);
    if (it != upSellOrders.end()) {
        it->second.quantity -= tick.quantity;
        if (it->second.quantity <= 0) {
            upSellOrders.erase(it);
        }
    }

    if (tick.timestamp < 9'30'00'000) {
        openPrice = tick.price;
        openVolume += tick.quantity;
    } else if (tick.price >= upperLimitPriceApproach) {
        approachingLimitUp = true;
    }

    addRealTrade(tick.timestamp, tick.price, tick.quantity);
}

HEAT_ZONE_ORDBOOK void StockCompute::addRealTrade(int32_t timestamp, int32_t price, int32_t quantity)
{
    stepSnapshotUntil(timestamp);

    int64_t q64 = quantity;
    fState.currSnapshot.lastPrice = price;
    ++fState.currSnapshot.numTrades;
    fState.currSnapshot.volume += q64;
    fState.currSnapshot.amount += price * q64;
}

HEAT_ZONE_COMPUTE void StockCompute::computeFutureWantBuy()
{
    saveSnapshot();
    stepSnapshotUntil(futureTimestamp);

    for (auto const &[sellOrderId, sell]: upSellOrders) {
        int64_t q64 = sell.quantity;
        ++fState.currSnapshot.numTrades;
        fState.currSnapshot.volume += q64;
        fState.currSnapshot.amount += sell.price * q64;
    }
    fState.currSnapshot.lastPrice = stockState().upperLimitPrice;
    stepSnapshot();

    bool wantBuy = decideWantBuy();
    auto &tickCache = MDD::g_tickCaches[stockIndex()];
    tickCache.pushWantBuyTimestamp(futureTimestamp, wantBuy);
#if REPLAY
    factorListCache[(tickCache.wantBuyCurrentIndex - 1) & (tickCache.wantBuyTimestamp.size() - 1)] = factorList;
#endif

    restoreSnapshot();
}

HEAT_ZONE_SNAPSHOT void StockCompute::stepSnapshotUntil(int32_t timestamp)
{
    if (timestamp > fState.nextTickTimestamp) {
        stepSnapshot();
        while (timestamp > (fState.nextTickTimestamp = timestampAdvance100ms(fState.nextTickTimestamp))) {
            stepSnapshot();
        }
    }
}

HEAT_ZONE_COMPUTE void StockCompute::saveSnapshot()
{
    bState.savingMode = true;
    bState.nextTickTimestamp = fState.nextTickTimestamp;
    bState.currSnapshot = fState.currSnapshot;
    bState.oldSnapshotsCount = fState.snapshots.size();
}

HEAT_ZONE_COMPUTE void StockCompute::restoreSnapshot()
{
    bState.savingMode = false;
    fState.nextTickTimestamp = bState.nextTickTimestamp;
    fState.currSnapshot = bState.currSnapshot;
    fState.snapshots.resize(bState.oldSnapshotsCount);
}

HEAT_ZONE_SNAPSHOT void StockCompute::stepSnapshot()
{
    fState.snapshots.push_back(fState.currSnapshot);
    fState.currSnapshot.numTrades = 0;
    fState.currSnapshot.volume = 0;
    fState.currSnapshot.amount = 0;
}

namespace
{

HEAT_ZONE_COMPUTE void computeSkewKurt(double &skew, double &kurt, double *first, double *last)
{
    size_t n = last - first;
    if (n <= 3) {
        skew = kurt = NAN;
        return;
    }

    double sum = 0;
    for (double *p = first; p != last; ++p) {
        sum += *p;
    }
    double mean = sum / n;

    double variance = 0;
    double skewness = 0;
    double kurtosis = 0;
    for (double *p = first; p != last; ++p) {
        double value = *p - mean;
        double valueSqr = value * value;
        variance += valueSqr;
        skewness += valueSqr * value;
        kurtosis += valueSqr * valueSqr;
    }

    if (variance <= 0) {
        skew = kurt = 0;
        return;
    }
    double var = variance / n;
    double std = std::sqrt(var);

    skew = skewness / (n * var * std);
    kurt = kurtosis / (n * var * var) - 3;

    skew = skew * std::sqrt(n * (n - 1)) / (n - 2);
    kurt = (kurt * (n + 1) + 6) * (n - 1) / ((n - 2) * (n - 3));
}

}

HEAT_ZONE_COMPUTE bool StockCompute::decideWantBuy()
{
    if (openPrice == 0) {
        return false;
    }

    /* momentum factors */
    {
        for (int32_t m = 0; m < kMomentumDurations.size(); ++m) {
            auto &factor = factorList.momentum[m];

            if (fState.snapshots.size() >= kMomentumDurations[m] + 1) {
                {
                    double sum = 0;
                    double sumSqr = 0;
                    double prevPrice = fState.snapshots[0].lastPrice;
                    for (int32_t t = 1; t < kMomentumDurations[m] + 1; ++t) {
                        double currPrice = fState.snapshots[t].lastPrice;
                        double changeRate = currPrice / prevPrice - 1.0;
                        sum += changeRate;
                        sumSqr += changeRate * changeRate;
                        prevPrice = currPrice;
                    }
                    double mean = sum * (1.0 / kMomentumDurations[m]);
                    double variance = (sumSqr - sum * sum * (1.0 / (kMomentumDurations[m] - 1))) * (1.0 / (kMomentumDurations[m] - 1));
                    double std = std::sqrt(std::max(0.0, variance));

                    factor.openMean = mean;
                    factor.openStd = std;
                    factor.openZScore = std > 1e-10 ? mean / std : 0.0;
                }

                {
                    double sum = 0;
                    double sumSqr = 0;
                    double prevPrice = prevPrice = fState.snapshots[fState.snapshots.size() - kMomentumDurations[m] - 1].lastPrice;
                    for (int32_t t = fState.snapshots.size() - kMomentumDurations[m]; t < fState.snapshots.size(); ++t) {
                        double currPrice = fState.snapshots[t].lastPrice;
                        double changeRate = currPrice / prevPrice - 1.0;
                        sum += changeRate;
                        sumSqr += changeRate * changeRate;
                        prevPrice = currPrice;
                    }
                    double mean = sum * (1.0 / kMomentumDurations[m]);
                    double variance = (sumSqr - sum * sum * (1.0 / (kMomentumDurations[m] - 1))) * (1.0 / (kMomentumDurations[m] - 1));
                    double std = std::sqrt(std::max(0.0, variance));

                    factor.highMean = mean;
                    factor.highStd = std;
                    factor.highZScore = std > 1e-10 ? mean / std : 0.0;
                }

                factor.diffMean = factor.openMean - factorList.momentum[m].highMean;
                factor.diffZScore = factor.openZScore - factor.highZScore;

            } else {
                std::memset(&factor, -1, sizeof(factor));
            }
        }
    }

    /* volatility factors */
    {
        std::memset(&factorList.volatility, -1, sizeof(factorList.volatility));

        size_t n = (fState.snapshots.size() + 4) / 5;
        if (n != 0) {
            std::vector<int64_t> volumeGather(n);
            std::vector<int64_t> amountGather(n);

            for (size_t i = 0; i < fState.snapshots.size(); ++i) {
                volumeGather[i / 5] += fState.snapshots[i].volume;
                amountGather[i / 5] += fState.snapshots[i].amount;
            }

            std::vector<double> vwap(n);
            double lastVWAP = openPrice;
            for (size_t i = 0; i < n; ++i) {
                if (volumeGather[i] != 0) {
                    lastVWAP = static_cast<double>(amountGather[i]) / static_cast<double>(volumeGather[i]);
                }
                vwap[i] = lastVWAP;
            }

            double p0 = stockState().preClosePrice;
            double p4 = p0 * 1.04;
            double p5 = p0 * 1.05;
            double p6 = p0 * 1.06;
            double p7 = p0 * 1.07;
            double p8 = p0 * 1.08;
            double p9 = p0 * 1.09;

            struct LCSRange
            {
                int64_t amount{};
                int64_t volume{};
                int32_t count{};

                int32_t start{0};
                int32_t stop{-1};
                int32_t curr{-1};
                int32_t last{-1};

                void lcsAdd(int32_t i)
                {
                    if (curr == -1 || i != last + 1) {
                        curr = i;
                    } else if (i - curr >= stop - start) {
                        start = curr;
                        stop = i;
                    }
                    last = i;
                }
            };

            LCSRange ranges[3];
            for (size_t i = 0; i < n; ++i) {
                double v = vwap[i];
                if (v >= p4 && v <= p7) {
                    ranges[0].lcsAdd(i);
                    ranges[0].amount += amountGather[i];
                    ranges[0].volume += volumeGather[i];
                    ++ranges[0].count;
                }
                if (v >= p5 && v <= p8) {
                    ranges[1].lcsAdd(i);
                    ranges[1].amount += amountGather[i];
                    ranges[1].volume += volumeGather[i];
                    ++ranges[1].count;
                }
                if (v >= p6 && v <= p9) {
                    ranges[2].lcsAdd(i);
                    ranges[2].amount += amountGather[i];
                    ranges[2].volume += volumeGather[i];
                    ++ranges[2].count;
                }
            }

            double floatMV = factorList.rawFactors[FactorEnum::circ_mv];
            int64_t totalAmount = 0;
            int64_t totalVolume = 0;
            for (size_t i = 0; i < fState.snapshots.size(); ++i) {
                totalAmount += fState.snapshots[i].amount;
                totalVolume += fState.snapshots[i].volume;
            }

            computeSkewKurt(factorList.volatility[0].vwapSkew, factorList.volatility[0].vwapKurt, vwap.data(), vwap.data() + vwap.size());
            factorList.volatility[2].vwapSkew = factorList.volatility[1].vwapSkew = factorList.volatility[0].vwapSkew;
            factorList.volatility[2].vwapKurt = factorList.volatility[1].vwapKurt = factorList.volatility[0].vwapKurt;

            for (int32_t l = 0; l < 3; ++l) {
                auto &range = ranges[l];
                auto &factor = factorList.volatility[l];

                if (range.start <= range.stop) {
                    double sumSqr = 0;
                    double sumPosSqr = 0;
                    double sumPosSqrSqr = 0;
                    int32_t nPositive = 0;
                    for (int32_t i = std::max(1, range.start); i <= range.stop; ++i) {
                        double r = vwap[i] / vwap[i - 1] - 1.0;
                        double r2 = r * r;
                        sumSqr += r2;
                        if (r > 0) {
                            sumPosSqr += r2;
                            sumPosSqrSqr += r2 * r2;
                            ++nPositive;
                        }
                    }

                    if (sumSqr != 0) {
                        factor.upVolume = std::sqrt(sumPosSqr);
                        factor.upRatio = sumPosSqr / sumSqr;
                        if (nPositive > 1) {
                            factor.upStd = std::sqrt(std::max(0.0, (sumPosSqrSqr - (sumPosSqr * sumPosSqr) / (nPositive - 1)) / (nPositive - 1)));
                        }
                    }

                    factor.time = range.count * 0.25;
                    factor.consecTime = (range.stop - range.start + 1) * 0.25;
                    factor.turnover = static_cast<double>(range.amount) / floatMV * 0.01;
                    factor.amountRatio = static_cast<double>(range.amount) / totalAmount;
                    factor.volumeRatio = static_cast<double>(range.volume) / totalVolume;

                    int64_t lcsAmount = 0;
                    int64_t lcsVolume = 0;
                    for (int32_t i = range.start; i <= range.stop; ++i) {
                        lcsAmount += amountGather[i];
                        lcsVolume += volumeGather[i];
                    }
                    factor.consecTurnover = static_cast<double>(lcsAmount) / floatMV * 0.01;
                    factor.consecAmountRatio = static_cast<double>(lcsAmount) / totalAmount;
                    factor.consecVolumeRatio = static_cast<double>(lcsVolume) / totalVolume;

                    computeSkewKurt(factor.consecVwapSkew, factor.consecVwapKurt, vwap.data() + range.start, vwap.data() + range.stop + 1);
                }
            }
        }
    }

    return computeModel();
}

HEAT_ZONE_COMPUTE bool StockCompute::computeModel()
{
    /* LightGBM here */
    return true;
}

[[gnu::always_inline]] int32_t StockCompute::stockIndex() const
{
    return this - MDD::g_stockComputes.get();
}

[[gnu::always_inline]] StockState &StockCompute::stockState() const
{
    return MDD::g_stockStates[stockIndex()];
}
