#include "StockState.h"
#include "stockCodes.h"
#include "L2/timestamp.h"
#include <spdlog/spdlog.h>

namespace
{

int32_t timestampRound100ms(int32_t timestamp)
{
    return (timestamp + 90) / 100;
}

}


void StockState::start(int32_t stockIndex)
{
    alive = true;

    stockCode = kStockCodes[stockIndex];
    auto stat = MDS::getStatic(stockCode);
    upperLimitPrice = stat.upperLimitPrice;
    preClosePrice = stat.preClosePrice;

    // upperLimitPriceApproach = std::min(static_cast<int32_t>(std::floor(upperLimitPrice / 1.02)), upperLimitPrice - 10) - 1;
    upperLimitPriceApproach = static_cast<int32_t>(std::floor(upperLimitPrice / 1.02)) - 2;
    fState.currSnapshot.lastPrice = preClosePrice;
    fState.snapshotTimestamp100ms = 9'30'00'0;

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
    timestampLastTick = L2::millisecondsToTimestamp(L2::timestampToMilliseconds(timestampLastTick) + 10);
    if (timestampLastTick >= 9'30'00'000) {
        virtPred100ms(timestampLastTick);
    }
}
#endif

#if SH
void StockState::onOrder(MDS::Tick &tick)
{
    bool limitUp = tick.isBuyOrder() && tick.price == upperLimitPrice && tick.timestamp >= 9'30'00'000;
    if (limitUp) [[likely]] {
        bool virtPredNotReady = timestampVirtPred100ms != timestampRound100ms(tick.timestamp);
        bool onFlyCompute;
        if (virtPredNotReady) [[unlikely]] {
            if (tick.timestamp <= 9'30'00'100) [[unlikely]] {
                virtPredNotReady = onFlyCompute = wantBuy = false;
            } else {
                onFlyCompute = !wantBuy || timestampVirtPred100ms == 0;
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
    auto &trade = pendTrades.emplace_back();
    trade.timestamp = tick.timestamp;
    trade.sellOrderNo = tick.sellOrderNo;
    trade.quantity = tick.quantity;
    trade.price = tick.price;
}

void StockState::virtPred100ms(int32_t timestamp)
{
    int32_t timestamp100ms = timestampRound100ms(timestamp);

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
            addTrade(timestampRound100ms(trade.timestamp), trade.price, trade.quantity);
        }
        pendTrades.clear();
    }

    if (approchingLimitUp && timestamp >= 9'30'00'000) {
        if (timestampVirtPred100ms != timestamp100ms || pendTradeUpdated || upSellChangeSinceVirtPred != 0) {
            updateVirtTradePred(timestamp100ms);
            timestampVirtPred100ms = timestamp100ms;
            upSellChangeSinceVirtPred = 0;
        }
    } else {
        timestampVirtPred100ms = 0;
    }
}

void StockState::updateVirtTradePred(int32_t timestamp100ms)
{
    saveSnapshot();
    if (timestamp100ms > fState.snapshotTimestamp100ms) {
        stepSnapshot();
        fState.prevSnapshot = fState.currSnapshot;
        fState.currSnapshot.numTrades = 0;
        fState.currSnapshot.quantity = 0;
        fState.currSnapshot.amount = 0;
        while (timestamp100ms > (fState.snapshotTimestamp100ms =
            timestampRound100ms(L2::positiveAbsoluteMillisecondsToTimestamp(
                L2::timestampToPositiveAbsoluteMilliseconds(
                    fState.snapshotTimestamp100ms * 100) + 100)))) {
            stepSnapshot();
        }
    }

    for (auto const &[sellOrderId, sell]: upSellOrders) {
        int64_t q64 = sell.quantity;
        fState.currSnapshot.lastPrice = sell.price;
        ++fState.currSnapshot.numTrades;
        fState.currSnapshot.quantity += q64;
        fState.currSnapshot.amount += sell.price * q64;
    }
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
    if (timestamp100ms > fState.snapshotTimestamp100ms) {
        stepSnapshot();
        fState.prevSnapshot = fState.currSnapshot;
        fState.currSnapshot.numTrades = 0;
        fState.currSnapshot.quantity = 0;
        fState.currSnapshot.amount = 0;
        while (timestamp100ms > (fState.snapshotTimestamp100ms =
            timestampRound100ms(L2::positiveAbsoluteMillisecondsToTimestamp(
                L2::timestampToPositiveAbsoluteMilliseconds(
                    fState.snapshotTimestamp100ms * 100) + 100)))) {
            stepSnapshot();
        }
    }

    int64_t q64 = quantity;
    fState.currSnapshot.lastPrice = price;
    ++fState.currSnapshot.numTrades;
    fState.currSnapshot.quantity += q64;
    fState.currSnapshot.amount += price * q64;
}

void StockState::saveSnapshot()
{
    bState.savingMode = true;
    bState.snapshotTimestamp100ms = fState.snapshotTimestamp100ms;
    bState.currSnapshot = fState.currSnapshot;
    bState.prevSnapshot = fState.prevSnapshot;
    bState.priceSeqOldSize = fState.priceSeq.size();
}

void StockState::restoreSnapshot()
{
    bState.savingMode = false;
    fState.snapshotTimestamp100ms = bState.snapshotTimestamp100ms;
    fState.currSnapshot = bState.currSnapshot;
    fState.prevSnapshot = bState.prevSnapshot;
    fState.priceSeq.resize(bState.priceSeqOldSize);
}

void StockState::stepSnapshot()
{
}

void StockState::decideWantBuy()
{
    wantBuy = true;
}
