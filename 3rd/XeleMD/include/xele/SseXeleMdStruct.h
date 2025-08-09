/*****************************************************************************
艾科朗克上交所行情结构体定义，字节序为小端
******************************************************************************/
#ifndef SSE_XELE_MD_STRUCT_H
#define SSE_XELE_MD_STRUCT_H

#include <stdint.h>


#define MSG_TYPE_SNAPSHOT               0x1   //上交快照
#define MSG_TYPE_BEST_ORDERS            0x2   //订单明细，最多揭示50笔
#define MSG_TYPE_INDEX                  0x3   //指数行情
#define MSG_TYPE_TRADE                  0x4   //逐笔成交
#define MSG_TYPE_ORDER                  0x5   //逐笔委托

#define MSG_TYPE_AFTER_SNAP_SSE         0x7   //上交盘后快照 
#define MSG_TYPE_AFTER_TRADE_SSE        0x8   //上交盘后逐笔成交
#define MSG_TYPE_BOND_SNAPSHOT          0x0C   //上交债券快照
#define MSG_TYPE_BOND_TICK              0x0D   //上交债券逐笔
#define MSG_TYPE_OPTION_SSE             0x0E  //上交期权

#define MSG_TYPE_ETF_SNAPSHOT           0x10   //上交ETF统计
#define MSG_TYPE_TICK_MERGE             0x11   //上交逐笔合并消息
#define MSG_TYPE_STATIC_SSE             0x0F   //上交静态信息


#define SNAPSHOT_LEVEL                  10
#define BEST_ORDERS_LEVEL               50



#pragma pack(push)
#pragma pack(1)

struct BidAskPriceQtySse {
  uint32_t price;    //申买、申卖价格，实际值除以1000
  uint64_t qty;      //申买、申卖数量，实际值除以1000
};

/*
 * 行情快照
 */
struct MarketDataSnapshotSse {
  uint8_t  messageType;    //消息类型，快照为0x1
  uint32_t sequence;       //udp输出包序号，从1开始
  uint8_t  exchangeID;     //交易所id，上交所：1，深交所：2
  char     securityID[9];  //证券代码
  uint8_t  resv[2];        //保留字段
  /*
  产品所处的交易阶段代码
  该字段为8位字符串，左起每位表示特定的含义，无定义则填空格。
  第1位：‘S’表示启动（开市前）时段，‘C’表示开盘集合竞价时段，‘T’表示连续交易时段，‘E’表示闭市时段，‘P’表示产品停牌，
        ‘M’表示可恢复交易的熔断时段（盘中集合竞价），‘N’表示不可恢复交易的熔断时段（暂停交易至闭市），‘U’表示收盘集合竞价时段。
  第2位：‘0’表示此产品不可正常交易，‘1’表示此产品可正常交易，无意义填空格。
  第3位：‘0’表示未上市，‘1’表示已上市。
  第4位：‘0’表示此产品在当前时段不接受订单申报，‘1’ 表示此产品在当前时段可接受订单申报。无意义填空格
  */
  char tradingPhaseCode[8];
  /*
   * 当前品种交易状态：
   * - START：启动
   * - OCALL：开市集合竞价
   * - TRADE：连续自动撮合
   * - SUSP：停牌
   * - CCALL：收盘集合竞价
   * - CLOSE：闭市，自动计算闭市价格
   * - ENDTR：交易结束
   * 详见上交所接口说明文档
   */
  char instrumentStatus[6];
  uint8_t resv2[5];                          //保留字段
  uint32_t timeStamp;                        //最新订单时间，格式HHMMSS，例如143025 表示 14:30:25
  uint32_t preClosePrice;                    //昨收价（来源消息头)，实际值除以1000
  uint32_t numTrades;                        //总成交笔数
  uint64_t totalVolumeTrade;                 //总成交量，实际值除以1000
  uint64_t totalValueTrade;                  //总成交金额，实际值除以100000
  uint32_t lastPrice;                        //最近价，实际值除以1000
  uint32_t openPrice;                        //开盘价，实际值除以1000
  uint32_t closePrice;                       //今日收盘价，实际值除以1000
  uint32_t highPrice;                        //最高价，实际值除以1000
  uint32_t lowPrice;                         //最低价，实际值除以1000
  uint64_t resv3;                            //保留字段
  uint64_t resv4;                            //保留字段
  uint32_t bidAvgPrice;                      //买入委托加权平均价，实际值除以1000
  uint64_t bidTotalQty;                      //买入委托总数量，实际值除以1000
  uint32_t askAvgPrice;                      //卖出委托加权平均价，实际值除以1000
  uint64_t askTotalQty;                      //卖出委托总数量，实际值除以1000
  BidAskPriceQtySse bidInfo[SNAPSHOT_LEVEL]; //申买信息
  BidAskPriceQtySse askInfo[SNAPSHOT_LEVEL]; //申卖信息
  uint32_t msgSeqID;                         //消息序号
};


/*
 * 订单明细，最多揭示50笔
 */
struct BestOrdersSse {
  uint8_t  messageType;                //消息类型，订单明细为0x2
  uint32_t sequence;                   //udp输出包序号，从1开始
  uint8_t  exchangeID;                 //交易所id，上交所：1，深交所：2
  char     securityID[9];              //证券代码
  char     recv[3];                    //保留字段
  uint32_t msgSeqID;                   //消息序号
  uint8_t  side;                       //买卖标识：买：1，卖：2
  uint8_t  number;                     //明细个数
  uint32_t timeStamp;                  //最新订单时间，格式HHMMSS，例如143025 表示 14:30:25                                     //示例：20190411102939120表示2019年04月11日10点29分39秒120毫秒
  uint32_t price;                      //委托价格，实际值除以1000
  uint64_t orders;                     //申买/卖数量，实际值除以1000
  uint64_t volume[50];                 //委托数量，最多揭示50笔，实际值除以1000
};


/*
 * 指数行情
 */
struct IndexSse {
  uint8_t  messageType;               //消息类型，指数为0x3
  uint32_t sequence;                  //udp输出包序号，从1开始
  uint8_t  exchangeID;                //交易所id，上交所：1，深交所：2
  char     securityID[9];             //证券代码
  uint8_t  resv[5];                   //保留字段
  uint32_t timeStamp;                 //最新订单时间，格式HHMMSS，例如143025 表示 14:30:25
  uint32_t tradeTime;                 //成交时间，格式HHMMSS，例如143025 表示 14:30:25
  uint32_t msgSeqID;                  //消息序号
  uint64_t preClosePrice;             //前盘指数（来源扩展字段），实际值除以100000
  uint64_t openPrice;                 //开盘指数，实际值除以100000
  uint64_t lastPrice;                 //最新指数，实际值除以100000
  uint64_t highPrice;                 //最高指数，实际值除以100000
  uint64_t lowPrice;                  //最低指数，实际值除以100000
  uint64_t closePrice;                //今日收盘指数，实际值除以100000
  uint64_t totalVolume;               //成交总量，实际值除以100000
  uint64_t totalValue;                //成交总金额，实际值除以10
};


/*
 * 上交逐笔成交
 */
struct TradeSse {
  uint8_t  messageType;               //消息类型，逐笔成交为0x4
  uint32_t sequence;                  //udp输出包序号，从1开始
  uint8_t  exchangeID;                //交易所id，上交所：1，深交所：2
  char     securityID[9];             //证券代码
  char     tradeBSFlag;               //内外盘标志 ：B-外盘，主动买，S-内盘，主动买，N-未知
  uint32_t tradeIndex;                //成交序号
  uint32_t channelNo;                 //成交通道
  uint32_t transactTime;              //成交时间，格式HHMMSSmm，精确到10毫秒，例如10031945代表10点03分19秒450毫秒
  uint32_t tradePrice;                //成交价格，实际值除以1000
  uint64_t tradeQty;                  //成交数量，实际值除以1000
  uint64_t tradeMoney;                //成交金额，实际值除以100000
  uint64_t bidapplSeqnum;             //买方委托索引
  uint64_t offerapplSeqnum;           //卖方委托索引
  uint64_t bizIndex;                  //业务序列号，与委托统一编号，从1开始，按Channel连续
  uint32_t msgSeqID;                  //消息序号（仅定位调试使用）
  char     resv2[4];                  //保留字段
};


/*
 * 上交所逐笔委托
 */
struct OrderSse {
  uint8_t  messageType;               //消息类型，逐笔委托为0x5
  uint32_t sequence;                  //udp输出包序号，从1开始
  uint8_t  exchangeID;                //交易所id，上交所：1，深交所：2
  char     securityID[9];             //证券代码
  char     orderType;                 //订单类型：A=委托订单(增加） D=委托订单(删除)
  char     orderBSFlag;               //订单状态: 0=买单 1=卖单            
  uint32_t orderIndex;                //委托序号，从1开始，按Channel连续
  uint32_t channelNo;                 //频道代码
  uint32_t transactTime;              //委托时间，格式HHMMSSmm，精确到10毫秒，例如10031945代表10点03分19秒450毫秒
  uint64_t orderNo;                   //原始订单号
  uint32_t price;                     //价格，实际值除以1000
  uint64_t balance;                   //剩余委托量，实际值除以1000
  uint64_t bizIndex;                  //业务序列号，与成交统一编号，从1开始，按Channel连续
  uint32_t msgSeqID;                  //消息序号（仅定位调试使用）
  char     resv[3];                   //保留字段
};

/*
 * 上交盘后逐笔成交
 */
struct AfterTradeSse {
  uint8_t  messageType;               //消息类型，上交盘后逐笔成交为0x8
  uint32_t sequence;                  //udp输出包序号，从1开始
  uint8_t  exchangeID;                //交易所id，上交所：1，深交所：2
  char     securityID[9];             //证券代码
  char     tradeBSFlag;               //内外盘标志 ：B-外盘，主动买，S-内盘，主动买，N-未知
  uint32_t tradeIndex;                //上交所：成交序号
  uint32_t channelNo;                 //成交通道
  uint32_t transactTime;              //成交时间，格式HHMMSSmm，精确到10毫秒，例如10031945代表10点03分19秒450毫秒
  uint32_t tradePrice;                //成交价格，实际值除以1000
  uint64_t tradeQty;                  //成交数量，实际值除以1000
  uint64_t tradeMoney;                //成交金额，实际值除以100000
  uint64_t bidapplSeqnum;             //买方委托索引
  uint64_t offerapplSeqnum;           //卖方委托索引
  uint32_t msgSeqID;                  //消息序号
  char     resv2[4];                  //保留字段
};

/*
*上交盘后行情快照
*/
struct AfterSnapshotSse {
  uint8_t  messageType;              //消息类型，上交快照为0x7
  uint32_t sequence;                 //udp输出包序号，从1开始
  uint8_t  exchangeID;               //交易所id，上交所：1，深交所：2
  char     securityID[9];            //证券代码
  uint8_t  resv;                     //保留字段
  /*
   * 当前品种交易状态：
   * - START：启动
   * - OCALL：开市集合竞价
   * - TRADE：连续自动撮合
   * - SUSP：停牌
   * - CCALL：收盘集合竞价
   * - CLOSE：闭市，自动计算闭市价格
   * - ENDTR：交易结束
   * 详见上交所接口说明文档
   */
  char     instrumentStatus[6];
  uint32_t timeStamp;                //最新订单时间，格式HHMMSS，例如143025 表示 14:30:25
  uint32_t closePrice;               //今日收价，实际值除以1000
  uint32_t numTrades;                //总成交笔数
  uint64_t totalVolumeTrade;         //总成交量，实际值除以1000
  uint64_t totalValueTrade;          //总成交金额，实际值除以100000
  uint64_t totalBidQty;              //买入委托总数量，实际值除以1000
  uint64_t totalAskQty;              //卖出委托总数量，实际值除以1000
  uint32_t withdrawBuyNumber;        //买入撤单笔数
  uint64_t withdrawBuyAmount;        //买入撤单数量，实际值除以1000
  uint32_t withdrawSellNumber;       //卖出撤单笔数
  uint64_t withdrawSellAmount;       //卖出撤单数量，实际值除以1000
  uint32_t msgSeqID;                 //消息序号
  uint32_t noBidLevel;               //买盘价位数量
  uint32_t noOfferLevel;             //卖盘价位数量

  uint64_t bidQty;                   //一档申买量，实际值除以1000
  uint64_t askQty;                   //一档申卖量，实际值除以1000
  uint64_t bidOrderNum;              //买一价申买量揭示笔数
  uint64_t askOrderNum;              //卖一价申卖量揭示笔数
};

/*
 * 债券快照
 */
struct BondSnapshotSse {
  uint8_t  messageType;                      //消息类型，债券快照为0x0C
  uint32_t sequence;                         //udp输出包序号，从1开始
  uint8_t  exchangeID;                       //交易所id，上交所：1，深交所：2
  char     securityID[9];                    //证券代码
  char     resv[2];                          //保留字段
  char instrumentStatus[6];                  //当前品种交易状态,详见上交所接口说明文档
  char resv2;                                //保留字段
  uint32_t timeStamp;                        //最新订单时间，精确到毫秒，格式HHMMSSmmm，例如143025001 表示 14:30:25:001
  uint32_t preClosePrice;                    //昨收价（来源消息头)，实际值除以1000
  uint32_t openPrice;                        //开盘价，实际值除以1000
  uint32_t highPrice;                        //最高价，实际值除以1000
  uint32_t lowPrice;                         //最低价，实际值除以1000
  uint32_t lastPrice;                        //最近价，实际值除以1000
  uint32_t closePrice;                       //今日收盘价，实际值除以1000
  uint32_t numTrades;                        //总成交笔数
  uint64_t totalVolumeTrade;                 //总成交量，实际值除以1000
  uint64_t totalValueTrade;                  //总成交金额，实际值除以100000
  uint64_t bidTotalQty;                      //买入委托总数量，实际值除以1000
  uint32_t bidAvgPrice;                      //买入委托加权平均价，实际值除以1000
  uint64_t askTotalQty;                      //卖出委托总数量，实际值除以1000
  uint32_t askAvgPrice;                      //卖出委托加权平均价，实际值除以1000
  uint32_t avgPrice;                         //加权平均价，实际值除以1000
  BidAskPriceQtySse bidInfo[SNAPSHOT_LEVEL];  //申买信息
  BidAskPriceQtySse askInfo[SNAPSHOT_LEVEL];  //申卖信息
  uint32_t msgSeqID;                          //消息序号    
};

/*
 * 债券逐笔
 */
struct BondTickSse {
  uint8_t  messageType;    //消息类型，债券逐笔为0x0D
  uint32_t sequence;       //udp输出包序号，从1开始
  uint8_t  exchangeID;     //交易所id，上交所：1，深交所：2
  char     securityID[9];  //证券代码
  char     tickType;       //类型：A=新增委托订单 D=删除委托订单 S=产品状态订单 T=成交
  /*
  订单状态
  为新增或删除委托订单时：
    0-买单  1-卖单
  为产品状态订单时；
    0=产品未上市 1=启动 2=开市集合竞价 3=连续自动撮合 4=停牌 5=闭市 6=交易结束
  为成交时：
    0-外盘，主动买 1-内盘，主动卖 2-未知
  */
  char     tickBSFlag;

  uint32_t tickIndex;      //逐笔序号,从1开始，按channel连续
  uint32_t channelNo;      //频道代码
  uint32_t tickTime;       //订单或成交时间，精确到毫秒，格式HHMMSSmmm，例如143025001 表示 14:30:25:001
  uint64_t buyOrderNo;     //买方订单号
  uint64_t sellOrderNo;    //卖方订单号
  uint32_t price;          //价格，实际值除以1000
  uint64_t qty;            //数量，实际值除以1000
  uint64_t tradeMoney;     //成交金额，仅适用于TickType=T时，实际值除以100000
  uint32_t msgSeqID;       //消息序号（仅定位调试使用）
  char     resv[3];        //保留字段
};



/*******
 * 上交期权结构体
 *******/
struct MarketDataOptionSse
{
  uint8_t    messageType;               //消息类型，期权为0x0E
  uint32_t   sequence;                  //udp输出包序号，从1开始
  uint8_t    exchangeID;                //交易所id，上交所：1，深交所：2
  char       securityID[9];             //证券代码
  char       resv;                      //保留字段

  uint64_t   tradeVolume;               //成交数量
  uint64_t   totalValueTraded;          //成交金额，实际值除以100
  uint64_t   totalLongPosition;         //持仓总量
  uint64_t   preClosePrice;             //昨日收盘价，实际值除以100000
  uint64_t   prevSetPrice;              //昨日结算价，实际值除以100000
  uint64_t   tradePrice;                //最新价，实际值除以100000
  uint64_t   openPrice;                 //今日开盘价，实际值除以100000
  uint64_t   closePrice;                //今日收盘价，实际值除以100000
  uint64_t   settlPrice;                //今日结算价，实际值除以100000
  uint64_t   highPrice;                 //最高价，实际值除以100000
  uint64_t   lowPrice;                  //最低价，实际值除以100000
  uint64_t   auctionPrice;              //动态参考价，实际值除以100000
  uint64_t   auctionQty;                //虚拟匹配数量
  char       tradingPhaseCode[8];       //指数实时阶段及标志

  uint64_t   bidPrice1;                 //申买价1，实际值除以100000
  uint64_t   bidVolume1;                //申买数量1
  uint64_t   bidPrice2;                 //申买价2，实际值除以100000
  uint64_t   bidVolume2;                //申买数量2
  uint64_t   bidPrice3;                 //申买价3，实际值除以100000
  uint64_t   bidVolume3;                //申买数量3
  uint64_t   bidPrice4;                 //申买价4，实际值除以100000
  uint64_t   bidVolume4;                //申买数量4
  uint64_t   bidPrice5;                 //申买价5，实际值除以100000
  uint64_t   bidVolume5;                //申买数量5
  uint64_t   askPrice1;                 //申卖价1，实际值除以100000
  uint64_t   askVolume1;                //申卖数量1
  uint64_t   askPrice2;                 //申卖价2，实际值除以100000
  uint64_t   askVolume2;                //申卖数量2
  uint64_t   askPrice3;                 //申卖价3，实际值除以100000
  uint64_t   askVolume3;                //申卖数量3
  uint64_t   askPrice4;                 //申卖价4，实际值除以100000
  uint64_t   askVolume4;                //申卖数量4
  uint64_t   askPrice5;                 //申卖价5，实际值除以100000
  uint64_t   askVolume5;                //申卖数量5
  uint32_t   msgSeqID;                  //fix消息序号
  uint32_t   timeStamp;                 //行情更新时间，格式HHMMSS，例如103045代表10点30分45秒
  uint16_t   timeMs;                    //行情更新时间毫秒部分，例如456代表456毫秒
};


/*
 * 竞价逐笔合并
 */
struct TickMergeSse {
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
  uint32_t tickTime;       //订单或成交时间，精确到毫秒，格式HHMMSSmmm，例如143025001 表示 14:30:25:001
  uint64_t buyOrderNo;     //买方订单号
  uint64_t sellOrderNo;    //卖方订单号
  uint32_t price;          //价格，实际值除以1000
  uint64_t qty;            //数量，实际值除以1000
  uint64_t tradeMoney;     //成交金额，仅适用于TickType=T时，实际值除以100000
  uint32_t msgSeqID;       //消息序号（仅定位调试使用）
  char     resv[7];        //保留字段
};


/*
 * ETF行情统计信息
 */
struct ETFSnapshotSse {
  uint8_t  messageType;    //消息类型，ETF快照为0x10
  uint32_t sequence;       //udp输出包序号，从1开始
  uint8_t  exchangeID;     //交易所id，上交所：1，深交所：2
  char     securityID[9];  //证券代码
  uint32_t timeStamp;      //更新时间，格式HHMMSS，例如143025 表示 14:30:25
  uint32_t etfBuyNum;      //ETF申购笔数
  uint64_t etfBuyAmount;   //ETF申购数量，实际值除以1000
  uint64_t etfBuyMoney;    //ETF申购金额，实际值除以100000
  uint32_t etfSellNum;     //ETF赎回笔数
  uint64_t etfSellAmount;  //ETF赎回数量，实际值除以1000
  uint64_t etfSellMoney;   //ETF赎回金额，实际值除以100000
  uint32_t msgSeqID;       //消息序号（仅定位调试使用）
  char     resv[1];        //保留字段
};


#pragma pack(pop)

#endif
