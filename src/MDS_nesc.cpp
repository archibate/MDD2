#include "config.h"
#if NE
#include "MDS.h"
#include "MDD.h"
#include "timestamp.h"
#include "threadAffinity.h"
#include "securityId.h"
#include "heatZone.h"
#include "XeleCompat.h"
#include <nesc/NescMd.h>
#include <nesc/Const.h>
#include <nesc/SseMdStruct.h>
#include <nesc/SzeMdStruct.h>
#include <spdlog/spdlog.h>
#include <fmt/core.h>
#include <stdexcept>
#include <fstream>
#include <iterator>
#include <nlohmann/json.hpp>

namespace
{

NescForesight::NescMdUDPClient g_nesc;
std::atomic_flag g_isStopped{false};

#if SH
HEAT_ZONE_TICK void handleShTickMerge(const uint8_t *buf, int len)
{
    // assert(buf[0] == XeleCompat::MSG_TYPE_TICK_MERGE_SSE);
    auto &tick = const_cast<MDS::Tick &>(*reinterpret_cast<MDS::Tick const *>(buf));
    MDD::handleTick(tick);
}
#endif

#if SZ
HEAT_ZONE_TICK void handleSzTradeAndOrder(const uint8_t *buf, int len)
{
    // assert(buf[0] == XeleCompat::MSG_TYPE_TRADE_SZ || buf[0] == XeleCompat::MSG_TYPE_ORDER_SZ);
    auto &tick = const_cast<MDS::Tick &>(*reinterpret_cast<MDS::Tick const *>(buf));
    MDD::handleTick(tick);
}
#endif

void handleShSnapShotLevel1(const uint8_t *buf, int len)
{
    // assert(buf[0] == MSG_TYPE_SNAPSHOT_L1_SSE);
    auto *snapshot = reinterpret_cast<const NescForesight::MarketDataSnapshotSse *>(buf);
    MDS::Snap snap{};
    snap.stock = securityId(snapshot->securityID);
    snap.amount = snapshot->totalValueTrade / 1000;
    snap.volume = snapshot->totalVolumeTrade / 1000;
    snap.numTrades = snapshot->numTrades;
    snap.lastPrice = snapshot->lastPrice / 10;
    snap.preClosePrice = snapshot->preClosePrice / 10;
    snap.openPrice = snapshot->openPrice / 10;
    snap.highPrice = snapshot->highPrice / 10;
    snap.lowPrice = snapshot->lowPrice / 10;
    for (int i = 0; i < 5; ++i) {
        snap.askPrice[i] = snapshot->askInfo[i].price / 10;
        snap.askQuantity[i] = snapshot->askInfo[i].qty / 1000;
        snap.bidPrice[i] = snapshot->bidInfo[i].price / 10;
        snap.bidQuantity[i] = snapshot->bidInfo[i].qty / 1000;
    }
    MDD::handleSnap(snap);
}

void handleSzSnapShotLevel1(const uint8_t *buf, int len)
{
    // assert(buf[0] == MSG_TYPE_SNAPSHOT_L1_SZ);
    auto *snapshot = reinterpret_cast<const NescForesight::MarketDataSnapshotSz *>(buf);
    MDS::Snap snap{};
    snap.stock = securityId(snapshot->securityID);
    snap.amount = snapshot->totalValueTrade / 100;
    snap.volume = snapshot->totalVolumeTrade / 100;
    snap.numTrades = snapshot->numTrades;
    snap.lastPrice = snapshot->lastPrice / 10000;
    snap.preClosePrice = snapshot->preClosePrice / 100;
    snap.openPrice = snapshot->openPrice / 10000;
    snap.highPrice = snapshot->highPrice / 10000;
    snap.lowPrice = snapshot->lowPrice / 10000;
    snap.upperLimitPrice = snapshot->upperlmtPrice / 10000;
    snap.upperLimitPrice = snapshot->lowerlmtPrice / 10000;
    for (int i = 0; i < 5; ++i) {
        snap.askPrice[i] = snapshot->askInfo[i].price / 10000;
        snap.askQuantity[i] = snapshot->askInfo[i].qty / 100;
        snap.bidPrice[i] = snapshot->bidInfo[i].price / 10000;
        snap.bidQuantity[i] = snapshot->bidInfo[i].qty / 100;
    }
    MDD::handleSnap(snap);
}

}

void MDS::start(const char *config)
{
    std::string username;
    std::string password;

    try {
        nlohmann::json json;
        std::ifstream(config) >> json;
        username = json["mds_username"];
        password = json["mds_password"];

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
            .handler = handleShTickMerge,

            .m_backupIntName = "enp2s0f0",
            .m_backupLocalIp = "10.102.139.102",
            .m_backupMcastIp = "234.20.2.1",
            .m_backSwitchTime = 15,
            .m_backupMcastPort = 12001,
            .m_backupNicType = NescForesight::E_NIC_NORMAL,
        },
        {
            .m_interfaceName = "enp2s0f0",
            .m_localIp = "10.102.139.102",
            .m_mcastIp = "234.30.2.1",
            .m_mcastPort = 50500,
            .m_bindCpuId = kMDSBindCpu,
            .m_nicType = NescForesight::E_NIC_NORMAL,
            .handler = handleShSnapShotLevel1,

            .m_backupIntName = "enp2s0f0",
            .m_backupLocalIp = "10.102.139.102",
            .m_backupMcastIp = "234.30.4.1",
            .m_backSwitchTime = 15,
            .m_backupMcastPort = 60500,
            .m_backupNicType = NescForesight::E_NIC_NORMAL,
        },
        {
            .m_interfaceName = "enp2s0f0",
            .m_localIp = "10.102.139.102",
            .m_mcastIp = "234.30.6.1",
            .m_mcastPort = 51500,
            .m_bindCpuId = kMDSBindCpu,
            .m_nicType = NescForesight::E_NIC_NORMAL,
            .handler = handleSzSnapShotLevel1,

            .m_backupIntName = "enp2s0f0",
            .m_backupLocalIp = "10.102.139.102",
            .m_backupMcastIp = "234.30.8.1",
            .m_backSwitchTime = 15,
            .m_backupMcastPort = 61500,
            .m_backupNicType = NescForesight::E_NIC_NORMAL,
        },
    };
#endif
#if SZ
#if SZ2
    static NescForesight::MdParam mdChannels[] = {
        {
            .m_interfaceName = "enp1s0f1",
            .m_localIp = "127.0.0.1",
            .m_mcastIp = "234.20.8.1",
            .m_mcastPort = 16599,
            .m_bindCpuId = kMDSBindCpu,
            .m_nicType = NescForesight::E_NIC_SOLARFLARE_EFVI,
            .handler = handleSzTradeAndOrder,

            .m_backupIntName = "enp2s0f0",
            .m_backupLocalIp = "10.107.52.34",
            .m_backupMcastIp = "234.20.6.1",
            .m_backSwitchTime = 15,
            .m_backupMcastPort = 15000,
            .m_backupNicType = NescForesight::E_NIC_NORMAL,
        },
        {
            .m_interfaceName = "enp2s0f0",
            .m_localIp = "10.107.52.34",
            .m_mcastIp = "234.30.2.1",
            .m_mcastPort = 50500,
            .m_bindCpuId = kMDSBindCpu,
            .m_nicType = NescForesight::E_NIC_NORMAL,
            .handler = handleShSnapShotLevel1,

            .m_backupIntName = "enp2s0f0",
            .m_backupLocalIp = "10.107.52.34",
            .m_backupMcastIp = "234.30.4.1",
            .m_backSwitchTime = 15,
            .m_backupMcastPort = 60500,
            .m_backupNicType = NescForesight::E_NIC_NORMAL,
        },
        {
            .m_interfaceName = "enp2s0f0",
            .m_localIp = "10.107.52.34",
            .m_mcastIp = "234.30.6.1",
            .m_mcastPort = 51500,
            .m_bindCpuId = kMDSBindCpu,
            .m_nicType = NescForesight::E_NIC_NORMAL,
            .handler = handleSzSnapShotLevel1,

            .m_backupIntName = "enp2s0f0",
            .m_backupLocalIp = "10.107.52.34",
            .m_backupMcastIp = "234.30.8.1",
            .m_backSwitchTime = 15,
            .m_backupMcastPort = 61500,
            .m_backupNicType = NescForesight::E_NIC_NORMAL,
        },
    };
#else
    static NescForesight::MdParam mdChannels[] = {
        {
            .m_interfaceName = "enp1s0f0",
            .m_localIp = "127.0.0.1",
            .m_mcastIp = "234.20.8.1",
            .m_mcastPort = 16599,
            .m_bindCpuId = kMDSBindCpu,
            .m_nicType = NescForesight::E_NIC_SOLARFLARE_EFVI,
            .handler = handleSzTradeAndOrder,

            .m_backupIntName = "enp2s0f0",
            .m_backupLocalIp = "10.107.52.102",
            .m_backupMcastIp = "234.20.6.1",
            .m_backSwitchTime = 15,
            .m_backupMcastPort = 15000,
            .m_backupNicType = NescForesight::E_NIC_NORMAL,
        },
        {
            .m_interfaceName = "enp2s0f0",
            .m_localIp = "10.107.52.102",
            .m_mcastIp = "234.30.2.1",
            .m_mcastPort = 50500,
            .m_bindCpuId = kMDSBindCpu,
            .m_nicType = NescForesight::E_NIC_NORMAL,
            .handler = handleShSnapShotLevel1,

            .m_backupIntName = "enp2s0f0",
            .m_backupLocalIp = "10.107.52.102",
            .m_backupMcastIp = "234.30.4.1",
            .m_backSwitchTime = 15,
            .m_backupMcastPort = 60500,
            .m_backupNicType = NescForesight::E_NIC_NORMAL,
        },
        {
            .m_interfaceName = "enp2s0f0",
            .m_localIp = "10.107.52.102",
            .m_mcastIp = "234.30.6.1",
            .m_mcastPort = 51500,
            .m_bindCpuId = kMDSBindCpu,
            .m_nicType = NescForesight::E_NIC_NORMAL,
            .handler = handleSzSnapShotLevel1,

            .m_backupIntName = "enp2s0f0",
            .m_backupLocalIp = "10.107.52.102",
            .m_backupMcastIp = "234.30.8.1",
            .m_backSwitchTime = 15,
            .m_backupMcastPort = 61500,
            .m_backupNicType = NescForesight::E_NIC_NORMAL,
        },
    };
#endif
#endif
    NescForesight::MdParam *params[std::size(mdChannels)];
    for (size_t i = 0; i < std::size(mdChannels); ++i) {
        params[i] = &mdChannels[i];
    }
    if (g_nesc.Init(params, std::size(mdChannels)) != 0) {
        SPDLOG_ERROR("mds nesc channels init failed");
        throw std::runtime_error("mds nesc channels init failed");
    }

    static const NescForesight::EMdMsgType kSubscribeMessageTypes[] = {
#if SZ
        NescForesight::eSzTrade,
        NescForesight::eSzOrder,
#endif
#if SH
        NescForesight::eShTickMerge,
#endif
        NescForesight::eSzSnapShotLevel1,
        NescForesight::eShSnapShotLevel1,
    };

    // std::vector<std::string> securityIdStrs;
    // std::vector<const char *> securityIds;
    // securityIdStrs.reserve(n);
    // securityIds.reserve(n);
    // for (int32_t i = 0; i < n; ++i) {
    //     securityIdStrs.push_back(fmt::format("{:06d}", stocks[i]));
    //     securityIds.push_back(securityIdStrs.back().c_str());
    // }
    //
    // if (g_nesc.SubscribeMarketData(
    //     kSubscribeMessageTypes, std::size(kSubscribeMessageTypes),
    //     NescForesight::MarketType(MARKET_ID),
    //     securityIds.data(), securityIds.size()) != 0) {
    //     SPDLOG_ERROR("mds nesc subscribe failed");
    //     throw std::runtime_error("mds nesc subscribe failed");
    // }

    if (g_nesc.SubscribeAll(kSubscribeMessageTypes, std::size(kSubscribeMessageTypes)) != 0) {
        SPDLOG_ERROR("mds nesc subscribe failed");
        throw std::runtime_error("mds nesc subscribe failed");
    }
}

void MDS::startReceive()
{
    NescForesight::SseStaticInfoField g_sseStatic;
    NescForesight::SzStaticInfoField g_szStatic;

    SPDLOG_INFO("querying SH static info, please wait");
    if (g_nesc.QuerySseStaticInfo(g_sseStatic) != 0) {
        SPDLOG_ERROR("nesc QuerySseStaticInfo failed");
        throw std::runtime_error("nesc QuerySseStaticInfo failed");
    }
    SPDLOG_DEBUG("publishing {} SH static info", g_sseStatic.count);
    for (int i = 0; i < g_sseStatic.count; ++i) {
        MDS::Stat stat{};
        stat.stock = securityId(g_sseStatic.staticInfos[i].securityID);
        stat.preClosePrice = static_cast<uint32_t>(std::round(g_sseStatic.staticInfos[i].prevClosePx * 100));
        stat.upperLimitPrice = static_cast<uint32_t>(std::round(g_sseStatic.staticInfos[i].upperLimitPrice * 100));
        stat.lowerLimitPrice = static_cast<uint32_t>(std::round(g_sseStatic.staticInfos[i].lowerLimitPrice * 100));
        MDD::handleStatic(stat);
    }

    SPDLOG_INFO("querying SZ static info, please wait");
    if (g_nesc.QuerySzStaticInfo(g_szStatic) != 0) {
        SPDLOG_ERROR("nesc QuerySzStaticInfo failed");
        throw std::runtime_error("nesc QuerySzStaticInfo failed");
    }
    SPDLOG_DEBUG("publishing {} SZ static info", g_szStatic.count);
    for (int i = 0; i < g_szStatic.count; ++i) {
        MDS::Stat stat{};
        stat.stock = securityId(g_szStatic.staticInfos[i].securityID);
        stat.preClosePrice = static_cast<uint32_t>(std::round(g_szStatic.staticInfos[i].prevClosePx * 100));
        stat.upperLimitPrice = static_cast<uint32_t>(std::round(g_szStatic.staticInfos[i].upperLimitPrice * 100));
        stat.lowerLimitPrice = static_cast<uint32_t>(std::round(g_szStatic.staticInfos[i].lowerLimitPrice * 100));
        MDD::handleStatic(stat);
    }

    SPDLOG_INFO("starting nesc receive");
    if (g_nesc.Start() != 0) {
        SPDLOG_ERROR("mds nesc start failed");
        throw std::runtime_error("mds nesc start failed");
    }
}

void MDS::stop()
{
    if (!g_isStopped.test_and_set()) {
        g_nesc.Stop();
    }
}

void MDS::requestStop()
{
    MDS::stop();
}

bool MDS::isFinished()
{
    return g_isStopped.test();
}

bool MDS::isStarted()
{
    return true;
}
#endif
