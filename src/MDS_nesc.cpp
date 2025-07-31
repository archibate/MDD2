#include "config.h"
#if NE
#include "MDS.h"
#include "MDD.h"
#include "timestamp.h"
#include "threadAffinity.h"
#include <nesc/NescMd.h>
#include <spdlog/spdlog.h>
#include <fmt/core.h>
#include <stdexcept>
#include <fstream>
#include <nlohmann/json.hpp>

namespace
{

NescForesight::NescMdUDPClient g_nesc;
NescForesight::SseStaticInfoField g_sseStatic;
NescForesight::SzStaticInfoField g_szStatic;

}

void MDS::subscribe(int32_t const *stocks, int32_t n)
{
    std::vector<std::string> securityIdStrs;
    std::vector<const char *> securityIds;
    securityIdStrs.reserve(n);
    securityIds.reserve(n);
    for (int32_t i = 0; i < n; ++i) {
        securityIdStrs.push_back(fmt::format("{:06d}", i));
        securityIds.push_back(securityIdStrs.back().c_str());
    }

    static const NescForesight::EMdMsgType messageTypes[] = {
        NescForesight::eSzTrade,
        NescForesight::eSzOrder,
        NescForesight::eShTickMerge,
        NescForesight::eSzSnapShotLevel1,
        NescForesight::eShSnapShotLevel1,
    };
    if (g_nesc.SubscribeMarketData(
        messageTypes, sizeof(messageTypes) / sizeof(messageTypes[0]),
        NescForesight::MarketType(MARKET_ID),
        securityIds.data(), securityIds.size()) != 0) {
        SPDLOG_ERROR("mds nesc subscribe failed");
        throw std::runtime_error("mds nesc subscribe failed");
    }
}

MDS::Stat MDS::getStatic(int32_t stock)
{
    if (stock >= 600000) {
        auto *stat = g_sseStatic.staticInfos;
        auto *statEnd = g_sseStatic.staticInfos + g_sseStatic.count;
        for (; stat < statEnd; ++stat) {
            break;
        }
        if (stat == statEnd) {
            SPDLOG_ERROR("not found in static info: stock={}", stock);
            return {};
        }
        if (stat->productStatus[0] == 'N' || stat->productStatus[3] != 'D' || stat->productStatus[6] != 'F' || stat->productStatus[9] != 'N') {
            SPDLOG_WARN("stock has bad product status: stock={} status={:.21s}", stock, stat->productStatus);
        }
        return {.marketType = NescForesight::SSE, .staticSseInfo = *stat};

    } else {
        auto *stat = g_szStatic.staticInfos;
        auto *statEnd = g_szStatic.staticInfos + g_szStatic.count;
        for (; stat < statEnd; ++stat) {
            break;
        }
        if (stat == statEnd) {
            SPDLOG_ERROR("not found in static info: stock={}", stock);
            return {};
        }
        if (stat->securityStatus[0] || stat->securityStatus[3] || stat->securityStatus[4] || stat->securityStatus[5] || stat->securityStatus[9] || stat->securityStatus[16] || stat->securityStatus[17]) {
            SPDLOG_WARN("stock has bad product status: stock={} status={:.20s}", stock, stat->securityStatus);
        }
        return {.marketType = NescForesight::SZE, .staticSzInfo = *stat};
    }
}

void MDS::start(const char *config)
{
    std::string username;
    std::string password;

    try {
        nlohmann::json json;
        std::ifstream(config) >> json;
        username = json["username"];
        password = json["password"];

    } catch (std::exception const &e) {
        SPDLOG_ERROR("config json parse failed: {}", e.what());
        throw;
    }

    if (!g_nesc.Login(
        /*tcp_ipaddr=*/"10.102.134.46",
        /*tcp_port=*/30000,
        /*username=*/username.c_str(),
        /*password=*/password.c_str(),
        /*back_tcp_ipaddr=*/"10.102.134.47",
        /*back_tcp_port=*/30000
    )) {
        SPDLOG_ERROR("mds nesc login failed");
        throw std::runtime_error("mds nesc login failed");
    }

#if SH
    static NescForesight::MdParam mdChannels[] = {
        {
            .m_interfaceName = "enp1s0f0",
            .m_localIp = "127.0.0.1",
            .m_mcastIp = "234.20.4.3",
            .m_mcastPort = 23501,
            .m_bindCpuId = kMDSBindCpu,
            .m_nicType = NescForesight::E_NIC_SOLARFLARE_EFVI,

            .m_backupIntName = "enp2s0f0",
            .m_backupLocalIp = "10.102.139.102",
            .m_backupMcastIp = "234.20.2.1",
            .m_backSwitchTime = 15,
            .m_backupMcastPort = 12001,
            .m_backupNicType = NescForesight::E_NIC_NORMAL,
        },
    };
#endif
#if SZ
    static const NescForesight::MdParam mdChannels[] = {
        {
            .m_interfaceName = "enp1s0f0",
            .m_localIp = "127.0.0.1",
            .m_mcastIp = "234.20.8.1",
            .m_mcastPort = 16599,
            .m_bindCpuId = kMDSBindCpu,
            .m_nicType = NescForesight::E_NIC_SOLARFLARE_EFVI,

            .m_backupIntName = "enp2s0f0",
            .m_backupLocalIp = "10.107.52.102",
            .m_backupMcastIp = "234.20.6.1",
            .m_backSwitchTime = 15,
            .m_backupMcastPort = 15000,
            .m_backupNicType = NescForesight::E_NIC_NORMAL,
        },
    };
#endif
    constexpr size_t kNumMdParams = sizeof(mdChannels) / sizeof(mdChannels[0]);
    NescForesight::MdParam *params[kNumMdParams];
    for (size_t i = 0; i < kNumMdParams; ++i) {
        params[i] = &mdChannels[i];
    }
    if (g_nesc.Init(params, kNumMdParams) == 0) {
        SPDLOG_ERROR("mds nesc channels init failed");
        throw std::runtime_error("mds nesc channels init failed");
    }
}

void MDS::startReceive()
{
    if (g_nesc.Start() != 0) {
        SPDLOG_ERROR("mds nesc start failed");
        throw std::runtime_error("mds nesc start failed");
    }

    SPDLOG_INFO("querying SH static info");
    if (g_nesc.QuerySseStaticInfo(g_sseStatic) != 0) {
        SPDLOG_ERROR("nesc QuerySseStaticInfo failed");
        throw std::runtime_error("nesc QuerySseStaticInfo failed");
    }
    SPDLOG_INFO("querying SZ static info");
    if (g_nesc.QuerySzStaticInfo(g_szStatic) != 0) {
        SPDLOG_ERROR("nesc QuerySzStaticInfo failed");
        throw std::runtime_error("nesc QuerySzStaticInfo failed");
    }
}

void MDS::stop()
{
    g_nesc.Stop();
}

void MDS::requestStop()
{
    g_nesc.Stop();
}

bool MDS::isFinished()
{
    return false;
}

bool MDS::isStarted()
{
    return true;
}
#endif
