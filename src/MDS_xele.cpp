#include "config.h"
#if XC
#include "MDS.h"
#include "MDD.h"
#include "timestamp.h"
#include "threadAffinity.h"
#include "securityId.h"
#include "heatZone.h"
#include <xele/XeleMd.h>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <fstream>
#include <iterator>
#include <nlohmann/json.hpp>

namespace
{

XeleMd g_xeleMd;
std::atomic_flag g_isStopped{false};

#if SH
HEAT_ZONE_TICK void handleShTickMerge(uint8_t *buf, int len)
{
    // assert(buf[0] == MSG_TYPE_TICK_MERGE_SSE);
    auto &tick = const_cast<MDS::Tick &>(*reinterpret_cast<MDS::Tick const *>(buf));
    MDD::handleTick(tick);
}
#endif

#if SZ
HEAT_ZONE_TICK void handleSzTradeAndOrder(const uint8_t *buf, int len)
{
    // assert(buf[0] == MSG_TYPE_TRADE_SZ || buf[0] == MSG_TYPE_ORDER_SZ);
    auto &tick = const_cast<MDS::Tick &>(*reinterpret_cast<MDS::Tick const *>(buf));
    MDD::handleTick(tick);
}
#endif

}

void MDS::start(const char *config)
{
    std::string username;
    std::string password;

    try {
        nlohmann::json json;
        std::ifstream(config) >> json;

    } catch (std::exception const &e) {
        SPDLOG_ERROR("config json parse failed: {}", e.what());
        throw;
    }

#if SH
    static MdParam mdChannels[] = {
        {
            .m_interfaceName = "enp1s0f0",
            .m_localIp = "10.208.48.74",
            .m_mcastIp = "233.0.102.114",
            .m_mcastPort = 50104,
            .m_bindCpuId = kMDSBindCpu,
            .m_nicType = E_NIC_SOLARFLARE_EFVI,
            .handler = handleShTickMerge,
            .bindHandler = nullptr,
            .m_polling = true,
            .m_cache = 2048,
            .m_cacheCpuId = kMDSCacheCpu,

            .m_backupIntName = "enp1s0f0",
            .m_backupLocalIp = "10.208.48.74",
            .m_backupMcastIp = "233.0.102.114",
            .m_backupMcastPort = 50114,
            .m_backupNicType = E_NIC_SOLARFLARE_EFVI,
            .m_backupCpuId = kMDSBackupCpu,
        },
    };
#endif
#if SZ
    static MdParam mdChannels[] = {
        {
            .m_interfaceName = "enp1s0f1",
            .m_localIp = "10.33.58.86",
            .m_mcastIp = "233.0.202.112",
            .m_mcastPort = 50102,
            .m_bindCpuId = kMDSBindCpu,
            .m_nicType = E_NIC_SOLARFLARE_EFVI,
            .handler = handleSzTradeAndOrder,
            .bindHandler = nullptr,
            .m_polling = true,
            .m_cache = 2048,
            .m_cacheCpuId = kMDSCacheCpu,

            .m_backupIntName = "enp1s0f1",
            .m_backupLocalIp = "10.33.58.86",
            .m_backupMcastIp = "233.0.202.112",
            .m_backupMcastPort = 50112,
            .m_backupNicType = E_NIC_SOLARFLARE_EFVI,
            .m_backupCpuId = kMDSBackupCpu,
        },
    };
#endif
    MdParam *params[std::size(mdChannels)];
    for (size_t i = 0; i < std::size(mdChannels); ++i) {
        params[i] = &mdChannels[i];
    }
    if (g_xeleMd.Init(params, std::size(mdChannels)) != 0) {
        SPDLOG_ERROR("mds xele channels init failed");
        throw std::runtime_error("mds xele channels init failed");
    }
}

void MDS::startReceive()
{
    SPDLOG_INFO("starting xele receive");
    if (g_xeleMd.Start() != 0) {
        SPDLOG_ERROR("mds xele start failed");
        throw std::runtime_error("mds xele start failed");
    }
}

void MDS::stop()
{
    if (!g_isStopped.test_and_set()) {
        g_xeleMd.Stop();
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
