#include "config.h"
#if OST
#include "OES.h"
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
int32_t g_requestID;

CUTApi *api;
std::atomic_bool g_loginOK = false;
std::vector<CUTDepthMarketDataField> g_marketStatics;

class OstUserSpi : public CUTSpi
{
public:
    virtual void OnFrontConnected() {
        SPDLOG_INFO("front connected");
    }

    virtual void OnFrontDisconnected(int nReason) {
        SPDLOG_WARN("front disconnected: reason=0x{:x}", nReason);
    }

    virtual void OnRspLogin(CUTRspLoginField *pRspLogin, CUTRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
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
        int err = api->ReqQryDepthMarketData(&depthQuery, ++g_requestID);
        if (err != 0) {
            SPDLOG_ERROR("query depth market data error: {}", err);
            return;
        }
    }

	virtual void OnRspQryDepthMarketData(CUTDepthMarketDataField *pDepthMarketData, CUTRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
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
    int err = api->ReqLogin(&login, ++g_requestID);
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
    api->ReqOrderInsert(&reqOrder.inputOrder, ++g_requestID);
}

HEAT_ZONE_REQORDER void OES::sendReqCancel(ReqCancel &reqCancel)
{
    api->ReqOrderAction(&reqCancel.inputOrderAction, ++g_requestID);
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
