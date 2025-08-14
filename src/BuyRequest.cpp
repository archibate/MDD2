#include "BuyRequest.h"
#include "config.h"
#include "OES.h"
#include <spdlog/spdlog.h>

void makeBuyRequest(BuyRequest &buyRequest, int32_t stockCode, int32_t upperLimitPrice, int32_t quantity)
{
    SPDLOG_TRACE("prepared buy request: stock={:06d} upperLimitPrice={} reportQuantity={}",
                 stockCode, upperLimitPrice, quantity);

#if SPLIT_ORDER
    OES::ReqOrderBatch *reqOrderBatch = &buyRequest;
    std::memset(reqOrderBatch, 0, sizeof(OES::ReqOrderBatch));
#else
    OES::ReqOrder *reqOrder = &buyRequest;
    std::memset(reqOrder, 0, sizeof(OES::ReqOrder));
#endif

#if REPLAY

    reqOrder->stockCode = stockCode;
    reqOrder->price = upperLimitPrice;
    reqOrder->quantity = quantity;
    reqOrder->limitType = 'U';
    reqOrder->direction = 'B';

#elif XC || NE

#if !SPLIT_ORDER
     reqOrder->userLocalID = stockCode;
     auto *xeleInsert = &reqOrder->xeleReqOrderInsert;
#else
    reqOrderBatch->userLocalID = stockCode;
    reqOrderBatch->xeleReqBatchOrderInsert.BatchOrderQty = kExchangeFronts.size();
    for (size_t i = 0; i < kExchangeFronts.size(); ++i) {
        auto *xeleInsert = reqOrderBatch->xeleReqBatchOrderInsert.ReqOrderInsertField + i;
#endif
        std::sprintf(xeleInsert->SecuritiesID, "%06d", stockCode);
        xeleInsert->Direction = XELE_ORDER_BUY;
        xeleInsert->LimitPrice = upperLimitPrice * 0.01;
        xeleInsert->Volume = quantity;
        xeleInsert->OrderType = XELE_LIMIT_PRICE_TYPE;
        xeleInsert->TimeCondition = XELE_TIMEINFORCE_TYPE_GFD;
        xeleInsert->SecuritiesType = '0';
        xeleInsert->Operway = API_OPERWAY;
#if SPLIT_ORDER
        xeleInsert->ExchangeFrontID = kExchangeFronts[i];
#else
        xeleInsert->ExchangeFrontID = 0;
#endif
#if SPLIT_ORDER
    }
#endif

#elif OST

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
    reqOrder->inputOrder.LimitPrice = upperLimitPrice * 0.01;
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
}

#if SELL_GC001
void makeGCSellRequest(OES::ReqOrder &reqOrder, int32_t stockCode, int32_t price, int32_t quantity)
{
#error not implemented
}
#endif
