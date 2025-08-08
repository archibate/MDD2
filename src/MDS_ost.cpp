#include "config.h"
#if OST
#include "MDS.h"
#include "MDD.h"
#include "heatZone.h"
#include "ostmd/ostmd.h"
#include <atomic>
#include <fstream>
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>

namespace
{

std::atomic_flag g_isStopped{false};

}

#if SH
HEAT_ZONE_TICK void handleOstQuote(sse_hpf_tick &q)
{
    auto p = reinterpret_cast<MDS::Tick *>(reinterpret_cast<char *>(&q) + offsetof(sse_hpf_tick, m_tick_index));
    MDD::handleTick(*p);
}
#endif

#if SZ
HEAT_ZONE_TICK void handleOstQuote(sze_hpf_pkt_head &q)
{
    auto p = reinterpret_cast<MDS::Tick *>(reinterpret_cast<char *>(&q) + offsetof(sze_hpf_pkt_head, m_message_type));
    MDD::handleTick(*p);
}
#endif

void handleOstQuote(sse_hpf_lev2 &q)
{
}

void handleOstQuote(sze_hpf_lev2_pkt &q)
{
}

void MDS::start(const char *config)
{
    SPDLOG_INFO("starting ostmd");
    std::string ostmdConfigFile;

    try {
        nlohmann::json json;
        std::ifstream(config) >> json;
        ostmdConfigFile = json["ostmd_config_file"];

    } catch (std::exception const &e) {
        SPDLOG_ERROR("config json parse failed: {}", e.what());
        throw;
    }

    OstStart(ostmdConfigFile.c_str());
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

#endif
