#include "StockState.h"
#include "MDD.h"
#include "OES.h"
#include "timestamp.h"
#include "TickCache.h"
#include "heatZone.h"
#include <spdlog/spdlog.h>
#include <atomic>


void StockState::start()
{
    alive = true;

    stockCode = MDD::g_stockCodes[stockIndex()];
    tickCache = &MDD::g_tickCaches[stockIndex()];

    auto stat = MDS::getStatic(stockCode);
#if REPLAY
    preClosePrice = stat.preClosePrice;
    upperLimitPrice = stat.upperLimitPrice;
#else NE
#endif

    SPDLOG_TRACE("initial static: stock={} preClose={} upperLimit={}",
                 stockCode, preClosePrice, upperLimitPrice);

    reqOrder = {};
    reqOrder.stockCode = stockCode;
    reqOrder.price = upperLimitPrice;
    reqOrder.quantity = 100;
    reqOrder.limitType = 'U';
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

#if REPLAY
    bool limitUp = tick.buyOrderNo != 0 && tick.sellOrderNo == 0 && tick.quantity > 0
        && tick.price == upperLimitPrice && tick.timestamp >= 9'30'00'000 && tick.timestamp < 14'57'00'000;
    if (limitUp) {
        auto intent = tickCache->checkWantBuyAtTimestamp(tick.timestamp);
        if (intent == TickCache::WantBuy) [[likely]] {
            OES::sendRequest(reqOrder);
        }

        stop(tick.timestamp + static_cast<int32_t>(intent));
        return;
    }
#elif NE && SH
    bool limitUp = tick.tickMergeSse.tickType == 'A' && tick.tickMergeSse.tickBSFlag == '0'
        && tick.tickMergeSse.price == upperLimitPrice1000 && tick.tickMergeSse.tickTime >= 9'30'00'00
        && tick.tickMergeSse.tickTime < 14'57'00'00;
    if (limitUp) {
        int32_t timestamp = tick.tickMergeSse.tickTime * 10;
        auto intent = tickCache->checkWantBuyAtTimestamp(timestamp);
        if (intent == TickCache::WantBuy) [[likely]] {
            OES::sendRequest(reqOrder);
        }

        stop(timestamp + static_cast<int32_t>(intent));
        return;
    }

#elif NE && SZ
    static_assert(0, "not implemented");
#endif

    tickCache->pushTick(tick);
}

HEAT_ZONE_RSPORDER void StockState::onRspOrder(OES::RspOrder &rspOrder)
{
    if (rspOrder.errorId != 0) {
        SPDLOG_ERROR("response order error: errorId={}", rspOrder.errorId);
        return;
    }
    SPDLOG_INFO("response order: messageType={} stock={} orderStatus={} orderSysId={} orderPrice={} orderQuantity={}", rspOrder.messageType, rspOrder.stockCode, rspOrder.orderStatus, rspOrder.orderSysId, rspOrder.orderPrice, rspOrder.orderQuantity);
}

[[gnu::always_inline]] int32_t StockState::stockIndex() const
{
    return this - MDD::g_stockStates.get();
}

[[gnu::always_inline]] StockCompute &StockState::stockCompute() const
{
    return MDD::g_stockComputes[stockIndex()];
}
