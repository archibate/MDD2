#include "StockState.h"
#include "MDD.h"
#include "timestamp.h"
#include "TickCache.h"
#include "heatZone.h"
#include <spdlog/spdlog.h>
#include <atomic>


void StockState::start()
{
    alive = true;

    stockCode = MDD::g_stockCodes[stockIndex()];
    auto stat = MDS::getStatic(stockCode);
    preClosePrice = stat.preClosePrice;
    upperLimitPrice = stat.upperLimitPrice;
    floatMV = stat.floatMV;
    openPrice = 0;

    upperLimitPriceApproach = static_cast<int32_t>(std::floor(upperLimitPrice * 0.996)) - 2;
    SPDLOG_TRACE("initial static: stock={} preClose={} upperLimit={} upperLimitApproach={}",
                 stockCode, preClosePrice, upperLimitPrice, upperLimitPriceApproach);
}

void StockState::stop(int32_t timestamp)
{
    if (alive) {
        alive = false;
        MDS::Tick endSign{};
        endSign.timestamp = timestamp;
        MDD::g_tickCaches[stockIndex()].pushTick(endSign);
    }
}

HEAT_ZONE_TICK void StockState::onTick(MDS::Tick &tick)
{
    if (!alive) [[unlikely]] {
        return;
    }

    bool limitUp = tick.buyOrderNo != 0 && tick.sellOrderNo == 0 && tick.quantity > 0
        && tick.price == upperLimitPrice && tick.timestamp >= 9'30'00'000 && tick.timestamp < 14'57'00'000;
    if (limitUp) {
        auto intent = MDD::g_tickCaches[stockIndex()].checkWantBuyAtTimestamp(tick.timestamp);
        if (intent == TickCache::WantBuy) [[likely]] {
            /* send buy request */;
        }

        stop(tick.timestamp + static_cast<int32_t>(intent));
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
