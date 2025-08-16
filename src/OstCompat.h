#include <cstdint>

namespace OstCompat
{
#pragma pack(push)
#pragma pack(1)

struct sse_hpf_tick_64 {
    uint32_t                        m_tick_index;
    uint32_t                        m_channel_id;
    char                            m_symbol_id[9];
    uint8_t                         m_secu_type;
    uint8_t                         m_sub_secu_type;
    uint32_t                        m_tick_time;
    char                            m_tick_type;              // 类型， A新增订单， D删除订单， S产品状态订单， T成交
    uint64_t                        m_buy_order_no;           // 买方订单号，对产品状态订单无意义
    uint64_t                        m_sell_order_no;          // 卖方订单号，对产品状态订单无意义
    uint32_t                        m_order_price;            // 价格，对产品状态订单无意义
    uint64_t                        m_qty;                    // 数量（手），对产品状态订单无意义
    uint64_t                        m_trade_money;            // 成交金额（元），仅适用于成交消息
    char                            m_side_flag;
    uint8_t                         m_instrument_status;      // 标的状态，仅适用于产品状态订单
    char                            m_reserved[2];
};

struct sze_hpf_pkt_head_64
{
    // uint32_t                             m_sequence;
    // uint16_t                             m_tick1;
    // uint16_t                             m_tick2;
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

struct sze_hpf_order_pkt_64
{
    sze_hpf_pkt_head_64                 m_header;
    uint32_t                            m_px;
    uint64_t                            m_qty;
    char                                m_side;
    char                                m_order_type;
    char                                m_reserved[7];
};

struct sze_hpf_exe_pkt_64
{
    sze_hpf_pkt_head_64                 m_header;
    int64_t                             m_bid_app_seq_num;
    int64_t                             m_ask_app_seq_num;
    uint32_t                            m_exe_px;
    uint64_t                            m_exe_qty;
    char                                m_exe_type;
};

#pragma pack(pop)
}
