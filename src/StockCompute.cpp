#include "StockCompute.h"
#include "StockState.h"
#include "MDD.h"
#include "stockCodes.h"
#include "L2/timestamp.h"
#include <spdlog/spdlog.h>


namespace
{

int32_t timestampAdvance100ms(int32_t timestamp)
{
    return L2::positiveAbsoluteMillisecondsToTimestamp(
        L2::timestampToPositiveAbsoluteMilliseconds(
            timestamp) + 100);
}

}


void StockCompute::start()
{
    upperLimitPriceApproach = static_cast<int32_t>(std::floor(stockState().upperLimitPrice / 1.02)) - 2;
    fState.currSnapshot.lastPrice = stockState().preClosePrice;
    fState.currSnapshot.numTrades = 0;
    fState.currSnapshot.quantity = 0;
    fState.currSnapshot.amount = 0;
    fState.nextTickTimestamp = 9'30'00'000;
}

void StockCompute::stop()
{
    alive = false;
    approchingLimitUp = false;
}

void StockCompute::onTick(MDS::Tick &tick)
{
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

void StockCompute::onTimer()
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

    for (auto &tick: ticks) {
        onTick(tick);
    }
}

void StockCompute::onPostTimer()
{
    if (approchingLimitUp) {
        computeFutureWantBuy(fState.nextTickTimestamp);
        approchingLimitUp = false;
    }
}

void StockCompute::onOrder(MDS::Tick &tick)
{
    bool limitUp = tick.isBuyOrder() && tick.price == stockState().upperLimitPrice && tick.timestamp >= 9'30'00'000;
    if (limitUp) [[likely]] {
        stop();
        return;
    }

    if (tick.price >= upperLimitPriceApproach && tick.isSellOrder()) {
        UpSell sell;
        sell.price = tick.price;
        sell.quantity = tick.quantity;
        upSellOrders.insert({tick.orderNo(), sell});
    }
}

void StockCompute::onCancel(MDS::Tick &tick)
{
    if (tick.isSellOrder()) {
        auto it = upSellOrders.find(tick.orderNo());
        if (it != upSellOrders.end()) {
            upSellOrders.erase(it);
        }
    }
}

void StockCompute::onTrade(MDS::Tick &tick)
{
    auto it = upSellOrders.find(tick.sellOrderNo);
    if (it != upSellOrders.end()) {
        it->second.quantity -= tick.quantity;
        if (it->second.quantity <= 0) {
            upSellOrders.erase(it);
        }
    }

    if (tick.price >= upperLimitPriceApproach) {
        approchingLimitUp = true;
    }
    addRealTrade(tick.timestamp, tick.price, tick.quantity);
}

void StockCompute::addRealTrade(int32_t timestamp, int32_t price, int32_t quantity)
{
    stepSnapshotUntil(timestamp);

    int64_t q64 = quantity;
    fState.currSnapshot.lastPrice = price;
    ++fState.currSnapshot.numTrades;
    fState.currSnapshot.quantity += q64;
    fState.currSnapshot.amount += price * q64;
}

void StockCompute::computeFutureWantBuy(int32_t timestamp)
{
    saveSnapshot();
    stepSnapshotUntil(timestamp);

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
    if (wantBuy) {
        wantBuyCurrentIndex.store((wantBuyCurrentIndex.load(std::memory_order_relaxed) + 1)
                                  & (wantBuyTimestamps.size() - 1), std::memory_order_relaxed);
        wantBuyTimestamps[wantBuyCurrentIndex].store(timestamp, std::memory_order_relaxed);
        SPDLOG_INFO("predicted wantBuyTimestamp={}", timestamp);
    }

    restoreSnapshot();
    factorList.dumpFactors(timestamp, stockState().stockCode);
}

void StockCompute::stepSnapshotUntil(int32_t timestamp)
{
    if (timestamp > fState.nextTickTimestamp) {
        stepSnapshot();
        while (timestamp > (fState.nextTickTimestamp = timestampAdvance100ms(fState.nextTickTimestamp))) {
            stepSnapshot();
        }
    }
}

void StockCompute::saveSnapshot()
{
    bState.savingMode = true;
    bState.nextTickTimestamp = fState.nextTickTimestamp;
    bState.currSnapshot = fState.currSnapshot;
    bState.oldSnapshotsCount = fState.snapshots.size();
}

void StockCompute::restoreSnapshot()
{
    bState.savingMode = false;
    fState.nextTickTimestamp = bState.nextTickTimestamp;
    fState.currSnapshot = bState.currSnapshot;
    fState.snapshots.resize(bState.oldSnapshotsCount);
}

void StockCompute::stepSnapshot()
{
    fState.snapshots.push_back(fState.currSnapshot);
    fState.currSnapshot.numTrades = 0;
    fState.currSnapshot.quantity = 0;
    fState.currSnapshot.amount = 0;
}

bool StockCompute::decideWantBuy()
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

    return true;
}

int32_t StockCompute::stockIndex() const
{
    return this - MDD::g_stockComputes.get();
}

StockState &StockCompute::stockState() const
{
    return MDD::g_stockStates[stockIndex()];
}
