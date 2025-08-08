#pragma once


#include <absl/types/internal/variant.h>
#include <cstdint>
#include "config.h"
#if XC || NE
#include <xele/XeleSecuritiesUserApiStruct.h>
#elif OST
#include "UTApi.h"
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
};

///批量报单请求
struct ReqBatchOrder
{
    CXeleReqBatchOrderInsertField xeleReqBatchOrderInsert;
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
    TXeleOrderIDType userLocalID;
    TXeleRequestIDType requestID;
    TXeleErrorIdType errorID;

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

#else OST

struct ReqOrder
{
    CUTInputOrderField inputOrder;
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
        OstRtnTrade,
    };

    RspType rspType;
    int32_t requestID;
    int32_t errorID;

    union
    {
        ///报单应答
        CUTInputOrderField *ostRspOrderInsert;
        ///撤单应答
        CUTInputOrderActionField *ostRspOrderAction;
        ///报单回报
        CUTOrderField *ostRtnOrder;
        ///成交回报
        CUTTradeField *ostRtnTrade;
    };
};

CUTDepthMarketDataField *getDepthMarketData(int32_t stock);

#endif

void start(const char *config);
bool isStarted();
void stop();
void sendReqOrder(ReqOrder &reqOrder);
void sendReqCancel(ReqCancel &reqCancel);

}
