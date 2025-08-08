/*!
 * \file sze_hpf_quote.h
 * \date 2023/07/25 13:48
 *
 * \author zhangxuebing
 * Contact: xuebing.zhang@orientfutures.com
 *
 * \brief 行情输出结构
 */
#pragma once
#include <stdint.h>

const uint8_t  sze_exchange_id = 101;

const uint8_t  sze_msg_type_snap    = 21;
const uint8_t  sze_msg_type_idx     = 22;
const uint8_t  sze_msg_type_order   = 23;
const uint8_t  sze_msg_type_trade   = 24;

#pragma pack(push, 1)
struct px_qty_unit
{
    uint32_t                            m_price;
    uint64_t                            m_qty;
};

struct sze_hpf_pkt_head
{
    uint32_t                             m_sequence;
    uint16_t                             m_tick1;
    uint16_t                             m_tick2;
    uint8_t                              m_message_type;
    uint8_t                              m_security_type;
    uint8_t                              m_sub_security_type;
    char                                 m_symbol[9];
    uint8_t                              m_exchange_id;
    uint64_t                             m_quote_update_time;
    uint16_t                             m_channel_num;
    int64_t                              m_sequence_num;
    int32_t                              m_md_stream_id;
};

struct sze_hpf_lev2_pkt
{
    sze_hpf_pkt_head                    m_header;
    uint8_t                             m_trading_phase_code;
    int64_t                             m_trades_num;
    uint64_t                            m_total_quantity_trade;
    uint64_t                            m_total_value_trade;
    uint32_t                            m_previous_close_price;
    uint32_t                            m_last_price;
    uint32_t                            m_open_price;
    uint32_t                            m_day_high;
    uint32_t                            m_day_low;
    uint32_t                            m_today_close_price;
    uint32_t                            m_total_bid_weighted_avg_px;
    uint64_t                            m_total_bid_qty;
    uint32_t                            m_total_ask_weighted_avg_px;
    uint64_t                            m_total_ask_qty;
    uint32_t                            m_lpv;
    uint32_t                            m_iopv;
    uint32_t                            m_upper_limit_price;
    uint32_t                            m_lower_limit_price;
    uint32_t                            m_open_interest;
    px_qty_unit                         m_bid_unit[10];
    px_qty_unit                         m_ask_unit[10];
};

struct sze_hpf_idx_pkt
{
    sze_hpf_pkt_head                    m_header;
    int64_t                             m_trades_num;
    uint64_t                            m_total_quantity;
    uint64_t                            m_total_value;
    uint32_t                            m_last_price;
    uint32_t                            m_previous_close_price;
    uint32_t                            m_open_price;
    uint32_t                            m_day_high;
    uint32_t                            m_day_low;
    uint32_t                            m_close_price;
    char                                m_reserved[5];
};

struct sze_hpf_order_pkt
{
    sze_hpf_pkt_head                    m_header;
    uint32_t                            m_px;
    uint64_t                            m_qty;
    char                                m_side;
    char                                m_order_type;
    char                                m_reserved[7];
};

struct sze_hpf_exe_pkt
{
    sze_hpf_pkt_head                    m_header;
    int64_t                             m_bid_app_seq_num;
    int64_t                             m_ask_app_seq_num;
    uint32_t                            m_exe_px;
    uint64_t                            m_exe_qty;
    char                                m_exe_type;
};

struct sze_hpf_pkt_resend
{
    sze_hpf_pkt_head                    m_header;
    uint16_t                            m_channel_no;
    int64_t                             m_beg_seq_num;
    int64_t                             m_end_seq_num;
    uint8_t                             m_resend_status;
    char                                m_resv[2];
};

#pragma pack(pop)


