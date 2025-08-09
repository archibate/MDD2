#include "config.h"
#if OST
#include "MDS.h"
#include "MDD.h"
#include "OES.h"
#include "constants.h"
#include "heatZone.h"
#include "ostmd/ostmd.h"
#include <atomic>
#include <fstream>
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>

namespace
{

std::atomic_flag g_isStopped{false};
std::string g_ostmdConfigFile;

}

#if SH
HEAT_ZONE_TICK void MDS::handleOstQuote(sse_hpf_tick &q)
{
    auto p = reinterpret_cast<MDS::Tick *>(reinterpret_cast<char *>(&q) + offsetof(sse_hpf_tick, m_tick_index));
    MDD::handleTick(*p);
}
#endif

#if SZ
HEAT_ZONE_TICK void MDS::handleOstQuote(sze_hpf_pkt_head &q)
{
    auto p = reinterpret_cast<MDS::Tick *>(reinterpret_cast<char *>(&q) + offsetof(sze_hpf_pkt_head, m_message_type));
    MDD::handleTick(*p);
}
#endif

void MDS::handleOstQuote(sse_hpf_lev2 &q)
{
}

void MDS::handleOstQuote(sze_hpf_lev2_pkt &q)
{
}

void MDS::start(const char *config)
{
    SPDLOG_INFO("starting ostmd");

    try {
        nlohmann::json json;
        std::ifstream(config) >> json;
        g_ostmdConfigFile = json["ostmd_config_file"];

    } catch (std::exception const &e) {
        SPDLOG_ERROR("config json parse failed: {}", e.what());
        throw;
    }
}

void MDS::startReceive()
{
    SPDLOG_INFO("ostmd start receiving");
    SPDLOG_DEBUG("ostmd receive cpu: {}", kMDSBindCpu);
    OstStart(g_ostmdConfigFile.c_str(), kMDSBindCpu);
}

void MDS::subscribe(int32_t const *stocks, int32_t n)
{
}

void MDS::stop()
{
    OstStop();
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

MDS::Stat MDS::getStatic(int32_t stock)
{
    auto p = OES::getDepthMarketData(stock);
    if (!p) {
        SPDLOG_ERROR("not found in static info: stock={}", stock);
        return {};
    }
    return {*p};
}

#endif
