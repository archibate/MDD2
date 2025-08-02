#include "StockState.h"
#include "config.h"
#include "MDD.h"
#include "OES.h"
#include "timestamp.h"
#include "WantCache.h"
#include "TickRing.h"
#include "heatZone.h"
#include <spdlog/spdlog.h>
#include <atomic>
#include <cstring>
#include <cstdlib>
#include <cstdio>


void StockState::start()
{
    alive = true;

    stockCode = MDD::g_stockCodes[stockIndex()];
    wantCache = &MDD::g_wantCaches[stockIndex()];
}

void StockState::setChannelId(int32_t channelId)
{
    tickRing = &MDD::g_tickRings[channelId];
}

void StockState::setStatic(MDS::Stat const &stat)
{
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

    int32_t quantity = 100;

    SPDLOG_TRACE("initial static: stock={} preClose={} upperLimit={} reportQuantity={}",
                 stockCode, preClosePrice, upperLimitPrice, quantity);

    reqOrder = std::make_unique<OES::ReqOrder>();
    std::memset(reqOrder.get(), 0, sizeof(OES::ReqOrder));
#if REPLAY
    reqOrder->stockCode = stockCode;
    reqOrder->price = upperLimitPrice;
    reqOrder->quantity = quantity;
    reqOrder->limitType = 'U';
#elif XC || NE
    std::sprintf(reqOrder->xeleReq.SecuritiesID, "%06d", stockCode);
    reqOrder->xeleReq.Direction = XELE_ORDER_BUY;
#if SZ
    reqOrder->xeleReq.LimitPrice = stat.staticSzInfo.upperLimitPrice;
#endif
#if SH
    reqOrder->xeleReq.LimitPrice = stat.staticSseInfo.upperLimitPrice;
#endif
    reqOrder->xeleReq.Volume = quantity;
    reqOrder->xeleReq.OrderType = XELE_LIMIT_PRICE_TYPE;
    reqOrder->xeleReq.TimeCondition = XELE_TIMEINFORCE_TYPE_GFD;
    reqOrder->xeleReq.SecuritiesType = '0';
    reqOrder->xeleReq.Operway = API_OPERWAY;
    reqOrder->xeleReq.ExchangeFrontID = 0;
#endif
}

void StockState::stop(int32_t timestamp)
{
    if (alive) {
        alive = false;
        MDS::Tick endSign{};
#if REPLAY
        endSign.quantity = 0;
        endSign.stock = stockCode;
        endSign.timestamp = timestamp;
#elif NE && SH
        endSign.tickMergeSse.tickType = 0;
        std::sprintf(endSign.tickMergeSse.securityID, "%06d", stockCode);
        endSign.tickMergeSse.tickTime = timestamp / 10;
        endSign.tickMergeSse.tickBSFlag = timestamp % 10;
#elif NE && SZ
        endSign.tradeSz.messageType = 0;
        std::sprintf(endSign.tradeSz.securityID, "%06d", stockCode);
        endSign.tradeSz.transactTime = timestamp;
#endif
        tickRing->pushTick(endSign);
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
        auto intent = wantCache->checkWantBuyAtTimestamp(tick.timestamp);
#if ALWAYS_BUY
        intent = WantCache::WantBuy;
#endif
        if (intent == WantCache::WantBuy) [[likely]] {
            OES::sendRequest(*reqOrder);
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
        auto intent = wantCache->checkWantBuyAtTimestamp(timestamp);
#if ALWAYS_BUY
        intent = WantCache::WantBuy;
#endif
        if (intent == WantCache::WantBuy) [[likely]] {
            OES::sendRequest(*reqOrder);
        }

        stop(timestamp + static_cast<int32_t>(intent));
        return;
    }

#elif NE && SZ
#error not implemented
#endif

    tickRing->pushTick(tick);
}

HEAT_ZONE_RSPORDER void StockState::onRspOrder(OES::RspOrder &rspOrder)
{
#if REPLAY
    if (rspOrder.errorId != 0) {
        SPDLOG_ERROR("response order error: errorId={}", rspOrder.errorId);
        return;
    }
    SPDLOG_INFO("response order: messageType={} stock={} orderStatus={} orderSysId={} orderPrice={} orderQuantity={} orderDirection={}", rspOrder.messageType, rspOrder.stockCode, rspOrder.orderStatus, rspOrder.orderSysId, rspOrder.orderPrice, rspOrder.orderQuantity, rspOrder.orderDirection);
#elif XC || NE
    if (rspOrder.xeleRsp.ErrorId != 0) {
        SPDLOG_ERROR("response order error: errorId={}", rspOrder.xeleRsp.ErrorId);
        return;
    }
    SPDLOG_INFO("response order: stock={} orderSysId={} orderPrice={} orderQuantity={} orderDirection={}", std::atoi(rspOrder.xeleRsp.SecuritiesID), rspOrder.xeleRsp.OrderSysID, rspOrder.xeleRsp.LimitPrice, rspOrder.xeleRsp.Volume, rspOrder.xeleRsp.Direction == '1' ? "BUY" : rspOrder.xeleRsp.Direction == '2' ? "SELL" : "WARM");
#endif
}

int32_t StockState::stockIndex() const
{
    return this - MDD::g_stockStates.get();
}

StockCompute &StockState::stockCompute() const
{
    return MDD::g_stockComputes[stockIndex()];
}
