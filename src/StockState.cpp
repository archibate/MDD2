#include "StockState.h"
#include "config.h"
#include "MDD.h"
#include "OES.h"
#include "dateTime.h"
#include "timestamp.h"
#include "WantCache.h"
#include "generatedReflect.h"
#include "TickRing.h"
#include "heatZone.h"
#include <spdlog/spdlog.h>
#include <atomic>
#include <cstdint>
#include <cstring>
#include <cstdio>


void StockState::start()
{
    stockCode = MDD::g_stockCodes[stockIndex()];
    wantCache = &MDD::g_wantCaches[stockIndex()];
}

void StockState::setChannelId(int32_t channelId)
{
    tickRing = &MDD::g_tickRings[channelId];
}

void StockState::onStatic(MDS::Stat const &stat)
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

#elif OST
    int32_t prevClose = static_cast<int32_t>(std::round(stat.depthMarketData.PreClosePrice * 100));
    int32_t upperLimitPrice = static_cast<int32_t>(std::round(stat.depthMarketData.UpperLimitPrice * 100));
    int32_t lowerLimitPrice = static_cast<int32_t>(std::round(stat.depthMarketData.LowerLimitPrice * 100));

#if SH
    upperLimitPrice1000 = static_cast<uint64_t>(upperLimitPrice) * 10;
#endif
#if SZ
    upperLimitPrice10000 = static_cast<uint64_t>(upperLimitPrice) * 100;
    offsetTransactTime = stat.depthMarketData.TradingDay * UINT64_C(1'00'00'00'000);
#endif
#endif

    if (upperLimitPrice == 0) [[unlikely]] {
        SPDLOG_WARN("invalid static price: stock={} preClosePrice={} upperLimitPrice={}",
                    stockCode, preClosePrice, upperLimitPrice);
        return;
    }

    int32_t quantity = 100;
    SPDLOG_TRACE("initial static: stock={} preClose={} upperLimit={} reportQuantity={}",
                 stockCode, preClosePrice, upperLimitPrice, quantity);

    reqOrder = std::make_unique<OES::ReqOrder>();
    std::memset(reqOrder.get(), 0, sizeof(OES::ReqOrder));
    reqCancel = std::make_unique<OES::ReqCancel>();
    std::memset(reqCancel.get(), 0, sizeof(OES::ReqCancel));

#if REPLAY

    reqOrder->stockCode = stockCode;
    reqOrder->price = upperLimitPrice;
    reqOrder->quantity = quantity;
    reqOrder->limitType = 'U';
    reqOrder->direction = 'B';

#elif XC || NE
    reqCancel->xeleReqOrderAction.OwnerType = XELE_OWNER_PERSONAL_TYPE;
    reqCancel->xeleReqOrderAction.Operway = API_OPERWAY;
    reqCancel->xeleReqOrderAction.UserLocalID = stockCode;
    // reqCancel->xeleReqOrderAction.OrigSysID = ???;

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

#elif OST

    OES::getFrontID(reqCancel->inputOrderAction.FrontID, reqCancel->inputOrderAction.SessionID);
    reqCancel->inputOrderAction.ActionFlag = UT_AF_Delete;
    // reqCancel->inputOrderAction.OrderRef = ???;
    reqCancel->inputOrderAction.OrderActionRef = -1;

    reqOrder->userLocalID = stockCode;
    OES::getInvestorID(reqOrder->inputOrder.InvestorID);
#if SH
    reqOrder->inputOrder.ExchangeID = UT_EXG_SSE;
#endif
#if SZ
    reqOrder->inputOrder.ExchangeID = UT_EXG_SZSE;
#endif
    std::sprintf(reqOrder->inputOrder.InstrumentID, "%06d", stockCode);
    reqOrder->inputOrder.OrderRef = -1;
#if BEST_ORDER
    reqOrder->inputOrder.OrderPriceType = UT_OPT_BestPriceThisSide;
#else
    reqOrder->inputOrder.OrderPriceType = UT_OPT_LimitPrice;
#endif
    reqOrder->inputOrder.HedgeFlag = UT_HF_Speculation;
    reqOrder->inputOrder.Direction = UT_D_Buy;
    reqOrder->inputOrder.OffsetFlag = UT_OF_Open;
    reqOrder->inputOrder.LimitPrice = stat.depthMarketData.UpperLimitPrice;
    reqOrder->inputOrder.VolumeTotalOriginal = quantity;
    reqOrder->inputOrder.TimeCondition = UT_TC_GFD;
    reqOrder->inputOrder.VolumeCondition = UT_VC_AV;
    reqOrder->inputOrder.MinVolume = 0;
    reqOrder->inputOrder.ContingentCondition = UT_CC_Immediately;
    reqOrder->inputOrder.StopPrice = 0;
    reqOrder->inputOrder.IsAutoSuspend = 0;
    //股票,基金,债券买:HedgeFlag = UT_HF_Speculation, Direction = UT_D_Buy, OffsetFlag = UT_OF_Open
    //股票,基金,债券卖:HedgeFlag = UT_HF_Speculation, Direction = UT_D_Sell, OffsetFlag = UT_OF_Close
    //债券逆回购:HedgeFlag = UT_HF_Speculation, Direction = UT_D_Sell, OffsetFlag = UT_OF_Open
    //ETF申购:HedgeFlag = UT_HF_Redemption, Direction = UT_D_Buy, OffsetFlag = UT_OF_Open
    //ETF赎回:HedgeFlag = UT_HF_Redemption, Direction = UT_D_Sell, OffsetFlag = UT_OF_Close

#endif

    alive = true;
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
#elif OST && SH
        endSign.tick.m_tick_type = 0;
        std::sprintf(endSign.tick.m_symbol_id, "%06d", stockCode);
        endSign.tick.m_tick_time = timestamp;
#elif OST && SZ
        endSign.head.m_message_type = 0;
        std::sprintf(endSign.head.m_symbol, "%06d", stockCode);
        endSign.head.m_quote_update_time = timestamp;
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
            OES::sendReqOrder(*reqOrder);
        }

        stop(tick.timestamp + static_cast<int32_t>(intent));
        return;
    }

#if SZ
    if (tick.sellOrderNo != 0 && tick.price == upperLimitPrice) {
        if (tick.buyOrderNo == 0) {
            upRemainQty += tick.quantity;
        } else {
            upRemainQty -= tick.quantity;
        }
    }
#endif

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
    if (tick.messageType == NescForesight::MSG_TYPE_TRADE_SZ) {
        if (tick.tradeSz.tradePrice == upperLimitPrice10000) {
            upRemainQty100 -= tick.tradeSz.tradeQty;
            if (upRemainQty100 < 0 && tick.tradeSz.execType == 0x2) {
                int32_t timestamp = static_cast<uint32_t>(
                    tick.tradeSz.transactTime - offsetTransactTime);
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
    } else if (tick.messageType == NescForesight::MSG_TYPE_ORDER_SZ) [[unlikely]] {
        if (tick.orderSz.side == '2'
            && tick.orderSz.orderType == '2'
            && tick.orderSz.price == upperLimitPrice10000) {
            upRemainQty100 += tick.orderSz.qty;
        }
    }

#elif OST && SH
    bool limitUp = tick.tick.m_tick_type == 'A'
        && tick.tick.m_side_flag == '0'
        && tick.tick.m_order_price == upperLimitPrice1000
        && tick.tick.m_tick_time >= 9'30'00'00
        && tick.tick.m_tick_time < 14'57'00'00;
    if (limitUp) {
        int32_t timestamp = tick.tick.m_tick_time * 10;
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

#elif OST && SZ
    if (tick.head.m_message_type == sze_msg_type_trade) {
        if (tick.exe.m_exe_px == upperLimitPrice10000) {
            upRemainQty100 -= tick.exe.m_exe_qty;
            if (upRemainQty100 < 0 && tick.exe.m_exe_type == 0x2) {
                int32_t timestamp = static_cast<uint32_t>(
                    tick.head.m_quote_update_time - offsetTransactTime);
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
    } else if (tick.head.m_message_type == sze_msg_type_order) [[likely]] {
        if (tick.order.m_side == '2'
            && tick.order.m_order_type == '2'
            && tick.order.m_px == upperLimitPrice10000) {
            upRemainQty100 += tick.order.m_qty;
        }
    }
#endif

    tickRing->pushTick(tick);
}

HEAT_ZONE_RSPORDER void StockState::onRspOrder(OES::RspOrder &rspOrder)
{
#if REPLAY
    if (rspOrder.errorId != 0) {
        SPDLOG_ERROR("response order error: errorId={}", rspOrder.errorId);
    }
    SPDLOG_DEBUG("response order: messageType={} stock={} orderStatus={} orderSysId={} orderPrice={} orderQuantity={} orderDirection={}", rspOrder.messageType, rspOrder.stockCode, rspOrder.orderStatus, rspOrder.orderSysId, rspOrder.orderPrice, rspOrder.orderQuantity, rspOrder.orderDirection);

#elif XC || NE
    if (rspOrder.errorID != 0) {
        SPDLOG_ERROR("xele returned error: stock={} userLocalID={} errorID={} errorMsg=[{}]", stockCode, rspOrder.userLocalID, rspOrder.errorID, OES::strErrorId(rspOrder.errorID));
    }

    if (rspOrder.rspType == OES::RspOrder::XeleRspOrderInsert) {
        if (rspOrder.errorID != 0) {
            SPDLOG_ERROR("(XeleRspOrderInsert) stock={} errorId={} errorMsg=[{}]", stockCode, rspOrder.errorID, OES::strErrorId(rspOrder.errorID));
        }
        // SPDLOG_INFO("(XeleRspOrderInsert) stock={} securityId={} orderSysId={} orderPrice={} orderQuantity={} exchangeFrontId={} orderDirection={} errorId={}", stockCode, rspOrder.xeleRspOrderInsert->SecuritiesID, rspOrder.xeleRspOrderInsert->OrderSysID, rspOrder.xeleRspOrderInsert->LimitPrice, rspOrder.xeleRspOrderInsert->Volume, rspOrder.xeleRspOrderAction->ExchangeFrontID, rspOrder.xeleRspOrderInsert->Direction == XELE_ORDER_BUY ? "BUY" : rspOrder.xeleRspOrderInsert->Direction == '2' ? "SELL" : "WARM", rspOrder.xeleRspOrderAction->ErrorId);

    } else if (rspOrder.rspType == OES::RspOrder::XeleRspOrderAction) {
        if (rspOrder.errorID != 0) {
            SPDLOG_ERROR("(XeleRspOrderAction) stock={} errorId={} errorMsg=[{}]", stockCode, rspOrder.errorID, OES::strErrorId(rspOrder.errorID));
        }
        // SPDLOG_INFO("(XeleRspOrderAction) stock={} orderSysId={} orderExchangeId={} exchangeFrontId={} errorId={}", stockCode, rspOrder.xeleRspOrderAction->OrderSysID, rspOrder.xeleRspOrderAction->OrderExchangeID, rspOrder.xeleRspOrderAction->ExchangeFrontID, rspOrder.xeleRspOrderAction->ErrorId);

    } else if (rspOrder.rspType == OES::RspOrder::XeleRtnOrder) {
        if (rspOrder.errorID != 0) {
            SPDLOG_ERROR("(XeleRtnOrder) stock={} errorId={} errorMsg=[{}]", stockCode, rspOrder.errorID, OES::strErrorId(rspOrder.errorID));
        }
        // SPDLOG_INFO("(XeleRtnOrder) stock={} orderSysId={} orderExchangeId={} orderPrice={} orderQuantity={} orderDirection={} transactTime={} tradeQuantity={} leavesQuantity={} orderStatus={} exchangeFrontId={}", stockCode, rspOrder.xeleRtnOrder->OrderSysID, rspOrder.xeleRtnOrder->OrderExchangeID, rspOrder.xeleRtnOrder->LimitPrice, rspOrder.xeleRtnOrder->Volume, rspOrder.xeleRtnOrder->Direction == XELE_ORDER_BUY ? "BUY" : "SELL", rspOrder.xeleRtnOrder->TransactTime, rspOrder.xeleRtnOrder->TradeVolume, rspOrder.xeleRtnOrder->LeavesVolume, rspOrder.xeleRtnOrder->OrderStatus, rspOrder.xeleRtnOrder->ExchangeFrontID);

    } else if (rspOrder.rspType == OES::RspOrder::XeleRtnTrade) {
        if (rspOrder.errorID != 0) {
            SPDLOG_ERROR("(XeleRtnTrade) stock={} errorId={} errorMsg=[{}]", stockCode, rspOrder.errorID, OES::strErrorId(rspOrder.errorID));
        }
        // SPDLOG_INFO("(XeleRtnTrade) stock={} orderSysId={} orderExchangeId={} orderPrice={} orderQuantity={} transactTime={} origTime={} tradePrice={} tradeQuantity={} leavesQuantity={} orderStatus={} exchangeFrontId={}", stockCode, rspOrder.xeleRtnTrade->OrderSysID, rspOrder.xeleRtnTrade->OrderExchangeID, rspOrder.xeleRtnTrade->LimitPrice, rspOrder.xeleRtnTrade->Volume, rspOrder.xeleRtnTrade->Direction == XELE_ORDER_BUY ? "BUY" : "SELL", rspOrder.xeleRtnTrade->TransactTime, rspOrder.xeleRtnTrade->OrigTime, rspOrder.xeleRtnTrade->TradePrice, rspOrder.xeleRtnTrade->TradeVolume, rspOrder.xeleRtnTrade->LeavesVolume, rspOrder.xeleRtnTrade->OrderStatus, rspOrder.xeleRtnTrade->ExchangeFrontID);
    }

#elif OST
    if (rspOrder.errorID != 0) {
        SPDLOG_ERROR("ost returned error: stock={} userLocalID={} errorID={} errorMsg=[{}]", stockCode, rspOrder.userLocalID, rspOrder.errorID, rspOrder.errorMsg);
    }

    if (rspOrder.rspType == OES::RspOrder::OstRspOrderInsert) {
        SPDLOG_INFO("ost responsed rspType=OstRspOrderInsert stock={} userLocalID={} errorID={} {}", stockCode, rspOrder.userLocalID, rspOrder.errorID, refl::to_string(*rspOrder.ostRspOrderInsert));

        // if (rspOrder.errorID != 0) {
        //     SPDLOG_ERROR("(OstRspOrderInsert error) stock={} errorId={} errorMsg=[{}]", stockCode, rspOrder.errorID, rspOrder.errorMsg);
        // }
        // SPDLOG_INFO("(OstRspOrderInsert) stock={} securityId={} orderRef={} orderPrice={} orderQuantity={} orderDirection={} frontId={} investorId={} errorId={}", stockCode, rspOrder.ostRspOrderInsert->InstrumentID, rspOrder.ostRspOrderInsert->OrderRef, rspOrder.ostRspOrderInsert->LimitPrice, rspOrder.ostRspOrderInsert->VolumeTotalOriginal, rspOrder.ostRspOrderInsert->Direction == UT_D_Buy ? "BUY" : rspOrder.ostRspOrderInsert->Direction == UT_D_Sell ? "SELL" : "INVALID", rspOrder.ostRspOrderAction->FrontID, rspOrder.ostRspOrderInsert->InvestorID, rspOrder.errorID);
    
    } else if (rspOrder.rspType == OES::RspOrder::OstRspOrderAction) {
        SPDLOG_INFO("ost responsed rspType=OstRspOrderAction stock={} userLocalID={} errorID={} {}", stockCode, rspOrder.userLocalID, rspOrder.errorID, refl::to_string(*rspOrder.ostRspOrderAction));

        // if (rspOrder.errorID != 0) {
        //     SPDLOG_ERROR("(OstRspOrderAction) stock={} errorId={} errorMsg=[{}]", stockCode, rspOrder.errorID, rspOrder.errorMsg);
        // }
        // SPDLOG_INFO("(OstRspOrderAction) stock={} orderRef={} orderActionRef={} orderLocalID={} frontId={} errorId={}", stockCode, rspOrder.ostRspOrderAction->OrderRef, rspOrder.ostRspOrderAction->OrderActionRef, rspOrder.ostRspOrderAction->OrderLocalID, rspOrder.ostRspOrderAction->FrontID, rspOrder.errorID);
    
    } else if (rspOrder.rspType == OES::RspOrder::OstErrRtnOrderAction) {
        SPDLOG_INFO("ost responsed rspType=OstErrRtnOrderAction stock={} userLocalID={} errorID={} {}", stockCode, rspOrder.userLocalID, rspOrder.errorID, refl::to_string(*rspOrder.ostErrRtnOrderAction));

        // if (rspOrder.errorID != 0) {
        //     SPDLOG_ERROR("(OstErrRtnOrderAction) stock={} errorId={} errorMsg=[{}]", stockCode, rspOrder.errorID, rspOrder.errorMsg);
        // }
        // SPDLOG_INFO("(OstErrRtnOrderAction) stock={} orderRef={} orderActionRef={} orderActionStatus={} orderSysId={} orderLocalId={} frontId={} traderId={} installId={} traderInstallId={} errorId={}", stockCode, rspOrder.ostErrRtnOrderAction->OrderRef, rspOrder.ostErrRtnOrderAction->OrderActionRef, rspOrder.ostErrRtnOrderAction->OrderActionStatus, rspOrder.ostErrRtnOrderAction->OrderSysID, rspOrder.ostErrRtnOrderAction->OrderLocalID, rspOrder.ostErrRtnOrderAction->FrontID, rspOrder.ostErrRtnOrderAction->TraderID, rspOrder.ostErrRtnOrderAction->InstallID, rspOrder.ostErrRtnOrderAction->TraderInstallID, rspOrder.errorID);

    } else if (rspOrder.rspType == OES::RspOrder::OstRtnOrder) {
        SPDLOG_INFO("ost responsed rspType=OstRtnOrder stock={} userLocalID={} errorID={} {}", stockCode, rspOrder.userLocalID, rspOrder.errorID, refl::to_string(*rspOrder.ostRtnOrder));

        // if (rspOrder.errorID != 0) {
        //     SPDLOG_ERROR("(OstRtnOrder) stock={} errorId={} errorMsg=[{}]", stockCode, rspOrder.errorID, rspOrder.errorMsg);
        // }
        // SPDLOG_INFO("(OstRtnOrder) stock={} orderRef={} orderSysId={} orderLocalId={} traderId={} orderPrice={} orderQuantity={} orderDirection={} insertTime={} cancelTime={} tradeQuantity={} leavesQuantity={} orderStatus={} frontId={} installId={} traderInstallId={}", stockCode, rspOrder.ostRtnOrder->OrderRef, rspOrder.ostRtnOrder->OrderSysID, rspOrder.ostRtnOrder->OrderLocalID, rspOrder.ostRtnOrder->TraderID, rspOrder.ostRtnOrder->LimitPrice, rspOrder.ostRtnOrder->VolumeTotalOriginal, rspOrder.ostRtnOrder->Direction == UT_D_Buy ? "BUY" : "SELL", rspOrder.ostRtnOrder->InsertTime, rspOrder.ostRtnOrder->CancelTime, rspOrder.ostRtnOrder->VolumeTraded, rspOrder.ostRtnOrder->VolumeTotal, rspOrder.ostRtnOrder->OrderStatus, rspOrder.ostRtnOrder->FrontID, rspOrder.ostRtnOrder->InstallID, rspOrder.ostRtnOrder->TraderInstallID);
    
    } else if (rspOrder.rspType == OES::RspOrder::OstRtnTrade) {
        SPDLOG_INFO("ost responsed rspType=OstRtnTrade stock={} userLocalID={} errorID={} {}", stockCode, rspOrder.userLocalID, rspOrder.errorID, refl::to_string(*rspOrder.ostRtnTrade));

        // if (rspOrder.errorID != 0) {
        //     SPDLOG_ERROR("(OstRtnTrade) stock={} errorId={} errorMsg=[{}]", stockCode, rspOrder.errorID, rspOrder.errorMsg);
        // }
        // SPDLOG_INFO("(OstRtnTrade) stock={} orderRef={} orderSysId={} orderLocalId={} traderId={} tradeId={} tradePrice={} tradeQuantity={} orderDirection={} tradeTime={} frontId={} installId={}", stockCode, rspOrder.ostRtnTrade->OrderRef, rspOrder.ostRtnTrade->OrderSysID, rspOrder.ostRtnTrade->TraderID, rspOrder.ostRtnTrade->OrderLocalID, rspOrder.ostRtnTrade->TradeID, rspOrder.ostRtnTrade->Price, rspOrder.ostRtnTrade->Volume, rspOrder.ostRtnTrade->Direction == UT_D_Buy ? "BUY" : "SELL", rspOrder.ostRtnTrade->TradeTime, rspOrder.ostRtnTrade->FrontID, rspOrder.ostRtnTrade->InstallID);
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
