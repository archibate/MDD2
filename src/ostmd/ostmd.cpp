#include "ostmd.h"
#include "../MDS.h"
#include "config.h"
#include "sse_hpf_quote.h"
#include "sze_hpf_quote.h"
#include "efvi_receive_base.h"
#include "config_file_reader.h"
#include <spdlog/spdlog.h>
#include <functional>
#include <memory>

namespace
{

template <class Quote>
class UdpQuoteClass : public udp_quote_base
{
public:
    void process_packet(const char *data, const unsigned int &len) override
    {
        if (len < sizeof(Quote)) [[unlikely]] {
            return;
        }
        const char *end = data + len - sizeof(Quote) + 1;
        do {
            auto quote = (Quote *)data;
            MDS::handleOstQuote(*quote);
            data += sizeof(Quote);
        } while (data < end);
    }
};

template <>
class UdpQuoteClass<sze_hpf_pkt_head> : public udp_quote_base
{
public:
    void process_packet(const char *data, const unsigned int &len) override
    {
        if (len < sizeof(sze_hpf_order_pkt)) [[unlikely]] {
            return;
        }
        const char *end = data + len - sizeof(sze_hpf_order_pkt) + 1;
        while (true) {
            auto quote = (sze_hpf_pkt_head *)data;
            static_assert(sizeof(sze_hpf_exe_pkt) - sizeof(sze_hpf_order_pkt) == 8);
            static_assert(sze_msg_type_trade - sze_msg_type_order == 1);
            data += 1 << (quote->m_message_type - (sze_msg_type_order - 3));
            // if (quote->m_message_type == sze_msg_type_order) {
            //     data += sizeof(sze_hpf_order_pkt);
            // } else {
            //     assert(quote->m_message_type == sze_msg_type_trade);
            //     data += sizeof(sze_hpf_exe_pkt);
            // }
            if (data >= end) {
                break;
            }
            MDS::handleOstQuote(*quote);
        }
    }
};

std::vector<std::shared_ptr<udp_quote_base>> udpQuotes;

}

void OstStart(const char *configFile, int ticksBindCpu)
{
    SPDLOG_DEBUG("ostmd loading config file [{}]", configFile);
    ConfigFileReader config;
    config.LoadConfigFile(configFile);

    struct ChannelInfo
    {
        const char *name;
        std::function<std::shared_ptr<udp_quote_base>()> factory;
    };

    static ChannelInfo channels[] = {
#if SH
        {"SHJQ_TICK_MERGE", std::make_shared<UdpQuoteClass<sse_hpf_tick>>},
        {"SHJQ_SSE_SNAPSHOT", std::make_shared<UdpQuoteClass<sse_hpf_lev2>>},
        {"SHJQ_SZE_SNAPSHOT", std::make_shared<UdpQuoteClass<sze_hpf_lev2_pkt>>},
#endif
#if SZ
        {"NFZX_ORDER_EXE", std::make_shared<UdpQuoteClass<sze_hpf_pkt_head>>},
        {"NFZX_SSE_SNAPSHOT", std::make_shared<UdpQuoteClass<sse_hpf_lev2>>},
        {"NFZX_SZE_SNAPSHOT", std::make_shared<UdpQuoteClass<sze_hpf_lev2_pkt>>},
#endif
    };

    for (auto const &ch: channels) {
        sock_udp_param udp_param;
        config.GetSockUdpParam(udp_param, ch.name);
        if (ticksBindCpu != -1) {
            udp_param.m_cpu_id = ticksBindCpu;
            ticksBindCpu = -1;
        }

        if (!udp_param.m_open) {
            continue;
        }
        SPDLOG_DEBUG("ostmd initialzing channel {}", ch.name);
        auto quote = ch.factory();
        if (!quote->init(udp_param)) {
            SPDLOG_ERROR("ostmd channel {} initialize failed", ch.name);
            std::terminate(); // throw std::runtime_error("ostmd channel initialize failed");
        }
        udpQuotes.push_back(std::move(quote));
    }

    SPDLOG_DEBUG("ostmd initialized");
}

void OstStop()
{
    SPDLOG_DEBUG("ostmd stopping");
    for (auto const &quote: udpQuotes) {
        quote->close();
    }
    SPDLOG_DEBUG("ostmd stopped");
    udpQuotes.clear();
}

