#pragma once


#include <absl/types/internal/variant.h>
#include <cstdint>
#include "config.h"
#if XC || NE
#include <xele/XeleSecuritiesUserApiStruct.h>
#elif OST
#include <UTApi.h>
#endif


namespace OES
{

#if REPLAY

struct ReqOrder
{
    int32_t stockCode;
    int32_t price;
    int32_t quantity;
    char limitType;
    char direction;
};

struct ReqCancel
{
    int32_t stockCode;
    int32_t orderSysId;
};

struct RspOrder
{
    int32_t errorId;
    int32_t messageType;
    int32_t stockCode;
    int32_t orderStatus;
    int32_t orderSysId;
    int32_t orderPrice;
    int32_t tradePrice;
    char orderDirection;
    int32_t orderQuantity;
    int32_t tradeQuantity;
    int32_t totalTradedQuantity;
    int32_t remainQuantity;
    int32_t cancelledQuantity;
};

#elif XC || NE

///报单请求
struct ReqOrder
{
    CXeleReqOrderInsertField xeleReqOrderInsert;
    uint32_t userLocalID;
};

///批量报单请求
struct ReqOrderBatch
{
    CXeleReqBatchOrderInsertField xeleReqBatchOrderInsert;
    uint32_t userLocalID;
};

///撤单请求
struct ReqCancel
{
    CXeleReqOrderActionField xeleReqOrderAction;
};

struct RspOrder
{
    enum RspType
    {
        XeleRspOrderInsert,
        XeleRspOrderAction,
        XeleRtnOrder,
        XeleRtnTrade,
    };

    RspType rspType;
    TXeleRequestIDType requestID;
    TXeleErrorIdType errorID;
    uint32_t userLocalID;

    union
    {
        ///报单应答
        CXeleRspOrderInsertField *xeleRspOrderInsert;
        ///撤单应答
        CXeleRspOrderActionField *xeleRspOrderAction;
        ///报单回报
        CXeleRtnOrderField *xeleRtnOrder;
        ///成交回报
        CXeleRtnTradeField *xeleRtnTrade;
    };
};

const char *strErrorId(TXeleErrorIdType errorId);

#elif OST

struct ReqOrder
{
    CUTInputOrderField inputOrder;
    uint32_t userLocalID;
};

struct ReqCancel
{
    CUTInputOrderActionField inputOrderAction;
};

struct RspOrder
{
    enum RspType
    {
        OstRspOrderInsert,
        OstRspOrderAction,
        OstRtnOrder,
        OstErrRtnOrderAction,
        OstRtnTrade,
    };

    RspType rspType;
    int32_t requestID;
    int32_t errorID;
    const char *errorMsg;
    uint32_t userLocalID;

    union
    {
        ///报单应答
        CUTInputOrderField *ostRspOrderInsert;
        ///撤单应答
        CUTInputOrderActionField *ostRspOrderAction;
        ///报单回报
        CUTOrderField *ostRtnOrder;
        ///报单错误回报
        CUTOrderActionField *ostErrRtnOrderAction;
        ///成交回报
        CUTTradeField *ostRtnTrade;
    };
};

CUTDepthMarketDataField *getDepthMarketData(size_t &size);
void getInvestorID(TUTInvestorIDType investorID);
void getFrontID(TUTFrontIDType &frontID, TUTSessionIDType &sessionID);

#endif

void start(const char *config);
bool isStarted();
void stop();
void sendReqOrder(ReqOrder &reqOrder);
void sendReqOrderBatch(ReqOrderBatch &reqOrderBatch);
void sendReqCancel(ReqCancel &reqCancel);

}
