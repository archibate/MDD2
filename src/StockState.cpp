#include "StockState.h"
#include "stockCodes.h"
#include "L2/timestamp.h"
#include <spdlog/spdlog.h>


void StockState::start(int32_t stockIndex)
{
    alive = true;

    stockCode = kStockCodes[stockIndex];
    auto stat = MDS::getStatic(stockCode);
    upperLimitPrice = stat.upperLimitPrice;
    preClosePrice = stat.preClosePrice;
    openPrice = stat.openPrice;

    // upperLimitPriceApproach = std::min(static_cast<int32_t>(std::floor(upperLimitPrice / 1.02)), upperLimitPrice - 10) - 1;
    upperLimitPriceApproach = static_cast<int32_t>(std::floor(upperLimitPrice / 1.02)) - 2;
    fState.currSnapshot.lastPrice = preClosePrice;
    fState.currSnapshot.numTrades = 0;
    fState.currSnapshot.quantity = 0;
    fState.currSnapshot.amount = 0;
    fState.nextTickTimestamp = 9'30'00'000;

    // if (preClosePrice <= 2 || preClosePrice >= 500) {
    //     SPDLOG_WARN("dismissed due to abnormal price: stock={} preClosePrice={}", stockCode, preClosePrice);
    //     stop();
    // }
}

void StockState::stop()
{
    alive = false;
}

void StockState::handleTick(MDS::Tick &tick)
{
    if (!alive) [[unlikely]] {
        return;
    }

#if SH
    if (tick.timestamp == -1) {
        onTimer();
        return;
    }
#endif

    if (tick.isOrder()) {
        if (tick.isOrderCancel()) {
            onCancel(tick);
        } else {
            onOrder(tick);
        }

    } else if (tick.isTrade()) {
        onTrade(tick);
    }

#if SH
    timestampLastTick = tick.timestamp;
#endif
}

#if SH
void StockState::onTimer()
{
#if REPLAY_REAL_TIME
    timestampLastTick = L2::millisecondsToTimestamp(L2::timestampToMilliseconds(timestampLastTick) + 10);
    if (timestampLastTick >= 9'30'00'000) [[likely]] {
        virtPred100ms(timestampLastTick);
    }
#endif
}
#endif

#if SH
void StockState::onOrder(MDS::Tick &tick)
{
    bool limitUp = tick.isBuyOrder() && tick.price == upperLimitPrice && tick.timestamp >= 9'30'00'000;
    if (limitUp) [[likely]] {
        bool virtPredNotReady = tick.timestamp > timestampVirtPred100ms;
        bool onFlyCompute;
        if (virtPredNotReady) [[unlikely]] {
            if (tick.timestamp <= 9'30'01'000) [[unlikely]] {
                virtPredNotReady = onFlyCompute = wantBuy = false;
            } else {
                onFlyCompute = timestampVirtPred100ms == 0;
                if (onFlyCompute) {
                    virtPred100ms(tick.timestamp);
                }
            }
        }

        if (wantBuy) [[likely]] {
            // send buy order.
        }

        if (virtPredNotReady) {
            SPDLOG_WARN("virtual trade prediction not ready: stock={} timestamp={}", stockCode, tick.timestamp);
            if (onFlyCompute) {
                SPDLOG_WARN("updated 100ms prediction on-the-fly: stock={} timestamp={}", stockCode, tick.timestamp);
            }
        }
        SPDLOG_CRITICAL("limit up: stock={} timestamp={} wantBuy={}", stockCode, tick.timestamp, wantBuy);
        stop();
        return;
    }

    if (tick.price >= upperLimitPriceApproach && tick.isSellOrder()) {
        UpSell sell;
        sell.price = tick.price;
        sell.quantity = tick.quantity;
        upSellOrders.insert({tick.orderNo(), sell});
        upSellChangeSinceVirtPred += tick.quantity;
    }
}

void StockState::onCancel(MDS::Tick &tick)
{
    if (tick.isSellOrder()) {
        auto it = upSellOrders.find(tick.orderNo());
        if (it != upSellOrders.end()) {
            upSellChangeSinceVirtPred -= it->second.quantity;
            upSellOrders.erase(it);
        }
    }
}

void StockState::onTrade(MDS::Tick &tick)
{
// #if 1
    auto &trade = pendTrades.emplace_back();
    trade.timestamp = tick.timestamp;
    trade.sellOrderNo = tick.sellOrderNo;
    trade.quantity = tick.quantity;
    trade.price = tick.price;
// #else
//     auto it = upSellOrders.find(tick.sellOrderNo);
//     if (it != upSellOrders.end()) {
//         it->second.quantity -= tick.quantity;
//         if (it->second.quantity <= 0) {
//             upSellOrders.erase(it);
//         }
//     }
//
//     if (tick.price >= upperLimitPriceApproach) {
//         approchingLimitUp = true;
//     }
//     addTrade(tick.timestamp, tick.price, tick.quantity);
// #endif
}

void StockState::virtPred100ms(int32_t timestamp)
{
    bool pendTradeUpdated = !pendTrades.empty();
    if (pendTradeUpdated) {
        approchingLimitUp = false;
        for (auto const &trade: pendTrades) {
            auto it = upSellOrders.find(trade.sellOrderNo);
            if (it != upSellOrders.end()) {
                it->second.quantity -= trade.quantity;
                if (it->second.quantity <= 0) {
                    upSellOrders.erase(it);
                }
            }

            if (trade.price >= upperLimitPriceApproach) {
                approchingLimitUp = true;
            }
            addTrade(trade.timestamp, trade.price, trade.quantity);
        }
        pendTrades.clear();
    }

    if (approchingLimitUp && timestamp >= 9'30'00'000) {
        if (timestamp > timestampVirtPred100ms || pendTradeUpdated || upSellChangeSinceVirtPred != 0) {
            updateVirtTradePred(timestamp);
            timestampVirtPred100ms = (timestamp + 90) / 100 * 100;
            upSellChangeSinceVirtPred = 0;
        }
    } else {
        timestampVirtPred100ms = 0;
    }
}

void StockState::updateVirtTradePred(int32_t timestamp)
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
    fState.currSnapshot.lastPrice = upperLimitPrice;
    stepSnapshot();
    decideWantBuy();
    restoreSnapshot();
}
#endif

#if SZ
void StockState::onOrder(MDS::Tick &tick)
{
    if (tick.price >= upperLimitPriceApproach && tick.isSellOrder()) {
        UpSell sell;
        sell.price = tick.price;
        sell.quantity = tick.quantity;
        upSellOrders.insert({tick.orderNo(), sell});
        upSellChangeSinceVirtPred += tick.quantity;
    }
}

void StockState::onCancel(MDS::Tick &tick)
{
    if (tick.price == upperLimitPrice && tick.isSellOrder()) {
        upSellOrders.erase(tick.orderNo());
    }
}

void StockState::onTrade(MDS::Tick &tick)
{
    if (tick.price == upperLimitPrice) {
        auto it = upSellOrders.find(tick.sellOrderNo);
        if (it != upSellOrders.end()) [[unlikely]] {
            it->second -= tick.quantity;
            if (it->second <= 0) {
                upSellOrders.erase(it);
            }
        }
    }
}
#endif

void StockState::addTrade(int32_t timestamp100ms, int32_t price, int32_t quantity)
{
    stepSnapshotUntil(timestamp100ms);

    int64_t q64 = quantity;
    fState.currSnapshot.lastPrice = price;
    ++fState.currSnapshot.numTrades;
    fState.currSnapshot.quantity += q64;
    fState.currSnapshot.amount += price * q64;
}

void StockState::stepSnapshotUntil(int32_t timestamp)
{
    if (timestamp > fState.nextTickTimestamp) {
        stepSnapshot();
        while (timestamp > (fState.nextTickTimestamp =
            L2::positiveAbsoluteMillisecondsToTimestamp(
                L2::timestampToPositiveAbsoluteMilliseconds(
                    fState.nextTickTimestamp) + 100))) {
            stepSnapshot();
        }
    }
}

void StockState::saveSnapshot()
{
    bState.savingMode = true;
    bState.nextTickTimestamp = fState.nextTickTimestamp;
    bState.currSnapshot = fState.currSnapshot;
    bState.oldSnapshotsCount = fState.snapshots.size();
}

void StockState::restoreSnapshot()
{
    bState.savingMode = false;
    fState.nextTickTimestamp = bState.nextTickTimestamp;
    fState.currSnapshot = bState.currSnapshot;
    fState.snapshots.resize(bState.oldSnapshotsCount);
}

void StockState::stepSnapshot()
{
    fState.snapshots.push_back(fState.currSnapshot);
    fState.currSnapshot.numTrades = 0;
    fState.currSnapshot.quantity = 0;
    fState.currSnapshot.amount = 0;
}

void StockState::decideWantBuy()
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

    factorList.dumpFactors(fState.nextTickTimestamp, stockCode);
    wantBuy = true;
}
