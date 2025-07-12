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

    if (tick.isOrder()) {
        if (tick.isOrderCancel()) {
            onCancel(tick);
        } else {
            onOrder(tick);
        }

    } else if (tick.isTrade()) {
        onTrade(tick);

    // } else if (tick.isMeta()) {
    //     on10ms(tick.timestamp);
    }

    timestampLastTick = tick.timestamp;
}

void StockState::handle10ms()
{
    timestampLastTick = L2::millisecondsToTimestamp(L2::timestampToMilliseconds(timestampLastTick) + 10);
    SPDLOG_INFO("handle 10ms idle: stock={} timestamp={}", stockCode, timestampLastTick);
    on10ms(timestampLastTick);
}

#if SH
void StockState::onOrder(MDS::Tick &tick)
{
    if (tick.price == upperLimitPrice && tick.isSellOrder()) {
        upSellOrders.insert({tick.orderNo(), tick.orderQuantity()});
    }

    bool limitUp = !tick.isOpenCall() && tick.isBuyOrder() && tick.price == upperLimitPrice;
    if (limitUp) [[likely]] {
        if (timestampLast10ms != tick.timestamp) [[unlikely]] {
            SPDLOG_WARN("update 10ms on my way: stock={} timestamp={}", stockCode, tick.timestamp);
            on10ms(tick.timestamp);
        }

        // send buy order.
        SPDLOG_CRITICAL("limit up: stock={} timestamp={}", stockCode, tick.timestamp);
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
    for (auto const &[sellOrderNo, quantity]: upTrades) {
        auto it = upSellOrders.find(sellOrderNo);
        if (it != upSellOrders.end()) [[unlikely]] {
            it->second -= quantity;
            if (it->second <= 0) {
                upSellOrders.erase(it);
            }
        }
    }
    upTrades.clear();

    updateVirtUpTrade(timestamp);
    timestampLast10ms = timestamp;
}

void StockState::onTrade(MDS::Tick &tick)
{
    if (tick.price == upperLimitPrice) {
        upTrades.push_back({tick.sellOrderNo, tick.quantity});

    } else {
        addTrade(tick.timestamp, tick.price, tick.quantity);
    }
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
}

void StockState::updateVirtUpTrade(int32_t timestamp)
{
}
