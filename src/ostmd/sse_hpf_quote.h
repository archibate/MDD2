/*!
 * \file sse_hpf_quote.h
 * \date 2023/07/25 13:40
 *
 * \author zhangxuebing
 * Contact: xuebing.zhang@orientfutures.com
 *
 * \brief 行情输出结构
 */
#pragma once
#include <stdint.h>

const uint8_t  sse_exchange_id = 100;

const uint8_t  sse_msg_type_idx		  = 33;
const uint8_t  sse_msg_type_option	  = 35;
const uint8_t  sse_msg_type_tick	  = 36;
const uint8_t  sse_msg_type_snap	  = 39;
const uint8_t  sse_msg_type_bond_tick = 61;

#pragma pack(push, 1)
struct sse_hpf_head {
	uint32_t                     m_seq_num;
	uint32_t                     m_reserved;        /// 软件此值填0
	uint8_t                      m_msg_type;
	uint16_t                     m_msg_len;         /// 包括此消息头的长度，lev2_full=440， idx=80，timesale=88
	uint8_t                      m_exchange_id;
	uint16_t                     m_data_year;
	uint8_t                      m_data_month;
	uint8_t                      m_data_day;
	uint32_t                     m_send_time;
	uint8_t                      m_category_id;
	uint32_t                     m_msg_seq_id;
	uint8_t                      m_seq_lost_flag;   /// 1=有丢包，0=没有丢包
};


struct sse_hpf_idx {
	sse_hpf_head                 m_head;

	uint32_t                     m_update_time;
	char                         m_symbol_id[9];
	uint8_t                      m_secu_type;            /// 0=指数， 10=其他    
	uint32_t                     m_prev_close;
	uint32_t                     m_open_price;
	uint64_t                     m_traded_value;
	uint32_t                     m_day_high;
	uint32_t                     m_day_low;
	uint32_t                     m_last_price;
	uint64_t                     m_traded_volume;
	uint32_t                     m_close_price;
};

struct sse_hpf_tick {
	sse_hpf_head                    m_head;
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
	char                            m_reserved[8];
};

struct px_level {
	uint32_t                     resv;
	uint32_t                     m_px;
	uint64_t                     m_qty;
};

struct sse_hpf_lev2 {
	sse_hpf_head                 m_head;

	uint32_t                     m_update_time;
	char                         m_symbol_id[9];
	uint8_t                      m_secu_type;
	uint8_t                      m_update_type;
	uint8_t                      m_reserved;
	uint32_t                     m_prev_close;
	uint32_t                     m_open_price;
	uint32_t                     m_day_high;
	uint32_t                     m_day_low;
	uint32_t                     m_last_price;
	uint32_t                     m_close_price;
	uint8_t                      m_instrument_status;
	uint8_t                      m_trading_status;
	uint16_t                     m_reserved2;
	uint32_t                     m_trade_number;
	uint64_t                     m_trade_volume;
	uint64_t                     m_trade_value;

	uint64_t                     m_total_qty_bid;
	uint32_t                     m_weighted_avg_px_bid;
	uint64_t                     m_total_qty_ask;
	uint32_t                     m_weighted_avg_px_ask;
	uint32_t                     m_yield_to_maturity;
	uint8_t                      m_depth_bid;
	uint8_t                      m_depth_ask;

	px_level                     m_bid_px[10];
	px_level                     m_ask_px[10];
};

struct sse_hpf_lev2_extend {
	sse_hpf_head                 m_head;

	uint32_t                     m_update_time;
	char                         m_symbol_id[9];
	uint8_t                      m_secu_type;
	uint8_t                      m_update_type;
	uint8_t                      m_reserved;
	uint32_t                     m_prev_close;
	uint32_t                     m_open_price;
	uint32_t                     m_day_high;
	uint32_t                     m_day_low;
	uint32_t                     m_last_price;
	uint32_t                     m_close_price;
	uint8_t                      m_instrument_status;
	uint8_t                      m_trading_status;
	uint16_t                     m_reserved2;
	uint32_t                     m_trade_number;
	uint64_t                     m_trade_volume;
	uint64_t                     m_trade_value;

	uint64_t                     m_total_qty_bid;
	uint32_t                     m_weighted_avg_px_bid;
	uint64_t                     m_total_qty_ask;
	uint32_t                     m_weighted_avg_px_ask;
	uint32_t                     m_yield_to_maturity;
	uint8_t                      m_depth_bid;
	uint8_t                      m_depth_ask;

	px_level                     m_bid_px[10];
	px_level                     m_ask_px[10];
	uint32_t                     m_iopv;
	uint32_t                     m_reserved3;
};
struct sse_hpf_bond_tick {
	sse_hpf_head                    m_head;
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
	char                            m_reserved[8];
};

struct sse_option_unit {
	uint32_t            m_px;
	uint64_t            m_qty;
};


struct sse_hpf_option_pkt {
	sse_hpf_head            m_header;

	uint32_t                m_update_time;
	uint8_t                 m_secu_type;
	char                    m_symbol_id[9];
	uint8_t                 m_update_type;
	uint8_t                 m_reserved1;
	uint32_t                m_prev_close;
	uint32_t                m_prev_settle_px;
	uint32_t                m_open_price;
	uint32_t                m_day_high;
	uint32_t                m_day_low;
	uint32_t                m_last_price;
	uint32_t                m_close_price;
	uint32_t                m_settle_price;
	uint32_t                m_dynamic_price;
	uint8_t                 m_reserved2[3];
	uint8_t                 m_trading_status;
	uint64_t                m_open_interest;
	uint32_t                m_trade_number;
	uint64_t                m_trade_volume;
	uint64_t                m_trade_value;
	uint8_t                 m_depth_bid;
	uint8_t                 m_depth_ask;

	sse_option_unit         m_bid_unit[5];
	sse_option_unit         m_ask_unit[5];
};

#pragma pack(pop)