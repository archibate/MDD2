#include "config.h"
#if OST
#include "OES.h"
#include "MDD.h"
#include "constants.h"
#include "securityId.h"
#include "heatZone.h"
#include <atomic>
#include <stdexcept>
#include <fstream>
#include <vector>
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>


namespace
{

std::string g_username;
std::string g_password;
std::atomic<uint32_t> g_requestID{1};

CUTApi *api;
std::atomic_bool g_loginOK = false;
std::vector<CUTDepthMarketDataField> g_marketStatics;

class OstUserSpi : public CUTSpi
{
public:
    void OnFrontConnected() override
    {
        SPDLOG_INFO("front connected");
    }

    void OnFrontDisconnected(int nReason) override
    {
        SPDLOG_WARN("front disconnected: reason=0x{:x}", nReason);
    }

    void OnRspLogin(CUTRspLoginField *pRspLogin, CUTRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override
    {
        if (pRspInfo->ErrorID != 0) {
            SPDLOG_ERROR("login returned error: errorId={} errorMsg=[{}]", pRspInfo->ErrorID, pRspInfo->ErrorMsg);
            return;
        }

        SPDLOG_INFO("login returned: date={} timestamp={} frontId={} sessionId={} multiAddress={}", pRspLogin->TradingDay, pRspLogin->LoginTime, pRspLogin->FrontID, pRspLogin->SessionID, pRspLogin->MultiAddress);
        g_loginOK.store(true, std::memory_order_relaxed);

        CUTQryDepthMarketDataField depthQuery;
        depthQuery.ExchangeID = UT_EXG_SZSE;
        depthQuery.ExchangeID = UT_EXG_SSE;
        memset(&depthQuery, 0, sizeof(CUTQryDepthMarketDataField));
        int err = api->ReqQryDepthMarketData(&depthQuery, g_requestID.fetch_add(1, std::memory_order_relaxed));
        if (err != 0) {
            SPDLOG_ERROR("query depth market data error: {}", err);
            return;
        }
    }

	void OnRspQryDepthMarketData(CUTDepthMarketDataField *pDepthMarketData, CUTRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override
    {
        if (pRspInfo->ErrorID != 0) {
            SPDLOG_ERROR("depth market data returned error: errorId={} errorMsg=[{}]", pRspInfo->ErrorID, pRspInfo->ErrorMsg);
            return;
        }

        if (pDepthMarketData->ExchangeID == UT_EXG_SSE || pDepthMarketData->ExchangeID == UT_EXG_SZSE)
        {
            int32_t stock = securityId(pDepthMarketData->InstrumentID);
            g_marketStatics.push_back(*pDepthMarketData);
        }
    }

    void OnRspError(CUTRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override
    {
        SPDLOG_ERROR("ost trade api error: errorId={} errorMsg=[{}]", pRspInfo->ErrorID, pRspInfo->ErrorMsg);
    }

    void OnRspOrderInsert(CUTInputOrderField *pInputOrder, CUTRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override
    {
        // if (pRspInfo->ErrorID != 0) {
        //     SPDLOG_WARN("insert order returned error: stock={} orderId={} errorId={} errorMsg=[{}]", pInputOrder->InstrumentID, pInputOrder->OrderRef, pRspInfo->ErrorID, pRspInfo->ErrorMsg);
        // }

        OES::RspOrder rsp{};
        rsp.rspType = OES::RspOrder::OstRspOrderInsert;
        rsp.errorID = pRspInfo->ErrorID;
        rsp.errorMsg = pRspInfo->ErrorMsg;
        rsp.ostRspOrderInsert = pInputOrder;

        MDD::handleRspOrder(rsp);
    }

    void OnRspOrderAction(CUTInputOrderActionField *pInputOrderAction, CUTRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override
    {
        // if (pRspInfo->ErrorID != 0) {
        //     SPDLOG_WARN("cancel order returned error: orderCancelId={} orderId={} errorId={} errorMsg=[{}]", pInputOrderAction->OrderActionRef, pInputOrderAction->OrderRef, pRspInfo->ErrorID, pRspInfo->ErrorMsg);
        // }

        OES::RspOrder rsp{};
        rsp.rspType = OES::RspOrder::OstRspOrderAction;
        rsp.requestID = nRequestID;
        rsp.errorID = pRspInfo->ErrorID;
        rsp.errorMsg = pRspInfo->ErrorMsg;
        memcpy(rsp.userLocalID, pInputOrderAction->OrderLocalID, sizeof(rsp.userLocalID));
        rsp.ostRspOrderAction = pInputOrderAction;

        MDD::handleRspOrder(rsp);
    }

    void OnErrRtnOrderAction(CUTOrderActionField *pOrderAction) override
    {
        OES::RspOrder rsp{};
        rsp.rspType = OES::RspOrder::OstErrRtnOrderAction;
        rsp.requestID = 0;
        rsp.errorID = pOrderAction->ExchangeErrorID;
        rsp.errorMsg = "(ExchangeError)";
        memcpy(rsp.userLocalID, pOrderAction->OrderLocalID, sizeof(rsp.userLocalID));
        rsp.ostErrRtnOrderAction = pOrderAction;

        MDD::handleRspOrder(rsp);
    }

    void OnRtnOrder(CUTOrderField *pOrder) override
    {
        OES::RspOrder rsp{};
        rsp.rspType = OES::RspOrder::OstRtnOrder;
        rsp.errorID = 0;
        rsp.errorMsg = "(success)";
        memcpy(rsp.userLocalID, pOrder->OrderLocalID, sizeof(rsp.userLocalID));
        rsp.ostRtnOrder = pOrder;

        MDD::handleRspOrder(rsp);
    }

    void OnRtnTrade(CUTTradeField *pTrade) override
    {
        OES::RspOrder rsp{};
        rsp.rspType = OES::RspOrder::OstRtnTrade;
        rsp.errorID = 0;
        rsp.errorMsg = "(success)";
        memcpy(rsp.userLocalID, pTrade->OrderLocalID, sizeof(rsp.userLocalID));
        rsp.ostRtnTrade = pTrade;

        MDD::handleRspOrder(rsp);
    }
};

OstUserSpi *spi;

}


void OES::start(const char *config)
{
    std::string ostTradeFrontIP;
    try {
        nlohmann::json json;
        std::ifstream(config) >> json;
        g_username = json["username"];
        g_password = json["password"];
        ostTradeFrontIP = json["ost_trade_front_ip"];

    } catch (std::exception const &e) {
        SPDLOG_ERROR("config json parse failed: {}", e.what());
        throw;
    }

    SPDLOG_INFO("ost trade api version {}", CUTApi::GetApiVersion());
    SPDLOG_DEBUG("ost trade: CreateApi");
    api = CUTApi::CreateApi("", kOESRecvCpu);

    SPDLOG_DEBUG("ost trade: RegisterSpi");
	spi = new OstUserSpi;
	api->RegisterSpi(spi);

    SPDLOG_DEBUG("ost trade: ReqLogin");
    CUTReqLoginField login;
	memset(&login, 0, sizeof(login));
	strncpy(login.UserID, g_username.c_str(), sizeof(login.UserID));
	strncpy(login.Password, g_password.c_str(), sizeof(login.Password));
	strncpy(login.UserProductInfo, "MDD-OST", sizeof(login.UserProductInfo));
    int err = api->ReqLogin(&login, g_requestID.fetch_add(1, std::memory_order_relaxed));
    if (err != 0) {
        SPDLOG_ERROR("ost trade login error: {}", err);
        throw std::runtime_error("ost trade login error");
    }

    SPDLOG_DEBUG("ost trade: RegisterFront");
	api->RegisterFront(const_cast<char *>(ostTradeFrontIP.c_str()));
    SPDLOG_DEBUG("ost trade: SubscribePrivateTopic");
	api->SubscribePrivateTopic(UT_TERT_QUICK);
    SPDLOG_DEBUG("ost trade: Init");
    api->Init();
}

bool OES::isStarted()
{
    return true;
}

void OES::stop()
{
    api->Release();
}

HEAT_ZONE_REQORDER void OES::sendReqOrder(ReqOrder &reqOrder)
{
    api->ReqOrderInsert(&reqOrder.inputOrder, g_requestID.fetch_add(1, std::memory_order_relaxed));
}

HEAT_ZONE_REQORDER void OES::sendReqCancel(ReqCancel &reqCancel)
{
    api->ReqOrderAction(&reqCancel.inputOrderAction, g_requestID.fetch_add(1, std::memory_order_relaxed));
}

CUTDepthMarketDataField *OES::getDepthMarketData(int32_t stock)
{
    for (auto &data: g_marketStatics) {
        if (securityId(data.InstrumentID) == stock) {
            return &data;
        }
    }
    return nullptr;
}
#endif
