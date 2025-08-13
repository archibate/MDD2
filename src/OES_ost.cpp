#include "config.h"
#if OST
#include "OES.h"
#include "MDD.h"
#include "constants.h"
#include "heatZone.h"
#include "OrderRefLut.h"
#include "clockMonotonic.h"
#include <atomic>
#include <cstring>
#include <stdexcept>
#include <fstream>
#include <vector>
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>


namespace
{

std::string g_username;
std::string g_password;

std::vector<CUTDepthMarketDataField> g_marketStatics;
std::atomic_int g_staticOK{0};

class OstUserSpi : public CUTSpi
{
public:
    CUTApi *m_api;
    TUTFrontIDType m_frontID{};
    TUTSessionIDType m_sessionID{};
    std::atomic_bool m_loginOK{false};
    std::atomic<uint32_t> m_requestID{1};
    OrderRefLut orderRefLut;

    explicit OstUserSpi(CUTApi *api) : m_api(api) {}

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

        SPDLOG_INFO("login returned: date={} timestamp={} frontId={} sessionId={} multiAddress={}",
                    pRspLogin->TradingDay, pRspLogin->LoginTime, pRspLogin->FrontID, pRspLogin->SessionID, pRspLogin->MultiAddress);
        m_loginOK.store(true);

        m_frontID = pRspLogin->FrontID;
        m_sessionID = pRspLogin->SessionID;

        CUTQryDepthMarketDataField depthQuery;
        SPDLOG_DEBUG("querying SSE and SZSE depth market data");
        memset(&depthQuery, 0, sizeof(CUTQryDepthMarketDataField));
        depthQuery.ExchangeID = UT_EXG_SSE;
        int err = m_api->ReqQryDepthMarketData(&depthQuery, m_requestID.fetch_add(1, std::memory_order_relaxed));
        if (err != 0) {
            SPDLOG_ERROR("query SSE depth market data error: {}", err);
            return;
        }
        depthQuery.ExchangeID = UT_EXG_SZSE;
        err = m_api->ReqQryDepthMarketData(&depthQuery, m_requestID.fetch_add(1, std::memory_order_relaxed));
        if (err != 0) {
            SPDLOG_ERROR("query SZSE depth market data error: {}", err);
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
            g_marketStatics.push_back(*pDepthMarketData);
            if (bIsLast) {
                g_staticOK.fetch_add(1);
            }
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
        rsp.userLocalID = orderRefLut.orderRefLookup(pInputOrder->OrderRef);
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
        rsp.userLocalID = orderRefLut.orderRefLookup(pInputOrderAction->OrderRef);
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
        rsp.userLocalID = orderRefLut.orderRefLookup(pOrderAction->OrderRef);
        rsp.ostErrRtnOrderAction = pOrderAction;

        MDD::handleRspOrder(rsp);
    }

    void OnRtnOrder(CUTOrderField *pOrder) override
    {
        OES::RspOrder rsp{};
        rsp.rspType = OES::RspOrder::OstRtnOrder;
        rsp.errorID = 0;
        rsp.errorMsg = "(success)";
        rsp.userLocalID = orderRefLut.orderRefLookup(pOrder->OrderRef);
        rsp.ostRtnOrder = pOrder;

        MDD::handleRspOrder(rsp);
    }

    void OnRtnTrade(CUTTradeField *pTrade) override
    {
        OES::RspOrder rsp{};
        rsp.rspType = OES::RspOrder::OstRtnTrade;
        rsp.errorID = 0;
        rsp.errorMsg = "(success)";
        rsp.userLocalID = orderRefLut.orderRefLookup(pTrade->OrderRef);
        rsp.ostRtnTrade = pTrade;

        MDD::handleRspOrder(rsp);
    }
};

OstUserSpi *g_spi;

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
    SPDLOG_DEBUG("ost trade bound to cpu: {}", kOESRecvCpu);
    CUTApi *api = CUTApi::CreateApi("", kOESRecvCpu);

    SPDLOG_DEBUG("ost trade: RegisterSpi");
	g_spi = new OstUserSpi(api);
	api->RegisterSpi(g_spi);

    SPDLOG_DEBUG("ost trade: ReqLogin");
    CUTReqLoginField login;
	memset(&login, 0, sizeof(login));
	strncpy(login.UserID, g_username.c_str(), sizeof(login.UserID));
	strncpy(login.Password, g_password.c_str(), sizeof(login.Password));
	strncpy(login.UserProductInfo, "MDD-OST", sizeof(login.UserProductInfo));
    int err = api->ReqLogin(&login, g_spi->m_requestID.fetch_add(1, std::memory_order_relaxed));
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
    return g_spi->m_loginOK.load();
}

void OES::stop()
{
    g_spi->m_api->Release();
    g_spi->m_api = nullptr;
    delete g_spi;
    g_spi = nullptr;
}

HEAT_ZONE_REQORDER void OES::sendReqOrder(ReqOrder &reqOrder)
{
    int32_t requestID = g_spi->m_requestID.fetch_add(1, std::memory_order_relaxed);
    reqOrder.inputOrder.OrderRef = requestID;
    g_spi->m_api->ReqOrderInsert(&reqOrder.inputOrder, requestID);
    g_spi->orderRefLut.setOrderRef(requestID, reqOrder.userLocalID);
}

// HEAT_ZONE_REQORDER void OES::sendBatchReqOrder(ReqOrder &reqOrder)
// {
//     int32_t requestID = g_spi->m_requestID.fetch_add(1, std::memory_order_relaxed);
//     reqOrder.inputOrder.OrderRef = requestID;
//     g_spi->m_api->ReqOrderInsert(&reqOrder.inputOrder, requestID);
//     g_spi->orderRefLut.setOrderRef(requestID, reqOrder.userLocalID);
// }

HEAT_ZONE_REQORDER void OES::sendReqCancel(ReqCancel &reqCancel)
{
    int32_t requestID = g_spi->m_requestID.fetch_add(1, std::memory_order_relaxed);
    reqCancel.inputOrderAction.OrderActionRef = requestID;
    g_spi->m_api->ReqOrderAction(&reqCancel.inputOrderAction, requestID);
}

CUTDepthMarketDataField *OES::getDepthMarketData(size_t &size)
{
    while (g_staticOK.load() < 2) {
        SPDLOG_DEBUG("waiting for market statics ready");
        monotonicSleepFor(50'000'000);
    }
    size = g_marketStatics.size();
    return g_marketStatics.data();
}

void OES::getFrontID(TUTFrontIDType &frontID, TUTSessionIDType &sessionID)
{
    frontID = g_spi->m_frontID;
    sessionID = g_spi->m_sessionID;
}

void OES::getInvestorID(TUTInvestorIDType investorID)
{
    std::strncpy(investorID, g_username.c_str(), sizeof(TUTInvestorIDType));
}
#endif
