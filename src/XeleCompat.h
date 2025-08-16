#pragma once

#include <cstdint>

namespace XeleCompat
{
#pragma pack(push)
#pragma pack(1)

enum MarketType
{
    SSE = 1,
    SZE = 2,
};

enum MsgTypeSz
{
    MSG_TYPE_SNAPSHOT_SZ          =           0x01,   //快照
    MSG_TYPE_BEST_ORDERS_SZ       =           0x02,   //订单明细，最多揭示50笔
    MSG_TYPE_INDEX_SZ             =           0x03,   //指数行情
    MSG_TYPE_TRADE_SZ             =           0x04,   //逐笔成交
    MSG_TYPE_ORDER_SZ             =           0x05,   //逐笔委托
    MSG_TYPE_AFTER_SNAP_SZ        =           0x06,   //深交盘后快照
    MSG_TYPE_BLOCKTRADE_SZ        =           0x07,   //深交盘后定价大宗交易
    MSG_TYPE_BOND_TRADE_SZ        =           0x08,   //债券逐笔成交
    MSG_TYPE_BOND_BLOCKTRADE_SZ   =           0x09,   //债券大额逐笔成交
    MSG_TYPE_BOND_ORDER_SZ        =           0x0A,   //债券逐笔委托
    MSG_TYPE_BOND_BLOCKORDER_SZ   =           0x0B,   //债券大额逐笔委托
    MSG_TYPE_BOND_SNAPSHOT_SZ     =           0x0C,   //债券快照
    MSG_TYPE_BOND_BEST_ORDERS_SZ  =           0x0D,   //订单明细，最多揭示50笔
    MSG_TYPE_BOND_BID_TRADE_SZ    =           0x11,   //债券竞买逐笔成交
    MSG_TYPE_BOND_BID_ORDER_SZ    =           0x12,   //债券竞买逐笔委托

    MSG_TYPE_ORDER_BOOK_SNAP_SZ   =           0x22,   //深交订单簿行情

    MSG_TYPE_SNAPSHOT_L1_SZ       =           0x65,   //L1快照
    MSG_TYPE_INDEX_L1_SZ          =           0x67,   //L1指数行情
    MSG_TYPE_AFTER_SNAP_L1_SZ     =           0x6A,   //L1深交盘后快照
    MSG_TYPE_BLOCKTRADE_L1_SZ     =           0x6B,   //L1深交盘后定价大宗交易
    MSG_TYPE_BOND_SNAPSHOT_L1_SZ  =           0x70,   //L1债券快照
};

enum MsgTypeSse
{
    MSG_TYPE_SNAPSHOT_SSE          =       0x01,   //上交快照
    MSG_TYPE_BEST_ORDERS_SSE       =       0x02,   //订单明细，最多揭示50笔
    MSG_TYPE_INDEX_SSE             =       0x03,   //指数行情
    MSG_TYPE_TRADE_SSE             =       0x04,   //逐笔成交
    MSG_TYPE_ORDER_SSE             =       0x05,   //逐笔委托

    MSG_TYPE_AFTER_SNAP_SSE        =       0x07,   //上交盘后快照
    MSG_TYPE_AFTER_TRADE_SSE       =       0x08,   //上交盘后逐笔成交
    MSG_TYPE_BOND_SNAPSHOT_SSE     =       0x0C,   //上交债券快照
    MSG_TYPE_BOND_TICK_SSE         =       0x0D,   //上交债券逐笔

    MSG_TYPE_STATIC_SSE            =       0x0F,   //上交静态信息
    MSG_TYPE_TICK_MERGE_SSE        =       0x11,   //上交逐笔合并消息
    MSG_TYPE_ORDER_BOOK_SNAP_SSE   =       0x22,   //上交订单簿行情

    MSG_TYPE_SNAPSHOT_L1_SSE       =       0x65,   //上交L1快照
    MSG_TYPE_INDEX_L1_SSE          =       0x67,   //L1指数行情
    MSG_TYPE_AFTER_SNAP_L1_SSE     =       0x6B,   //上交盘后快照
    MSG_TYPE_BOND_SNAPSHOT_L1_SSE  =       0x70,   //L1上交债券快照
};

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

struct CommonHeadSz
{
    uint8_t messageType;
    uint32_t sequence;
    uint8_t exchangeID;
    char securityID[9];
};

#pragma pack(pop)
}
