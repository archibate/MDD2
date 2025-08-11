#pragma once

#include "config.h"
#include <cstdint>
#if REPLAY
#include "L2/Stat.h"
#include "L2/Tick.h"
#include "L2/Snap.h"
#elif NE
#include <nesc/Const.h>
#include <nesc/SseMdStruct.h>
#include <nesc/SzeMdStruct.h>
#elif OST
#include "ostmd/sse_hpf_quote.h"
#include "ostmd/sze_hpf_quote.h"
#include <UTApiStruct.h>
#endif


namespace MDS
{

#if REPLAY
using Stat = L2::Stat;
using Tick = L2::Tick;
using Snap = L2::Snap;
#elif NE

// #if COMPAT_TICK
namespace NescCompat
{
#pragma pack(push)
#pragma pack(1)

struct TickMergeSse
{
    uint8_t  messageType;    //消息类型，竞价逐笔合并为0x11
    uint32_t sequence;       //udp输出包序号，从1开始
    uint8_t  exchangeID;     //交易所id，上交所：1，深交所：2
    char     securityID[9];  //证券代码
    char     tickType;       //类型：A=新增委托订单 D=删除委托订单 S=产品状态订单 T=成交
    /*
    订单状态
    为新增或删除委托订单时：
      0-买单  1-卖单
    为产品状态订单时；
      0=启动 1=开市集合竞价 2=连续自动撮合 3=停牌 4=收盘集合竞价 5=闭市 6=交易结束
    为成交时：
      0-外盘，主动买 1-内盘，主动卖 2-未知
    */
    char     tickBSFlag;

    uint64_t bizIndex;       //逐笔序号,从1开始，按channel连续
    uint32_t channelNo;      //频道代码
    uint32_t tickTime;       //订单或成交时间，精确到10毫秒，格式HHMMSSmm，例如14302501
    uint64_t buyOrderNo;     //买方订单号
    uint64_t sellOrderNo;    //卖方订单号
    uint32_t price;          //价格，实际值除以1000
    uint64_t qty;            //数量，实际值除以1000
    char     padding[3];        //填充至64字节
    // uint64_t tradeMoney;     //TickType=T成交时，代表成交金额，实际值除以100000；
    // //TickType=A新增委托时，代表已成交委托数量，实际值除以1000，其他无意义
    // uint32_t msgSeqID;       //消息序号（仅定位调试使用）
    // char     resv[7];        //保留字段
};

struct TradeSz
{
    uint8_t  messageType;               //消息类型，逐笔成交为0x4
    uint32_t sequence;                  //udp输出包序号，从1开始
    uint8_t  exchangeID;                //交易所id，上交所：1，深交所：2
    char     securityID[9];             //证券代码
    uint8_t  execType;                  //成交类别：0x1--撤销；0x2--成交
    uint64_t applSeqNum;                //消息记录号
    uint64_t transactTime;              //成交时间，格式YYYYMMDDHHMMSSmmm，精确到毫秒。
    //示例：20190411102939120表示2019年04月11日10点29分39秒120毫秒
    uint64_t tradePrice;                //成交价格，实际值除以10000
    uint64_t tradeQty;                  //成交数量，实际值除以100

    uint64_t bidapplSeqnum;             //买方委托索引
    uint64_t offerapplSeqnum;           //卖方委托索引

    // uint16_t channelNo;                 //频道代码
    // uint16_t mdstreamid;                //行情类别
};

struct OrderSz
{
    uint8_t  messageType;               //消息类型，逐笔委托为0x5
    uint32_t sequence;                  //udp输出包序号，从1开始
    uint8_t  exchangeID;                //交易所id，上交所：1，深交所：2
    char     securityID[9];             //证券代码
    char rsvd;                          //保留字段
    char side;                          //买卖方向：1=买，2=卖，G=借入，F=出借
    char  orderType;                    //订单类别：1=市价，2=限价，U=本方最优
    uint64_t applSeqNum;                //消息记录号
    uint64_t transactTime;              //委托时间，格式YYYYMMDDHHMMSSmmm，精确到毫秒。
    //示例：20190411102939120表示2019年04月11日10点29分39秒120毫秒
    uint64_t price;                     //价格，实际值除以10000
    uint64_t qty;                       //数量，实际值除以100

    uint16_t channelNo;                 //频道代码
    uint16_t mdstreamid;                //行情类别
};

#pragma pack(pop)
}
// #else
// namespace NescCompat = NescForesight;
// #endif


struct Stat
{
    NescForesight::MarketType marketType;
    union
    {
        NescForesight::StaticSseInfo staticSseInfo;
        NescForesight::StaticSzInfo staticSzInfo;
    };
};

#if SH
struct Tick
{
    union
    {
        NescCompat::TickMergeSse tickMergeSse;
        uint8_t messageType;
    };
};
#endif

#if SZ
struct Tick
{
    union
    {
        NescCompat::TradeSz tradeSz;
        NescCompat::OrderSz orderSz;
        struct
        {
#pragma pack(push)
#pragma pack(1)
            uint8_t messageType;
            uint32_t sequence;
            uint8_t exchangeID;
            char securityID[9];
#pragma pack(pop)
        };
    };
};
#endif

struct Snap
{
    NescForesight::MarketType marketType;
    union {
        NescForesight::MarketDataSnapshotSse const *snapshotSse;
        NescForesight::MarketDataSnapshotSz const *snapshotSz;
    };
};

// #if COMPAT_TICK
static_assert(sizeof(Tick) == 64);
// #endif

#elif OST

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

struct Stat
{
    CUTDepthMarketDataField depthMarketData;
};

#if SZ
struct Tick
{
    union
    {
        sze_hpf_pkt_head_64 head;
        sze_hpf_order_pkt_64 order;
        sze_hpf_exe_pkt_64 exe;
    };
};
#endif

#if SH
struct Tick
{
    sse_hpf_tick_64 tick;
};
#endif

struct Snap
{
    bool isSz;
    union {
        sse_hpf_lev2 *sseLev2;
        sze_hpf_lev2_pkt *szeLev2;
    };
};

// #if COMPAT_TICK
static_assert(sizeof(Tick) == 64);
// #endif

void handleOstQuote(sse_hpf_tick &q);
void handleOstQuote(sze_hpf_pkt_head &q);
void handleOstQuote(sse_hpf_lev2 &q);
void handleOstQuote(sze_hpf_lev2_pkt &q);

#endif

void subscribe(int32_t const *stocks, int32_t n);
void start(const char *config);
void startReceive();
void stop();
void requestStop();
bool isFinished();
bool isStarted();

#if REPLAY
extern double g_timeScale;
#endif

}
