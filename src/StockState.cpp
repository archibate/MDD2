#include "StockState.h"
#include "config.h"
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
#elif NE
    switch (stat.marketType) {
        case NescForesight::SSE:
            preClosePrice = static_cast<int32_t>(std::round(stat.staticSseInfo.prevClosePx * 100));
            upperLimitPrice = static_cast<int32_t>(std::round(stat.staticSseInfo.upperLimitPrice * 100));
#if SH
            upperLimitPrice1000 = static_cast<uint32_t>(upperLimitPrice) * 10;
#endif
            break;
        case NescForesight::SZE:
            preClosePrice = static_cast<int32_t>(std::round(stat.staticSzInfo.prevClosePx * 100));
            upperLimitPrice = static_cast<int32_t>(std::round(stat.staticSzInfo.upperLimitPrice * 100));
#if SZ
            upperLimitPrice10000 = static_cast<uint64_t>(upperLimitPrice) * 100;
#endif
            break;
        default:
            SPDLOG_ERROR("failed to get static: stock={}", stockCode);
            return;
    }
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
#if REPLAY
        endSign.stock = 0;
        endSign.timestamp = timestamp;
#elif NE && SH
        endSign.tickMergeSse.tickType = 0;
        endSign.tickMergeSse.tickTime = timestamp / 10;
        endSign.tickMergeSse.tickBSFlag = timestamp % 10;
#elif NE && SZ
        endSign.tradeSz.transactTime = timestamp;
#endif
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
#error not implemented
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

int32_t StockState::stockIndex() const
{
    return this - MDD::g_stockStates.get();
}

StockCompute &StockState::stockCompute() const
{
    return MDD::g_stockComputes[stockIndex()];
}
