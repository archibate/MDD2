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

    if (tick.timestamp == -1) {
        handleTimer();
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

#if SH
    timestampLastTick = tick.timestamp;
#endif
}

void StockState::handleTimer()
{
#if SH
    timestampLastTick = L2::millisecondsToTimestamp(L2::timestampToMilliseconds(timestampLastTick) + 10);
    // SPDLOG_INFO("handle 10ms idle: stock={} timestamp={}", stockCode, timestampLastTick);
    on10ms(timestampLastTick);
#endif
}

#if SH
void StockState::onOrder(MDS::Tick &tick)
{
    if (tick.price == upperLimitPrice && tick.isSellOrder()) {
        upSellOrders.insert({tick.orderNo(), tick.orderQuantity()});
    }

    bool limitUp = !tick.isOpenCall() && tick.isBuyOrder() && tick.price == upperLimitPrice;
    if (limitUp) [[likely]] {
        bool virtPredNotReady = timestampVirtTradePred != tick.timestamp;
        if (virtPredNotReady) [[unlikely]] {
            if (!wantBuy || timestampVirtTradePred == 0) {
                on10ms(tick.timestamp);
            }
        }

        if (wantBuy) [[likely]] {
            // send buy order.
        }

        if (virtPredNotReady) {
            SPDLOG_WARN("updated 10ms on-the-fly: stock={} timestamp={}", stockCode, tick.timestamp);
        }
        SPDLOG_CRITICAL("limit up: stock={} timestamp={} wantBuy={}", stockCode, tick.timestamp, wantBuy);
        stop();
    }
}

void StockState::onCancel(MDS::Tick &tick)
{
    if (tick.price == upperLimitPrice && tick.isSellOrder()) {
        upSellOrders.erase(tick.orderNo());
    }
}

void StockState::on10ms(int32_t timestamp)
{
    if (!upTrades.empty()) {
        approchingLimitUp = false;
        for (auto const &trade: upTrades) {
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
        upTrades.clear();
    }

    if (approchingLimitUp && timestamp >= 9'30'00'000) {
        updateVirtTradePred(timestamp);
        timestampVirtTradePred = timestamp;
    } else {
        timestampVirtTradePred = 0;
    }
}

void StockState::onTrade(MDS::Tick &tick)
{
    auto &trade = upTrades.emplace_back();
    trade.timestamp = tick.timestamp;
    trade.sellOrderNo = tick.sellOrderNo;
    trade.quantity = tick.quantity;
    trade.price = tick.price;
}
#endif

#if SZ
void StockState::onOrder(MDS::Tick &tick)
{
}

void StockState::onCancel(MDS::Tick &tick)
{
}

void StockState::onTrade(MDS::Tick &tick)
{
}
#endif

void StockState::addTrade(int32_t timestamp, int32_t price, int32_t quantity)
{
    // update states here...
}

void StockState::updateVirtTradePred(int32_t timestamp)
{
    for (auto const &[sellOrderId, quantity]: upSellOrders) {
        addTrade(timestamp, upperLimitPrice, quantity);
    }
    // compute factors here...
    wantBuy = true;
}
