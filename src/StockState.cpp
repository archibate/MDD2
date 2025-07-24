#include "StockState.h"
#include "MDD.h"
#include "stockCodes.h"
#include "L2/timestamp.h"
#include <spdlog/spdlog.h>


void StockState::start()
{
    alive = true;

    stockCode = kStockCodes[stockIndex()];
    auto stat = MDS::getStatic(stockCode);
    upperLimitPrice = stat.upperLimitPrice;
    preClosePrice = stat.preClosePrice;
    openPrice = stat.openPrice;

    upperLimitPriceApproach = static_cast<int32_t>(std::floor(upperLimitPrice / 1.02)) - 2;
}

void StockState::stop()
{
    alive = false;
}

void StockState::onTick(MDS::Tick &tick)
{
    if (!alive) [[unlikely]] {
        return;
    }

    bool limitUp = tick.buyOrderNo != 0 && tick.sellOrderNo == 0 && tick.quantity > 0
        && tick.price == upperLimitPrice && tick.timestamp >= 9'30'00'000;
    if (limitUp) [[likely]] {
        SPDLOG_CRITICAL("limit up: stock={} timestamp={}", stockCode, tick.timestamp);
        stop();
        return;
    }

    MDD::g_tickCaches[stockIndex()].pushTick(tick);
}

void StockState::onOrder(MDS::Tick &tick)
{
}

void StockState::onCancel(MDS::Tick &tick)
{
}

void StockState::onTrade(MDS::Tick &tick)
{
}

int32_t StockState::stockIndex() const
{
    return this - MDD::g_stockStates.get();
}

StockCompute &StockState::stockCompute() const
{
    return MDD::g_stockComputes[stockIndex()];
}
