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

    upperLimitPriceApproach = static_cast<int32_t>(std::floor(upperLimitPrice / 1.02)) - 2;
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
    virtPred100ms(timestampLastTick);
}
#endif

#if SH
void StockState::onOrder(MDS::Tick &tick)
{
    bool limitUp = !tick.isOpenCall() && tick.isBuyOrder() && tick.price == upperLimitPrice;
    if (limitUp) [[likely]] {
        bool virtPredNotReady = timestampVirtPred100ms != tick.timestamp / 100;
        bool onFlyCompute;
        if (virtPredNotReady) [[unlikely]] {
            onFlyCompute = !wantBuy || timestampVirtPred100ms == 0;
            if (onFlyCompute) {
                virtPred100ms(tick.timestamp);
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

    if (tick.price == upperLimitPrice && tick.isSellOrder()) {
        upSellOrders.insert({tick.orderNo(), tick.quantity});
        upSellChangeSinceVirtPred += tick.quantity;
    }
}

void StockState::onCancel(MDS::Tick &tick)
{
    if (tick.price == upperLimitPrice && tick.isSellOrder()) {
        auto it = upSellOrders.find(tick.orderNo());
        if (it != upSellOrders.end()) [[likely]] {
            upSellChangeSinceVirtPred -= it->second;
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
    bool pendTradeUpdated = !pendTrades.empty();
    if (pendTradeUpdated) {
        approchingLimitUp = false;
        for (auto const &trade: pendTrades) {
            if (trade.price == upperLimitPrice) {
                auto it = upSellOrders.find(trade.sellOrderNo);
                if (it != upSellOrders.end()) [[unlikely]] {
                    it->second -= trade.quantity;
                    if (it->second <= 0) {
                        upSellOrders.erase(it);
                    }
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
        if (timestampVirtPred100ms != timestamp / 100 || pendTradeUpdated || upSellChangeSinceVirtPred != 0) {
            updateVirtTradePred(timestamp);
            timestampVirtPred100ms = timestamp / 100;
            upSellChangeSinceVirtPred = 0;
        }
    } else {
        timestampVirtPred100ms = 0;
    }
}

void StockState::updateVirtTradePred(int32_t timestamp)
{
    // SPDLOG_DEBUG("updating virtual trade: stock={} timestamp={}", stockCode, timestamp);
    for (auto const &[sellOrderId, quantity]: upSellOrders) {
        addTrade(timestamp, upperLimitPrice, quantity); // virtually
    }
    // compute factors here...
    wantBuy = true;
}
#endif

#if SZ
void StockState::onOrder(MDS::Tick &tick)
{
    if (tick.price == upperLimitPrice && tick.isSellOrder()) {
        upSellOrders.insert({tick.orderNo(), tick.quantity});
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

void StockState::addTrade(int32_t timestamp, int32_t price, int32_t quantity)
{
    // update states here...
}
