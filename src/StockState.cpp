#include "StockState.h"
#include "MDD.h"
#include "L2/timestamp.h"
#include "heatZone.h"
#include <spdlog/spdlog.h>


void StockState::start()
{
    alive = true;

    stockCode = MDD::g_stockCodes[stockIndex()];
    auto stat = MDS::getStatic(stockCode);
    upperLimitPrice = stat.upperLimitPrice;
    preClosePrice = stat.preClosePrice;
    openPrice = stat.openPrice;

    upperLimitPriceApproach = static_cast<int32_t>(std::floor(upperLimitPrice / 1.02)) - 2;
}

void StockState::stop()
{
    alive = false;
    MDD::g_tickCaches[stockIndex()].pushTick({});
}

HEAT_ZONE_TICK void StockState::onTick(MDS::Tick &tick)
{
    if (!alive) [[unlikely]] {
        return;
    }

    bool limitUp = tick.buyOrderNo != 0 && tick.sellOrderNo == 0 && tick.quantity > 0
        && tick.price == upperLimitPrice && tick.timestamp >= 9'30'00'000;
    if (limitUp) {
        bool wantBuy = MDD::g_tickCaches[stockIndex()].checkWantBuyAtTimestamp(tick.timestamp);
        if (wantBuy) [[likely]] {
            /* send buy request */;
        }

        SPDLOG_CRITICAL("limit up: stock={} timestamp={} wantBuy={}", stockCode, tick.timestamp, wantBuy);
        stop();
        return;
    }

    MDD::g_tickCaches[stockIndex()].pushTick(tick);
}

[[gnu::always_inline]] int32_t StockState::stockIndex() const
{
    return this - MDD::g_stockStates.get();
}

[[gnu::always_inline]] StockCompute &StockState::stockCompute() const
{
    return MDD::g_stockComputes[stockIndex()];
}
