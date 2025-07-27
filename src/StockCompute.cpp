#include "StockCompute.h"
#include "StockState.h"
#include "MDD.h"
#include "timestamp.h"
#include "heatZone.h"
#include <spdlog/spdlog.h>

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
    int32_t modelTimestamp = stockCompute.futureTimestamp;
    bool modelTimeCorrect = timestampLinear(modelTimestamp) == minLinearTimestamp;
    if (modelTimeCorrect) {
        stockCompute.factorList.dumpFactors(modelTimestamp, stockState.stockCode);
    }

    SPDLOG_INFO("limit up model status: stock={} tickTimestamp={} roundTimestamp={} modelTimestamp={} minTimestamp={} minDt={} toleranceDt={} modelTimeCorrect={} intent={}", stockState.stockCode, tickTimestamp, timestampLinear((timestampDelinear(tickTimestamp) + 90) / 100 * 100), modelTimestamp, timestampDelinear(minLinearTimestamp), minDt, kWantBuyTimeTolerance, modelTimeCorrect, magic_enum::enum_name(intent));
    SPDLOG_TRACE("limit up detected: stock={} timestamp={} intent={}", stockState.stockCode, tickTimestamp, magic_enum::enum_name(intent));
}

}


void StockCompute::start()
{
    upperLimitPriceApproach = static_cast<int32_t>(std::floor(stockState().upperLimitPrice / 1.02)) - 2;
    fState.currSnapshot.lastPrice = stockState().preClosePrice;
    fState.currSnapshot.numTrades = 0;
    fState.currSnapshot.quantity = 0;
    fState.currSnapshot.amount = 0;
    futureTimestamp = fState.nextTickTimestamp = 9'30'00'000;
}

COLD_ZONE void StockCompute::stop()
{
    alive = false;
    approachingLimitUp = false;
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
        // 600327
        futureTimestamp = fState.nextTickTimestamp;
    }
}

HEAT_ZONE_COMPUTE void StockCompute::onPostTimer()
{
    if (approachingLimitUp) {
        futureTimestamp = timestampAdvance100ms(futureTimestamp);
        computeFutureWantBuy();
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

    if (tick.price >= upperLimitPriceApproach) {
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
    fState.currSnapshot.quantity += q64;
    fState.currSnapshot.amount += price * q64;
}

HEAT_ZONE_COMPUTE void StockCompute::computeFutureWantBuy()
{
    saveSnapshot();
    stepSnapshotUntil(futureTimestamp);

    for (auto const &[sellOrderId, sell]: upSellOrders) {
        int64_t q64 = sell.quantity;
        fState.currSnapshot.lastPrice = sell.price;
        ++fState.currSnapshot.numTrades;
        fState.currSnapshot.quantity += q64;
        fState.currSnapshot.amount += sell.price * q64;
    }
    fState.currSnapshot.lastPrice = stockState().upperLimitPrice;
    stepSnapshot();

    bool wantBuy = decideWantBuy();
    auto &tickCache = MDD::g_tickCaches[stockIndex()];
    tickCache.pushWantBuyTimestamp(futureTimestamp, wantBuy);

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
    fState.currSnapshot.quantity = 0;
    fState.currSnapshot.amount = 0;
}

HEAT_ZONE_COMPUTE bool StockCompute::decideWantBuy()
{
    for (int32_t m = 0; m < kMomentumDurations.size(); ++m) {
        if (fState.snapshots.size() >= kMomentumDurations[m] + 1) {
            {
                double value = 0;
                double valueSqr = 0;
                double prevPrice = fState.snapshots[0].lastPrice;
                for (int32_t t = 1; t < kMomentumDurations[m] + 1; ++t) {
                    double currPrice = fState.snapshots[t].lastPrice;
                    double changeRate = currPrice / prevPrice - 1.0;
                    value += changeRate;
                    valueSqr += changeRate * changeRate;
                    prevPrice = currPrice;
                }
                double mean = value * (1.0 / kMomentumDurations[m]);
                double meanM1 = value * (1.0 / (kMomentumDurations[m] - 1));
                double variance = valueSqr * (1.0 / (kMomentumDurations[m] - 1)) - meanM1 * meanM1;
                double std = std::sqrt(std::max(0.0, variance));

                factorList.momentum[m].openMean = mean;
                factorList.momentum[m].openStd = std;
                factorList.momentum[m].openZScore = std > 1e-10 ? mean / std : 0.0;
            }

            {
                double value = 0;
                double valueSqr = 0;
                double prevPrice = prevPrice = fState.snapshots[fState.snapshots.size() - kMomentumDurations[m] - 1].lastPrice;
                for (int32_t t = fState.snapshots.size() - kMomentumDurations[m]; t < fState.snapshots.size(); ++t) {
                    double currPrice = fState.snapshots[t].lastPrice;
                    double changeRate = currPrice / prevPrice - 1.0;
                    value += changeRate;
                    valueSqr += changeRate * changeRate;
                    prevPrice = currPrice;
                }
                double mean = value * (1.0 / kMomentumDurations[m]);
                double meanM1 = value * (1.0 / (kMomentumDurations[m] - 1));
                double variance = valueSqr * (1.0 / (kMomentumDurations[m] - 1)) - meanM1 * mean;
                double std = std::sqrt(std::max(0.0, variance));

                factorList.momentum[m].highMean = mean;
                factorList.momentum[m].highStd = std;
                factorList.momentum[m].highZScore = std > 1e-10 ? mean / std : 0.0;
            }

            factorList.momentum[m].diffMean = factorList.momentum[m].openMean - factorList.momentum[m].highMean;
            factorList.momentum[m].diffZScore = factorList.momentum[m].openZScore - factorList.momentum[m].highZScore;
        } else {
            factorList.momentum[m].openMean = NAN;
            factorList.momentum[m].openStd = NAN;
            factorList.momentum[m].openZScore = NAN;
            factorList.momentum[m].highMean = NAN;
            factorList.momentum[m].highStd = NAN;
            factorList.momentum[m].highZScore = NAN;
            factorList.momentum[m].diffMean = NAN;
            factorList.momentum[m].diffZScore = NAN;
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
