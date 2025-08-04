#include "StockState.h"
#include "config.h"
#include "MDD.h"
#include "OES.h"
#include "dateTime.h"
#include "timestamp.h"
#include "WantCache.h"
#include "TickRing.h"
#include "heatZone.h"
#include <spdlog/spdlog.h>
#include <atomic>
#include <cstdint>
#include <cstring>
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
#if SH
        case NescForesight::SSE: {
            preClosePrice = static_cast<int32_t>(std::round(stat.staticSseInfo.prevClosePx * 100));
            upperLimitPrice = static_cast<int32_t>(std::round(stat.staticSseInfo.upperLimitPrice * 100));
            upperLimitPrice1000 = static_cast<uint32_t>(upperLimitPrice) * 10;
        } break;
#endif
#if SZ
        case NescForesight::SZE: {
            preClosePrice = static_cast<int32_t>(std::round(stat.staticSzInfo.prevClosePx * 100));
            upperLimitPrice = static_cast<int32_t>(std::round(stat.staticSzInfo.upperLimitPrice * 100));
            upperLimitPrice10000 = static_cast<uint64_t>(upperLimitPrice) * 100;
            offsetTransactTime = getToday() * UINT64_C(1'00'00'00'000);
        } break;
#endif
        default: {
            SPDLOG_ERROR("failed to get static: stock={} market={}", stockCode, stat.marketType);
            return;
        }
    }
#endif

    if (upperLimitPrice == 0) [[unlikely]] {
        SPDLOG_WARN("invalid static: stock={} preClosePrice={} upperLimitPrice={}", stockCode, preClosePrice, upperLimitPrice);
        alive = false;
        return;
    }

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
    std::sprintf(reqOrder->xeleReqOrderInsert.SecuritiesID, "%06d", stockCode);
    reqOrder->xeleReqOrderInsert.Direction = XELE_ORDER_BUY;
#if SZ
    reqOrder->xeleReqOrderInsert.LimitPrice = stat.staticSzInfo.upperLimitPrice;
#endif
#if SH
    reqOrder->xeleReqOrderInsert.LimitPrice = stat.staticSseInfo.upperLimitPrice;
#endif
    reqOrder->xeleReqOrderInsert.Volume = quantity;
    reqOrder->xeleReqOrderInsert.OrderType = XELE_LIMIT_PRICE_TYPE;
    reqOrder->xeleReqOrderInsert.TimeCondition = XELE_TIMEINFORCE_TYPE_GFD;
    reqOrder->xeleReqOrderInsert.SecuritiesType = '0';
    reqOrder->xeleReqOrderInsert.Operway = API_OPERWAY;
    reqOrder->xeleReqOrderInsert.ExchangeFrontID = 0;
    reqOrder->xeleReqOrderInsert.UserLocalID = stockCode;
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
#if SH
    bool limitUp = tick.buyOrderNo != 0
        && tick.sellOrderNo == 0
        && tick.quantity > 0
        && tick.price == upperLimitPrice
        && tick.timestamp >= 9'30'00'000
        && tick.timestamp < 14'57'00'000;
#endif
#if SZ
    bool limitUp = tick.buyOrderNo != 0
        && tick.sellOrderNo == 0
        && tick.quantity > upRemainQty
        && tick.price == upperLimitPrice
        && tick.timestamp >= 9'30'00'000
        && tick.timestamp < 14'57'00'000;
#endif
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

    if (tick.sellOrderNo != 0 && tick.price == upperLimitPrice) {
        if (tick.buyOrderNo == 0) {
            upRemainQty += tick.quantity;
        } else {
            upRemainQty -= tick.quantity;
        }
    }

#elif NE && SH
    bool limitUp = tick.tickMergeSse.tickType == 'A'
        && tick.tickMergeSse.tickBSFlag == '0'
        && tick.tickMergeSse.price == upperLimitPrice1000
        && tick.tickMergeSse.tickTime >= 9'30'00'00
        && tick.tickMergeSse.tickTime < 14'57'00'00;
    if (limitUp) {
        int32_t timestamp = tick.tickMergeSse.tickTime * 10;
        auto intent = wantCache->checkWantBuyAtTimestamp(timestamp);
#if ALWAYS_BUY
        intent = WantCache::WantBuy;
#endif
        if (intent == WantCache::WantBuy) [[likely]] {
            OES::sendReqOrder(*reqOrder);
        }

        stop(timestamp + static_cast<int32_t>(intent));
        return;
    }

#elif NE && SZ
    if (tick.messageType == NescForesight::MSG_TYPE_TRADE_SZ
        && tick.tradeSz.tradePrice == upperLimitPrice10000) {
        upRemainQty100 -= tick.tradeSz.tradeQty;
        if (upRemainQty100 < 0 && tick.tradeSz.execType == 0x2) {
            int32_t timestamp = static_cast<uint32_t>(tick.tradeSz.transactTime - offsetTransactTime);
            if (timestamp >= 9'30'00'000 && timestamp < 14'57'00'000) [[likely]] {
                auto intent = wantCache->checkWantBuyAtTimestamp(timestamp);
#if ALWAYS_BUY
                intent = WantCache::WantBuy;
#endif
                if (intent == WantCache::WantBuy) [[likely]] {
                    OES::sendReqOrder(*reqOrder);
                }

                stop(timestamp + static_cast<int32_t>(intent));
                return;
            }
        }
    }
    if (tick.messageType == NescForesight::MSG_TYPE_ORDER_SZ
        && tick.orderSz.side == '2'
        && tick.orderSz.orderType == '2'
        && tick.orderSz.price == upperLimitPrice10000) {
        upRemainQty100 += tick.orderSz.qty;
    }
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
    if (rspOrder.rspType == OES::RspOrder::XeleRspOrderInsert) {
        if (rspOrder.errorID != 0) {
            SPDLOG_ERROR("(XeleRspOrderInsert) stock={} errorId={}", stockCode, rspOrder.errorID);
        }
        SPDLOG_INFO("(XeleRspOrderInsert) stock={} securityId={} orderSysId={} orderPrice={} orderQuantity={} exchangeFrontId={} orderDirection={}", stockCode, rspOrder.xeleRspOrderInsert->SecuritiesID, rspOrder.xeleRspOrderInsert->OrderSysID, rspOrder.xeleRspOrderInsert->LimitPrice, rspOrder.xeleRspOrderInsert->Volume, rspOrder.xeleRspOrderInsert->Direction == '1' ? "BUY" : rspOrder.xeleRspOrderInsert->Direction == '2' ? "SELL" : "WARM", rspOrder.xeleRspOrderAction->ExchangeFrontID, rspOrder.xeleRspOrderAction->ErrorId);

    } else if (rspOrder.rspType == OES::RspOrder::XeleRspOrderAction) {
        if (rspOrder.errorID != 0) {
            SPDLOG_ERROR("(XeleRspOrderAction) stock={} errorId={}", stockCode, rspOrder.errorID);
        }
        SPDLOG_INFO("(XeleRspOrderAction) stock={} orderSysId={} orderExchangeId={} exchangeFrontId={} errorId={}", stockCode, rspOrder.xeleRspOrderAction->OrderSysID, rspOrder.xeleRspOrderAction->OrderExchangeID,  rspOrder.xeleRspOrderAction->ExchangeFrontID, rspOrder.xeleRspOrderAction->ErrorId);

    } else if (rspOrder.rspType == OES::RspOrder::XeleRtnOrder) {
        if (rspOrder.errorID != 0) {
            SPDLOG_ERROR("(XeleRtnOrder) stock={} errorId={}", stockCode, rspOrder.errorID);
        }
        SPDLOG_INFO("(XeleRtnOrder) stock={} orderSysId={} orderExchangeId={} orderPrice={} orderQuantity={} transactTime={} tradeQuantity={} leavesQuantity={} orderStatus={} exchangeFrontId={}", stockCode, rspOrder.xeleRtnOrder->OrderSysID, rspOrder.xeleRtnOrder->OrderExchangeID, rspOrder.xeleRtnOrder->LimitPrice, rspOrder.xeleRtnOrder->Volume, rspOrder.xeleRtnOrder->Direction == '1' ? "BUY" : "SELL", rspOrder.xeleRtnOrder->TransactTime, rspOrder.xeleRtnOrder->TradeVolume, rspOrder.xeleRtnOrder->LeavesVolume, rspOrder.xeleRtnOrder->OrderStatus, rspOrder.xeleRtnOrder->ExchangeFrontID);

    } else if (rspOrder.rspType == OES::RspOrder::XeleRtnTrade) {
        if (rspOrder.errorID != 0) {
            SPDLOG_ERROR("(XeleRtnTrade) stock={} errorId={}", stockCode, rspOrder.errorID);
        }
        SPDLOG_INFO("(XeleRtnTrade) stock={} orderSysId={} orderExchangeId={} orderPrice={} orderQuantity={} transactTime={} origTime={} tradePrice={} tradeQuantity={} leavesQuantity={} orderStatus={} exchangeFrontId={}", stockCode, rspOrder.xeleRtnTrade->OrderSysID, rspOrder.xeleRtnTrade->OrderExchangeID, rspOrder.xeleRtnTrade->LimitPrice, rspOrder.xeleRtnTrade->Volume, rspOrder.xeleRtnTrade->Direction == '1' ? "BUY" : "SELL", rspOrder.xeleRtnTrade->TransactTime, rspOrder.xeleRtnTrade->OrigTime, rspOrder.xeleRtnTrade->TradePrice, rspOrder.xeleRtnTrade->TradeVolume, rspOrder.xeleRtnTrade->LeavesVolume, rspOrder.xeleRtnTrade->OrderStatus, rspOrder.xeleRtnTrade->ExchangeFrontID);
    }
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
