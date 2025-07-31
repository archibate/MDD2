#include "config.h"
#if XC || NE
#include "OES.h"
#include "MDD.h"
#include "heatZone.h"
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#include <xele/XeleSecuritiesTraderApi.h>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include "LOG.h"

namespace
{

// void fmtSecurityId(TXeleSecuritiesIDType securityID, int32_t stock)
// {
//     securityID[5] = '0' + stock % 10;
//     stock /= 10;
//     securityID[4] = '0' + stock % 10;
//     stock /= 10;
//     securityID[3] = '0' + stock % 10;
//     stock /= 10;
//     securityID[2] = '0' + stock % 10;
//     stock /= 10;
//     securityID[1] = '0' + stock % 10;
//     stock /= 10;
//     securityID[0] = '0' + stock % 10;
// }

class XeleTdSpi final : public XeleSecuritiesTraderSpi
{
public:
    XeleTdSpi() = default;
    ~XeleTdSpi() override = default;

    //表明具有资金调拨的相关权限
    bool canFundTransfer{false};
    //表明具有柜台查询权限
    bool canQuery{false};
    //表明具有柜台报、撤单权限
    bool canOrder{false};

private:
    ///艾科管理中心登录应答,当只有登录管理中心的需求时，收到该回报即可进行管理中心相关接口操作
    void onRspLoginManager(CXeleRspUserLoginManagerField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) override;

    ///艾科柜台登录应答,当需要管理中心接口可用，但是只需求艾科柜台查询接口可用时，收到该回报即可进行操作
    void onRspLogin(CXeleRspUserLoginField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) override;

    ///添加交易链路应答,当收到该回报时，标记艾科柜台报、撤单接口可用
    void onRspInitTrader(CXeleRspInitTraderField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) override;

    ///艾科管理中心登出应答
    void onRspLogoutManager(CXeleRspUserLogoutManagerField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) override;

    ///艾科柜台登出应答
    void onRspLogout(CXeleRspUserLogoutField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) override;

    ///当客户端与管理中心服务端查询通信连接断开时，该方法被调用。
    ///该回报触发条件为客户主动发起登出请求，或者链路发生了断连。
    ///出现以上情况时，若不是主动发起登出请求，建议先发起登出请求，再重新登录，即可再次使用相关接口
    void onFrontManagerQueryDisconnected(int nReason) override;

    ///当客户端与服务端查询通信连接断开时，该方法被调用。
    ///该回报触发条件为客户主动发起登出请求，或者链路发生了断连。
    ///出现以上情况时，若不是主动发起登出请求，建议先发起登出请求，再重新登录，即可再次使用相关接口
    void onFrontQueryDisconnected(int nReason) override;

    ///当客户端与服务端交易通信连接断开时，该方法被调用。
    ///当收到该回调，表示当前连接失去了报、撤单功能.
    ///如需恢复，参考OnFrontQueryDisconnected处理方法
    void onFrontTradeDisconnected(int nReason) override;

    ///api内部消息打印回调
    void onApiMsg(int ret, const char *strFormat, ...) override;

    ///////////////////////////////
    /////// 证券、期权公用 ////////
    //////////////////////////////

    ///报单应答
    void onRspInsertOrder(CXeleRspOrderInsertField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) override;

    ///报单错误回报
    void onErrRtnInsertOrder(CXeleRspOrderInsertField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) override;

    ///撤单应答
    void onRspCancelOrder(CXeleRspOrderActionField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) override;

    ///撤单错误回报
    void onErrRtnCancelOrder(CXeleRspOrderActionField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) override;

    ///报单回报
    void onRtnOrder(CXeleRtnOrderField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) override;

    ///成交回报
    void onRtnTrade(CXeleRtnTradeField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) override;

    ///报单查询应答
    void onRspQryOrder(CXeleRspQryOrderField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) override;

    ///成交查询应答
    void onRspQryTrade(CXeleRspQryTradeField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) override;

    ///////////////////////////////
    /////// 证券相关 //////////////
    //////////////////////////////

    ///证券资金查询应答
    void onRspQryFund(CXeleRspQryStockClientAccountField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) override;

    ///证券持仓查询应答
    void onRspQryPosition(CXeleRspQryStockPositionField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) override;
};

XeleTdSpi *g_userSpi;
XeleSecuritiesTraderApi *g_tradeApi;
int32_t g_requestID;
TXeleOrderIDType g_maxUserLocalID;

std::string g_username;
std::string g_password;
std::string g_xeleConfigFile;
int32_t g_xeleTradeNode;


/// 艾科管理中心登录应答,当只有登录管理中心的需求时，收到该回报即可进行管理中心相关接口操作
void XeleTdSpi::onRspLoginManager(CXeleRspUserLoginManagerField* pRspField, CXeleRspInfo* pRspInfo, int nRequestID, bool bIsLast)
{
    if (pRspInfo->ErrorID == 0)
    {
        canFundTransfer = true;
        LOGf(INFO, "now can use manager interface\n");
    }
    else
    {
        LOGf(ERROR, "login manager error,ErrID[%d],ErrMsg[%s]\n", pRspInfo->ErrorID, pRspInfo->ErrorMsg);
    }
};

/// 艾科柜台登录应答,当需要管理中心接口可用，但是只需求艾科柜台查询接口可用时，收到该回报即可进行操作
void XeleTdSpi::onRspLogin(CXeleRspUserLoginField* pRspField, CXeleRspInfo* pRspInfo, int nRequestID, bool bIsLast)
{
    if (pRspInfo->ErrorID == 0)
    {
        canQuery = true;
        g_maxUserLocalID = pRspField->MaxUserLocalID;
        LOGf(INFO, "now can use query interface, maxUserLocalID:%d\n", pRspField->MaxUserLocalID);
    }
    else
    {
        LOGf(ERROR, "login counter error,ErrID[%d],ErrMsg[%s]\n", pRspInfo->ErrorID, pRspInfo->ErrorMsg);
    }
};

/// 添加交易链路应答,当收到该回报时，标记艾科柜台报、撤单接口可用
void XeleTdSpi::onRspInitTrader(CXeleRspInitTraderField* pRspField, CXeleRspInfo* pRspInfo, int nRequestID, bool bIsLast)
{
    if (pRspInfo->ErrorID == 0)
    {
        canOrder = true;
        LOGf(INFO, "now can use order interface\n");
    }
    else
    {
        LOGf(ERROR, "create order link error,ErrID[%d],ErrMsg[%s]\n", pRspInfo->ErrorID, pRspInfo->ErrorMsg);
    }
};

/// 艾科管理中心登出应答
void XeleTdSpi::onRspLogoutManager(CXeleRspUserLogoutManagerField* pRspField, CXeleRspInfo* pRspInfo, int nRequestID, bool bIsLast)
{
    LOGf(INFO, "  Test LogoutManager pass\n");
    LOGf(INFO, "=======================");
    if (pRspInfo->ErrorID == 0)
    {
        canFundTransfer = false;
        LOGf(INFO, "now can't use manager interface\n");
    }
    else
    {
        LOGf(ERROR, "logout manager error,ErrID[%d],ErrMsg[%s]\n", pRspInfo->ErrorID, pRspInfo->ErrorMsg);
    }
};

/// 艾科柜台登出应答
void XeleTdSpi::onRspLogout(CXeleRspUserLogoutField* pRspField, CXeleRspInfo* pRspInfo, int nRequestID, bool bIsLast)
{
    LOGf(INFO, "  Test Logout pass\n");
    LOGf(INFO, "=======================\n");
    if (pRspInfo->ErrorID == 0)
    {
        canQuery = false;
        LOGf(INFO, "now can't use query interface\n");
    }
    else
    {
        LOGf(ERROR, "logout counter error,ErrID[%d],ErrMsg[%s]\n", pRspInfo->ErrorID, pRspInfo->ErrorMsg);
    }
};

/// 当客户端与管理中心服务端查询通信连接断开时，该方法被调用。
/// 该回报触发条件为客户主动发起登出请求，或者链路发生了断连。
/// 出现以上情况时，若不是主动发起登出请求，建议先发起登出请求，再重新登录，即可再次使用相关接口
void XeleTdSpi::onFrontManagerQueryDisconnected(int nReason)
{
    canFundTransfer = false;
    LOGf(INFO, "need relogin\n");
};

/// 当客户端与服务端查询通信连接断开时，该方法被调用。
/// 该回报触发条件为客户主动发起登出请求，或者链路发生了断连。
/// 出现以上情况时，若不是主动发起登出请求，建议先发起登出请求，再重新登录，即可再次使用相关接口
void XeleTdSpi::onFrontQueryDisconnected(int nReason)
{
    canQuery = false;
    LOGf(INFO, "need relogin\n");
};

/// 当客户端与服务端交易通信连接断开时，该方法被调用。
/// 当收到该回调，表示当前连接失去了报、撤单功能.
/// 如需恢复，参考OnFrontQueryDisconnected处理方法
void XeleTdSpi::onFrontTradeDisconnected(int nReason)
{
    canOrder = false;
    LOGf(INFO, "need re-login\n");

    ///***当盘中发生了断连，可以采取如下的处理方式，来重新获取操作权限***///
    /// 当这三种回调被调用时onFrontManagerQueryDisconnected、onFrontQueryDisconnected、onFrontTradeDisconnected可以进行如下异常处理
    /// 可以选择三种回调中某一种进行处理
    /// 可以在回调函数中处理，也可以进行实时的异常检测另起线程处理
    ///*********************************///
    /// 先发送登出请求，取消用户注册权限
    g_tradeApi->reqLogout(g_username.c_str(), g_requestID++);
    LOGf(INFO, "reqLogout\n");
    sleep(2);
    /// 此处等待所有连接都断开后，再进行下一步处理
    while (true)
    {
        LOGf(INFO, "wait !canOrder && !canQuery && !canFundTransfer\n");
        sleep(1);
        if (!canOrder && !canQuery && !canFundTransfer)
            break;
    }
    LOGf(INFO, "now api disconnect,start re-login\n");

    /// 发送登录请求，重新获取账户权限
    while (g_tradeApi->reqLogin(g_xeleConfigFile.c_str(), g_username.c_str(), g_password.c_str(),
                                   g_xeleTradeNode, '0' + MARKET_ID, ++g_requestID) != 0)
    {
        LOGf(ERROR, "call reqLogin fail,try again\n");
        sleep(3);
    }
    LOGf(INFO, "relogin send success,now wait a moment,then you can reuse other Api interface\n");
    ///***登录请求发送完成后，就可以等待onRspLoginManager、onRspLogin、onRspInitTrader回调响应，再调用其他接口***///
};

/// api内部消息打印回调
void XeleTdSpi::onApiMsg(int ret, const char* strFormat, ...)
{
    char strLog[2048]{};
    va_list arglist;
    va_start(arglist, strFormat);
    vsprintf(strLog, strFormat, arglist);
    va_end(arglist);
    // 1:error, 0:normal
    if (ret)
    {
        LOGf(ERROR, "apiError:%s\n", strLog);
    }
    else
    {
        LOGf(INFO, "api:%s\n", strLog);
    }
};

///////////////////////////////
/////// 证券、期权公用 /////////
//////////////////////////////

/// 报单应答
HEAT_ZONE_RSPORDER void XeleTdSpi::onRspInsertOrder(CXeleRspOrderInsertField* pRspField, CXeleRspInfo* pRspInfo, int nRequestID, bool bIsLast)
{
    // extern CStrategyTrade* g_pStrategy; g_pStrategy->perfTick(PerfTickRspInsertOrder);
    //
    // // SPDLOG_TRACE("onRspInsertOrder {}", nRequestID);
    // Td_RtnOrder rtnOrder;
    // std::memset(&rtnOrder, 0, sizeof(Td_RtnOrder));
    // std::memcpy(rtnOrder.accountId, pRspField->AccountID, sizeof(TXeleUserIDType));
    // rtnOrder.msgType = MsgType_RspOrder;
    // rtnOrder.bsType = pRspField->Direction;
    // rtnOrder.symbolId = atoi(pRspField->SecuritiesID);
    // rtnOrder.orderSysId = pRspField->OrderSysID;
    // rtnOrder.userLocalId = pRspField->UserLocalID;
    // rtnOrder.origOrdPrice = pRspField->LimitPrice * 10000;
    // rtnOrder.origOrdQty = pRspField->Volume;
    // if (pRspInfo->ErrorID == 0)
    //     rtnOrder.ordStatus = ODRSTAT_REPORTED;
    // else
    //     rtnOrder.ordStatus = ODRSTAT_ERROR;
    // rtnOrder.errorId = pRspInfo->ErrorID;
    // rtnOrder.requestId = nRequestID;
    //
    // uint32_t index = stoi(pRspField->SecuritiesID) % 1000000;
    // uint32_t channelIndex = CContext::getInstance()->m_SymobolList[index].channelNo % MyConfigure::getInstance()->ChannelNum;
    // sCacheDataType sCachData;
    // sCachData.FnType = MsgType_RspOrder;
    // std::memcpy(sCachData.FnParam1, &rtnOrder, sizeof(Td_RtnOrder));
    // MarketMaker::getInstance()->m_ChannelPool[channelIndex]->tryPush(sCachData);
};

/// 报单错误回报
void XeleTdSpi::onErrRtnInsertOrder(CXeleRspOrderInsertField* pRspField, CXeleRspInfo* pRspInfo, int nRequestID, bool bIsLast)
{
    // Td_RtnOrder rtnOrder;
    // std::memset(&rtnOrder, 0, sizeof(Td_RtnOrder));
    // std::memcpy(rtnOrder.accountId, pRspField->AccountID, sizeof(TXeleUserIDType));
    // rtnOrder.msgType = MsgType_RspOrder;
    // rtnOrder.bsType = pRspField->Direction;
    // rtnOrder.symbolId = atoi(pRspField->SecuritiesID);
    // rtnOrder.orderSysId = pRspField->OrderSysID;
    // rtnOrder.userLocalId = pRspField->UserLocalID;
    // rtnOrder.origOrdPrice = pRspField->LimitPrice * 10000;
    // rtnOrder.origOrdQty = pRspField->Volume;
    // rtnOrder.ordStatus = ODRSTAT_ERROR;
    // rtnOrder.errorId = pRspInfo->ErrorID;
    // rtnOrder.requestId = nRequestID;
    //
    // uint32_t index = stoi(pRspField->SecuritiesID) % 1000000;
    // uint32_t channelIndex = CContext::getInstance()->m_SymobolList[index].channelNo % MyConfigure::getInstance()->ChannelNum;
    // sCacheDataType sCachData;
    // sCachData.FnType = MsgType_RspOrder;
    // std::memcpy(sCachData.FnParam1, &rtnOrder, sizeof(Td_RtnOrder));
    // MarketMaker::getInstance()->m_ChannelPool[channelIndex]->tryPush(sCachData);
};

/// 撤单应答
HEAT_ZONE_RSPORDER void XeleTdSpi::onRspCancelOrder(CXeleRspOrderActionField* pRspField, CXeleRspInfo* pRspInfo, int nRequestID, bool bIsLast)
{
    // Td_RtnOrder rtnOrder;
    // std::memset(&rtnOrder, 0, sizeof(Td_RtnOrder));
    // std::memcpy(rtnOrder.accountId, pRspField->AccountID, sizeof(TXeleUserIDType));
    // rtnOrder.msgType = MsgType_RspCancel;
    // rtnOrder.orderSysId = pRspField->OrderSysID;
    // rtnOrder.userLocalId = pRspField->UserLocalID;
    // if (pRspInfo->ErrorID == 0)
    //     rtnOrder.ordStatus = ODRSTAT_REPORTED;
    // else
    //     rtnOrder.ordStatus = ODRSTAT_ERROR;
    // rtnOrder.errorId = pRspInfo->ErrorID;
    // rtnOrder.requestId = nRequestID;
    //
    // uint32_t channelNo = CContext::getInstance()->RegisterLoaclID(pRspField->OrigUserLocalID, 0);
    // uint32_t channelIndex = channelNo % MyConfigure::getInstance()->ChannelNum;
    // sCacheDataType sCachData;
    // sCachData.FnType = MsgType_RspCancel;
    // std::memcpy(sCachData.FnParam1, &rtnOrder, sizeof(Td_RtnOrder));
    // MarketMaker::getInstance()->m_ChannelPool[channelIndex]->tryPush(sCachData);
};

/// 撤单错误回报
void XeleTdSpi::onErrRtnCancelOrder(CXeleRspOrderActionField* pRspField, CXeleRspInfo* pRspInfo, int nRequestID, bool bIsLast)
{
    // Td_RtnOrder rtnOrder;
    // std::memset(&rtnOrder, 0, sizeof(Td_RtnOrder));
    // std::memcpy(rtnOrder.accountId, pRspField->AccountID, sizeof(TXeleUserIDType));
    // rtnOrder.msgType = MsgType_RspCancel;
    // rtnOrder.orderSysId = pRspField->OrderSysID;
    // rtnOrder.userLocalId = pRspField->UserLocalID;
    // rtnOrder.ordStatus = ODRSTAT_ERROR;
    // rtnOrder.errorId = pRspInfo->ErrorID;
    // rtnOrder.requestId = nRequestID;
    //
    // uint32_t channelNo = CContext::getInstance()->RegisterLoaclID(pRspField->OrigUserLocalID, 0);
    // uint32_t channelIndex = channelNo % MyConfigure::getInstance()->ChannelNum;
    // sCacheDataType sCachData;
    // sCachData.FnType = MsgType_RspCancel;
    // std::memcpy(sCachData.FnParam1, &rtnOrder, sizeof(Td_RtnOrder));
    // MarketMaker::getInstance()->m_ChannelPool[channelIndex]->tryPush(sCachData);
};

/// 报单回报
HEAT_ZONE_RSPORDER void XeleTdSpi::onRtnOrder(CXeleRtnOrderField* pRspField, CXeleRspInfo* pRspInfo, int nRequestID, bool bIsLast)
{
    // if (pRspField->OrderStatus == ODRSTAT_REPORTED) {
    //     extern CStrategyTrade* g_pStrategy; g_pStrategy->perfTick(PerfTickRtnOrder);
    // }
    //
    // Td_RtnOrder rtnOrder;
    // std::memset(&rtnOrder, 0, sizeof(Td_RtnOrder));
    // std::memcpy(rtnOrder.accountId, pRspField->AccountID, sizeof(TXeleUserIDType));
    // rtnOrder.msgType = MsgType_RtnOrder;
    // rtnOrder.bsType = pRspField->Direction;
    // rtnOrder.symbolId = atoi(pRspField->SecuritiesID);
    // rtnOrder.orderSysId = pRspField->OrderSysID;
    // rtnOrder.userLocalId = pRspField->UserLocalID;
    // rtnOrder.origOrdPrice = pRspField->LimitPrice * 10000;
    // rtnOrder.origOrdQty = pRspField->Volume;
    // rtnOrder.origUserLocalId = pRspField->OrigUserLocalID;
    // rtnOrder.origOrderSysId = pRspField->OrigOrderSysID;
    // rtnOrder.ordStatus = pRspField->OrderStatus;
    // rtnOrder.trdQty = pRspField->TradeVolume;
    // rtnOrder.leaveQty = pRspField->LeavesVolume;
    // rtnOrder.errorId = pRspInfo->ErrorID;
    // rtnOrder.requestId = nRequestID;
    //
    // uint32_t index = stoi(pRspField->SecuritiesID) % 1000000;
    // uint32_t channelIndex = CContext::getInstance()->m_SymobolList[index].channelNo % MyConfigure::getInstance()->ChannelNum;
    // sCacheDataType sCachData;
    // sCachData.FnType = MsgType_RtnOrder;
    // std::memcpy(sCachData.FnParam1, &rtnOrder, sizeof(Td_RtnOrder));
    // MarketMaker::getInstance()->m_ChannelPool[channelIndex]->tryPush(sCachData);
};

/// 成交回报
HEAT_ZONE_RSPORDER void XeleTdSpi::onRtnTrade(CXeleRtnTradeField* pRspField, CXeleRspInfo* pRspInfo, int nRequestID, bool bIsLast)
{
    // Td_RtnOrder rtnOrder;
    // std::memset(&rtnOrder, 0, sizeof(Td_RtnOrder));
    // std::memcpy(rtnOrder.accountId, pRspField->AccountID, sizeof(TXeleUserIDType));
    // rtnOrder.msgType = MsgType_RtnTrade;
    // rtnOrder.bsType = pRspField->Direction;
    // rtnOrder.symbolId = atoi(pRspField->SecuritiesID);
    // rtnOrder.orderSysId = pRspField->OrderSysID;
    // rtnOrder.userLocalId = pRspField->UserLocalID;
    // rtnOrder.origOrdPrice = pRspField->LimitPrice * 10000;
    // rtnOrder.origOrdQty = pRspField->Volume;
    // rtnOrder.tradeId = pRspField->TradeID;
    // rtnOrder.trdPrice = pRspField->TradePrice * 10000;
    // rtnOrder.trdQty = pRspField->TradeVolume;
    // rtnOrder.cumQty = pRspField->CumQty;
    // rtnOrder.leaveQty = pRspField->LeavesVolume;
    // rtnOrder.ordStatus = pRspField->OrderStatus;
    // rtnOrder.errorId = pRspInfo->ErrorID;
    // rtnOrder.requestId = nRequestID;
    //
    // uint32_t index = stoi(pRspField->SecuritiesID) % 1000000;
    // uint32_t channelIndex = CContext::getInstance()->m_SymobolList[index].channelNo % MyConfigure::getInstance()->ChannelNum;
    // sCacheDataType sCachData;
    // sCachData.FnType = MsgType_RtnTrade;
    // std::memcpy(sCachData.FnParam1, &rtnOrder, sizeof(Td_RtnOrder));
    // MarketMaker::getInstance()->m_ChannelPool[channelIndex]->tryPush(sCachData);
};

/// 报单查询应答
void XeleTdSpi::onRspQryOrder(CXeleRspQryOrderField* pRspField, CXeleRspInfo* pRspInfo, int nRequestID, bool bIsLast)
{
    // Td_RspQryOrder RspQryOrder;
    // std::memset(&RspQryOrder, 0, sizeof(Td_RspQryOrder));
    // RspQryOrder.msgType = MsgType_RspQryOrder;
    // RspQryOrder.symbolId = atoi(pRspField->SecuritiesID);
    // std::memcpy(RspQryOrder.accountId, pRspField->AccountID, sizeof(TXeleUserIDType));
    // RspQryOrder.requestId = nRequestID;                     // 自增长请求号
    // RspQryOrder.userLocalId = pRspField->UserLocalID;       // 用户自定义编号
    // RspQryOrder.orderSysId = pRspField->OrderSysID;         // 系统订单号
    // RspQryOrder.bsType = pRspField->Direction;              // 买卖方向 TdDirectionType
    // RspQryOrder.ordPrice = pRspField->LimitPrice * 10000;   // 委托价格, 单位精确到元后四位, 即1元 = 10000
    // RspQryOrder.ordQty = pRspField->Volume;                 // 委托数量
    // RspQryOrder.trdQty = pRspField->TradeVolume;            // 成交数量
    // RspQryOrder.trdMoney = pRspField->TradeAmount * 100000; // 成交金额 (单位精确到元后四位, 即: 1元=10000)
    // RspQryOrder.ordStatus = pRspField->OrderStatus;         // 订单当前状态 TdOrdStatus
    // RspQryOrder.errorId = pRspInfo->ErrorID;                // 错误码
    // RspQryOrder.isLast = bIsLast;                           // 是否结束
    //
    // sCacheDataType sCachData;
    // sCachData.FnType = MsgType_RspQryOrder;
    // std::memcpy(sCachData.FnParam1, &RspQryOrder, sizeof(Td_RspQryOrder));
    // MarketMaker::getInstance()->m_NotifyPool->tryPush(sCachData);
};

/// 成交查询应答
void XeleTdSpi::onRspQryTrade(CXeleRspQryTradeField* pRspField, CXeleRspInfo* pRspInfo, int nRequestID, bool bIsLast)
{
    // Td_RspQryTrade RspQryTrade;
    // std::memset(&RspQryTrade, 0, sizeof(Td_RspQryTrade));
    // RspQryTrade.msgType = MsgType_RspQryTrade; // 消息类型
    // RspQryTrade.symbolId = atoi(pRspField->SecuritiesID);
    // std::memcpy(RspQryTrade.accountId, pRspField->AccountID, sizeof(TXeleUserIDType));
    // RspQryTrade.requestId = nRequestID;                    // 自增长请求号
    // RspQryTrade.userLocalId = pRspField->UserLocalID;      // 用户自定义编号
    // RspQryTrade.orderSysId = pRspField->OrderSysID;        // 系统订单号
    // RspQryTrade.tradeId = pRspField->TradeID;              // 成交编号
    // RspQryTrade.bsType = pRspField->Direction;             // 买卖方向 TdDirectionType
    // RspQryTrade.trdPrice = pRspField->TradePrice * 10000;  // 成交价格 (单位精确到元后四位, 即: 1元=10000)
    // RspQryTrade.trdQty = pRspField->TradeVolume;           // 成交数量
    // RspQryTrade.trdMoney = pRspField->TradeAmount * 10000; // 成交金额 (单位精确到元后四位, 即: 1元=10000)
    // RspQryTrade.ordStatus = pRspField->OrderStatus;        // 订单当前状态 TdOrdStatus
    // RspQryTrade.errorId = pRspInfo->ErrorID;               // 错误码
    // RspQryTrade.isLast = bIsLast;                          // 是否结束
    //
    // sCacheDataType sCachData;
    // sCachData.FnType = MsgType_RspQryTrade;
    // std::memcpy(sCachData.FnParam1, &RspQryTrade, sizeof(Td_RspQryTrade));
    // MarketMaker::getInstance()->m_NotifyPool->tryPush(sCachData);
};

///////////////////////////////
/////// 证券相关 //////////////
//////////////////////////////

/// 证券资金查询应答
void XeleTdSpi::onRspQryFund(CXeleRspQryStockClientAccountField* pRspField, CXeleRspInfo* pRspInfo, int nRequestID, bool bIsLast)
{
    LOGf(INFO, "onRspQryFund:%.15s,AvailableFund:%lf,TotalFund:%lf,InitTotalFund:%lf\n", pRspField->AccountID, pRspField->AvailableFund, pRspField->TotalFund, pRspField->InitTotalFund);

    // Td_RspQryCashAsset RspQryCashAsset;
    // std::memset(&RspQryCashAsset, 0, sizeof(Td_RspQryCashAsset));
    //
    // RspQryCashAsset.msgType = MsgType_RspQryCashasset; // 消息类型
    // std::memcpy(RspQryCashAsset.accountId, pRspField->AccountID, sizeof(TXeleUserIDType));
    // RspQryCashAsset.requestId = nRequestID;                           // 自增长请求号
    // RspQryCashAsset.frozeCapital = pRspField->FrozeCapital * 10000;   /// 冻结资金
    // RspQryCashAsset.frozenFee = pRspField->FrozenFee * 10000;         /// 冻结手续费
    // RspQryCashAsset.usedFee = pRspField->UsedFee * 10000;             /// 已付手续费
    // RspQryCashAsset.initTotalFund = pRspField->InitTotalFund * 10000; /// 初始上场资金（不变）
    // RspQryCashAsset.totalFund = pRspField->TotalFund * 10000;         /// 上场资金（可变）； 初始上场资金 + 出入金额 ，可能为负（建议客户不使用）
    // RspQryCashAsset.sellFund = pRspField->SellFund * 10000;           /// 总卖出
    // RspQryCashAsset.buyFund = pRspField->BuyFund * 10000;             /// 总买入
    // RspQryCashAsset.availableFund = pRspField->AvailableFund * 10000; /// 可用资金
    // RspQryCashAsset.errorId = pRspInfo->ErrorID;                      // 错误码
    // RspQryCashAsset.isLast = bIsLast;                                 // 是否结束
    //
    // sCacheDataType sCachData;
    // sCachData.FnType = MsgType_RspQryCashasset;
    // std::memcpy(sCachData.FnParam1, &RspQryCashAsset, sizeof(Td_RspQryCashAsset));
    // MarketMaker::getInstance()->m_NotifyPool->tryPush(sCachData);
};

/// 证券持仓查询应答
void XeleTdSpi::onRspQryPosition(CXeleRspQryStockPositionField* pRspField, CXeleRspInfo* pRspInfo, int nRequestID, bool bIsLast)
{
    LOGf(INFO, "onRspQryPosition:%s,SecuritiesID:%s,AvailablePosition:%ld, errorId:%d,%s\n", pRspField->AccountID, pRspField->SecuritiesID, pRspField->AvailablePosition, pRspInfo->ErrorID, pRspInfo->ErrorMsg);

    // Td_RspQryPosition RspQryPosition;
    // std::memset(&RspQryPosition, 0, sizeof(Td_RspQryPosition));
    //
    // RspQryPosition.msgType = MsgType_RspQryPosition;         // 消息类型
    // RspQryPosition.symbolId = atoi(pRspField->SecuritiesID); // 证券代码
    // std::memcpy(RspQryPosition.accountId, pRspField->AccountID, sizeof(TXeleUserIDType));
    // RspQryPosition.requestId = nRequestID;                           // 自增长请求号
    // RspQryPosition.tdBuyPosition = pRspField->TdBuyPosition;         /// 今买仓
    // RspQryPosition.tdSellPosition = pRspField->TdSellPosition;       /// 今卖仓
    // RspQryPosition.unTdBuyPosition = pRspField->UnTdBuyPosition;     /// 在途买仓
    // RspQryPosition.unTdSellPosition = pRspField->UnTdSellPosition;   /// 在途卖仓
    // RspQryPosition.ydPosition = pRspField->YdPosition;               /// 昨持仓（不变）
    // RspQryPosition.totalCost = pRspField->TotalCost * 10000;         /// 持仓成本
    // RspQryPosition.remainingPosition = pRspField->RemainingPosition; /// 现有持仓数量（含未卖持仓）=老仓 + 今买仓 (-+)出入仓 - 今卖仓
    // RspQryPosition.availablePosition = pRspField->AvailablePosition; /// 可卖持仓数量
    // RspQryPosition.errorId = pRspInfo->ErrorID;                      // 错误码
    // RspQryPosition.isLast = bIsLast;                                 // 是否结束
    //
    // sCacheDataType sCachData;
    // sCachData.FnType = MsgType_RspQryPosition;
    // std::memcpy(sCachData.FnParam1, &RspQryPosition, sizeof(Td_RspQryPosition));
    // MarketMaker::getInstance()->m_NotifyPool->tryPush(sCachData);
};

}

void OES::start(const char *config)
{
    try {
        nlohmann::json json;
        std::ifstream(config) >> json;
        g_username = json["username"];
        g_password = json["password"];
        g_xeleTradeNode = json["xele_trade_node"];

        char nameBuf[L_tmpnam];
        g_xeleConfigFile = std::tmpnam(nameBuf) ?: "tmp_xele_config.txt";
        std::ofstream fout(g_xeleConfigFile, std::ios::binary);
        for (int32_t i = 0; i < json["xele_config"].size(); ++i) {
            fout << json["xele_config"][i] << '\n';
        }

    } catch (std::exception const &e) {
        SPDLOG_ERROR("config json parse failed: {}", e.what());
        throw;
    }

    g_userSpi = new XeleTdSpi;
    g_tradeApi = XeleSecuritiesTraderApi::createTraderApi();
    g_tradeApi->registerSpi(g_userSpi);

    int ret = g_tradeApi->reqLogin(g_xeleConfigFile.c_str(), g_username.c_str(), g_password.c_str(),
                                   g_xeleTradeNode, '0' + MARKET_ID, ++g_requestID);
    if (ret != 0) {
        /// ret可以结合XeleSecuritiesTraderApi.h中ApiReturnValue枚举返回值对应错误来判断常见的异常
        SPDLOG_INFO("oes xele login error: ret={}", ret);
        throw std::runtime_error("oes xele login error");
    }
}

bool OES::isStarted()
{
	// 此处是为了得到当前api可以执行的权限而做的等待，实际运用时可以直接在回调函数中进行操作
    return g_userSpi->canOrder && g_userSpi->canQuery && g_userSpi->canFundTransfer;
}

void OES::stop()
{
    delete g_userSpi;
    g_userSpi = nullptr;
    g_tradeApi->release();
    g_tradeApi = nullptr;
    if (!g_xeleConfigFile.empty()) {
        std::remove(g_xeleConfigFile.c_str());
    }
}

HEAT_ZONE_REQORDER void OES::sendRequest(ReqOrder &reqOrder)
{
    int32_t requestID = ++g_requestID;
    reqOrder.xeleReq.UserLocalID = g_maxUserLocalID + requestID;
    g_tradeApi->reqInsertOrder(reqOrder.xeleReq, requestID);
}
#endif
