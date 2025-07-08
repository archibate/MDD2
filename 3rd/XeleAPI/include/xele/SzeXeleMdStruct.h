/*****************************************************************************
艾科朗克深交所行情结构体定义，字节序为小端
******************************************************************************/
#ifndef SZE_XELE_MD_STRUCT_H
#define SZE_XELE_MD_STRUCT_H

#include <stdint.h>


#define MSG_TYPE_SNAPSHOT               0x1   //快照
#define MSG_TYPE_BEST_ORDERS            0x2   //订单明细，最多揭示50笔
#define MSG_TYPE_INDEX                  0x3   //指数行情
#define MSG_TYPE_TRADE                  0x4   //逐笔成交
#define MSG_TYPE_ORDER                  0x5   //逐笔委托
#define MSG_TYPE_AFTER_SNAP_SZ          0x6   //深交盘后快照
#define MSG_TYPE_BLOCKTRADE_SZ          0x7   //深交盘后定价大宗交易
#define MSG_TYPE_BOND_TRADE             0x8   //债券逐笔成交
#define MSG_TYPE_BOND_BLOCKTRADE        0x9   //债券大额逐笔成交
#define MSG_TYPE_BOND_ORDER             0x0A  //债券逐笔委托
#define MSG_TYPE_BOND_BLOCKORDER        0x0B  //债券大额逐笔委托
#define MSG_TYPE_BOND_SNAPSHOT          0x0C  //债券快照
#define MSG_TYPE_BOND_BEST_ORDERS       0x0D  //订单明细，最多揭示50笔
#define MSG_TYPE_BOND_SNAP_SZ_L1        0x0E  //深交L1债券快照
#define MSG_TYPE_SNAP_SZ_L1             0x0F  //深交L1快照
#define MSG_TYPE_TREE_SNAP_SZ           0x10  //深交逐笔转快照消息类型
#define MSG_TYPE_BOND_BID_TRADE         0x11  //债券竞买逐笔成交
#define MSG_TYPE_BOND_BID_ORDER         0x12  //债券竞买逐笔委托

#define SNAPSHOT_LEVEL                  10
#define BEST_ORDERS_LEVEL               50



#pragma pack(push)
#pragma pack(1)

struct BidAskPriceQtySz {
  uint64_t price;    //申买、申卖价格，实际值除以1000000
  uint64_t qty;      //申买、申卖数量，实际值除以100
};

/*
 * 行情快照，期权复用该结构体
 */
struct MarketDataSnapshotSz {
  uint8_t  messageType;    //消息类型，快照为0x1
  uint32_t sequence;       //udp输出包序号，从1开始
  uint8_t  exchangeID;     //交易所id，上交所：1，深交所：2
  char     securityID[9];  //证券代码
  uint8_t  resv[2];        //保留字段
  /*
   * 产品所处的交易阶段代码
   * 第0位：S=启动（开市前）,O=开盘集合竞价,T=连续竞价,B=休市,C=收盘集合竞价,E=已闭市,H=临时停牌,A=盘后交易,V=波动性中断
   * 第1位：0=正常状态,1=全天停牌
   */
  char tradingPhaseCode[8];

  uint8_t resv2[7];                          //保留字段
  uint64_t timeStamp;                        //数据生成时间（切片时间），格式YYYYMMDDHHMMSSmmm，精确到毫秒。
                                             //示例：20190411102939120表示2019年04月11日10点29分39秒120毫秒
  uint64_t preClosePrice;                    //昨收价（来源消息头），实际值除以10000
  uint64_t numTrades;                        //总成交笔数
  uint64_t totalVolumeTrade;                 //总成交量，实际值除以100
  uint64_t totalValueTrade;                  //总成交金额，实际值除以10000
  uint64_t lastPrice;                        //最近价，实际值除以1000000
  uint64_t openPrice;                        //开盘价，实际值除以1000000
  uint64_t openInterest;                     //合约持仓量，实际值除以100
  uint64_t highPrice;                        //最高价，实际值除以1000000
  uint64_t lowPrice;                         //最低价，实际值除以1000000
  uint64_t upperlmtPrice;                    //涨停价，实际值除以1000000
  uint64_t lowerlmtPrice;                    //跌停价，实际值除以1000000
  uint64_t bidAvgPrice;                      //买入委托数量加权平均价，实际值除以1000000
  uint64_t bidTotalQty;                      //买入委托总数量，实际值除以100
  uint64_t askAvgPrice;                      //卖出委托数量加权平均价，实际值除以1000000
  uint64_t askTotalQty;                      //卖出委托总数量，实际值除以100
  BidAskPriceQtySz bidInfo[SNAPSHOT_LEVEL];  //申买信息
  BidAskPriceQtySz askInfo[SNAPSHOT_LEVEL];  //申卖信息
  uint16_t channelNo;                        //频道代码
  uint16_t mdstreamid;                       //行情类别
  uint32_t resv3;                            //保留字段
  uint64_t IOPV;                             //基金实时参考值，实际值除以1000000
  uint64_t bidNumOfOrders[SNAPSHOT_LEVEL];   //买10档价位总委托笔数  
  uint64_t askNumOfOrders[SNAPSHOT_LEVEL];   //卖10档价位总委托笔数  
};

/*
 * 逐笔转行情快照结构体
 */
struct MarketDataTreeSnapSz {
  uint8_t  messageType;    //消息类型，逐笔转行情快照为0x10
  uint32_t sequence;       //udp输出包序号，从1开始
  uint8_t  exchangeID;     //交易所id，上交所：1，深交所：2
  char     securityID[9];  //证券代码
  uint64_t timeStamp;                        //数据生成时间（切片时间），格式YYYYMMDDHHMMSSmmm，精确到毫秒。
                                             //示例：20190411102939120表示2019年04月11日10点29分39秒120毫秒
  uint64_t numTrades;                        //总成交笔数
  uint64_t totalVolumeTrade;                 //总成交量，实际值除以100
  uint64_t totalValueTrade;                  //总成交金额，实际值除以10000
  uint64_t lastPrice;                        //最近价，实际值除以1000000
  uint64_t openInterest;                     //合约持仓量，实际值除以100
  uint8_t  signLoss;                         //丢包标志,1表示丢包， 0表示未丢包
  char     resv[3];                          //保留位
  uint64_t highPrice;                        //最高价，实际值除以1000000
  uint64_t lowPrice;                         //最低价，实际值除以1000000
  uint64_t bidTotalQty;                      //买入委托总数量，实际值除以100
  uint64_t askTotalQty;                      //卖出委托总数量，实际值除以100
  uint8_t  level_ask;                        //申卖有效档位
  uint8_t  level_bid;                        //申买有效档位
  uint8_t  level_send;                       //申买、申卖发送档位
  BidAskPriceQtySz bidAskInfo[1];            //申买、申卖信息，先发送level_send档的申买信息，再发送level_send档的申卖信息
};

/*
 * 订单明细，最多揭示50笔,变长
 */
struct BestOrdersSz {
  uint8_t  messageType;                //消息类型，订单明细为0x2
  uint32_t sequence;                   //udp输出包序号，从1开始
  uint8_t  exchangeID;                 //交易所id，上交所：1，深交所：2
  char     securityID[9];              //证券代码
  char     recv;                       //保留字段

  uint64_t timeStamp;                  //数据生成时间（切片时间），格式YYYYMMDDHHMMSSmmm，精确到毫秒。
                                       //示例：20190411102939120表示2019年04月11日10点29分39秒120毫秒
  uint8_t  side;                       //买卖标识：买：1，卖：2
  uint8_t  number;                     //明细个数
  uint64_t price;                      //委托价格，实际值除以1000000
  uint64_t orders;                     //申买/卖数量，实际值除以100
  uint16_t channelNo;                  //频道代码
  uint16_t mdstreamid;                 //行情类别
  char     resv[2];                    //保留字段
  uint64_t volume[0];                  //委托数量，变长数组，最多揭示50笔，实际值除以100
};

/*
 * 指数行情
 */
struct IndexSz {
  uint8_t  messageType;               //消息类型，指数为0x3
  uint32_t sequence;                  //udp输出包序号，从1开始
  uint8_t  exchangeID;                //交易所id，上交所：1，深交所：2
  char     securityID[9];             //证券代码
  uint8_t  resv;                      //保留字段
  /*
   * 产品所处的交易阶段代码(仅深交所有效)
   * 第0位：S=启动（开市前）,O=开盘集合竞价,T=连续竞价,B=休市,C=收盘集合竞价,E=已闭市,H=临时停牌,A=盘后交易,V=波动性中断
   * 第1位：0=正常状态,1=全天停牌
   */
  char tradingPhaseCode[8]; 

  uint64_t timeStamp;                 //数据生成时间（切片时间），格式YYYYMMDDHHMMSSmmm，精确到毫秒。
                                      //示例：20190411102939120表示2019年04月11日10点29分39秒120毫秒
  uint64_t preClosePrice;             //前盘指数（来源扩展字段），实际值除以1000000
  uint64_t openPrice;                 //开盘指数，实际值除以1000000
  uint64_t lastPrice;                 //最新指数，实际值除以1000000

  uint64_t highPrice;                 //最高指数，实际值除以1000000
  uint64_t lowPrice;                  //最低指数，实际值除以1000000
  uint64_t closePrice;                //今日收盘指数，实际值除以1000000
  uint64_t tradeNum;                  //成交笔数
  uint64_t totalVolume;               //成交总量，实际值除以100
  uint64_t totalValue;                //成交总金额，实际值除以10000

  uint16_t channelNo;                 //频道代码
  uint16_t mdstreamid;                //行情类别
};

/*
 * 深交逐笔成交
 */
struct TradeSz {
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

  uint16_t channelNo;                 //频道代码
  uint16_t mdstreamid;                //行情类别
};

/*
 * 深交所逐笔委托
 */
struct OrderSz {
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

/*
*深交盘后行情快照
*/
struct AfterSnapshotSz {
  uint8_t  messageType;               //消息类型，深交盘后快照为0x6
  uint32_t sequence;                  //udp输出包序号，从1开始
  uint8_t  exchangeID;                //交易所id，上交所：1，深交所：2
  char     securityID[9];             //证券代码
  uint8_t  flag;                      //字段有效标识：bit0-买入有效，bit1-卖出有效，其余位保留

  /*
   * 产品所处的交易阶段代码
   * 深交所：
   * 第0位：S=启动（开市前）,O=开盘集合竞价,T=连续竞价,B=休市,C=收盘集合竞价,E=已闭市,H=临时停牌,A=盘后交易,V=波动性中断
   * 第1位：0=正常状态,1=全天停牌
   */
  char tradingPhaseCode[8];

  uint64_t timeStamp;                     //数据生成时间（切片时间），格式YYYYMMDDHHMMSSmmm，精确到毫秒。
                                          //示例：20190411102939120表示2019年04月11日10点29分39秒120毫秒
  uint64_t preClosePrice;                 //昨收价，实际值除以1000000                                      
  uint64_t bidPrice;                      //买入价格，实际值除以1000000
  uint64_t bidQty;                        //买入数量，实际值除以1000000
  uint64_t askPrice;                      //卖出价格，实际值除以1000000
  uint64_t askQty;                        //卖出数量，实际值除以1000000
  uint64_t numTrades;                     //总成交笔数
  uint64_t totalVolumeTrade;              //总成交量，实际值除以100
  uint64_t totalValueTrade;               //总成交金额，实际值除以10000

  uint16_t channelNo;                     //频道代码
  uint16_t mdstreamid;                    //行情类别
};

/*
 * 深交L1快照结构体
 */
struct L1MarketDataSnapSz {
  uint8_t  messageType;               //消息类型，深交L1快照为0xf
  uint32_t sequence;                  //udp输出包序号，从1开始
  uint8_t  exchangeID;                //交易所id，上交所：1，深交所：2
  char     securityID[9];             //证券代码
  uint8_t  resv[2];                   //保留字段

  /*
   * 产品所处的交易阶段代码
   * 深交所:
   * 第0位：S=启动（开市前）,O=开盘集合竞价,T=连续竞价,B=休市,C=收盘集合竞价,E=已闭市,H=临时停牌,A=盘后交易,V=波动性中断
   * 第1位：0=正常状态,1=全天停牌
   */
  char tradingPhaseCode[8];

  uint8_t resv2[7];                          //保留字段
  uint64_t timeStamp;                        //数据生成时间（切片时间），格式YYYYMMDDHHMMSSmmm，精确到毫秒。
                                             //示例：20190411102939120表示2019年04月11日10点29分39秒120毫秒
  uint64_t preClosePrice;                    //昨收价（来源消息头），实际值除以10000
  uint64_t numTrades;                        //总成交笔数
  uint64_t totalVolumeTrade;                 //总成交量，实际值除以100
  uint64_t totalValueTrade;                  //总成交金额，实际值除以10000
  uint64_t lastPrice;                        //最近价，实际值除以1000000
  uint64_t openPrice;                        //开盘价，实际值除以1000000
  uint64_t openInterest;                     //合约持仓量，实际值除以100
  uint64_t highPrice;                        //最高价，实际值除以1000000
  uint64_t lowPrice;                         //最低价，实际值除以1000000
  uint64_t upperlmtPrice;                    //涨停价，实际值除以1000000
  uint64_t lowerlmtPrice;                    //跌停价，实际值除以1000000
  uint64_t bidAvgPrice;                      //买入委托数量加权平均价，实际值除以1000000
  uint64_t bidTotalQty;                      //买入委托总数量，实际值除以100
  uint64_t askAvgPrice;                      //卖出委托数量加权平均价，实际值除以1000000
  uint64_t askTotalQty;                      //卖出委托总数量，实际值除以100
  BidAskPriceQtySz bidInfo[5];               //申买信息
  BidAskPriceQtySz askInfo[5];               //申卖信息
  uint16_t channelNo;                        //频道代码
  uint16_t mdstreamid;                       //行情类别
  uint32_t resv3;                            //保留字段
  uint64_t IOPV;                             //基金实时参考值，实际值除以1000000
  uint64_t bidNumOfOrders[5];                //买5档价位总委托笔数  
  uint64_t askNumOfOrders[5];                //卖5档价位总委托笔数  
};

/*
 * 深交盘后定价大宗交易
 */
struct BlockTradeSz {
  uint8_t  messageType;               //消息类型，盘后定价大宗交易为0x7
  uint32_t sequence;                  //udp输出包序号，从1开始
  uint8_t  exchangeID;                //交易所id，上交所：1，深交所：2
  char     securityID[9];             //证券代码
  uint8_t  flag;                      //字段有效标识：bit0，买入有效，bit1，卖出有效，其余位保留
  /*
   * 产品所处的交易阶段代码
   * 深交所:
   * 第0位：S=启动（开市前）,O=开盘集合竞价,T=连续竞价,B=休市,C=收盘集合竞价,E=已闭市,H=临时停牌,A=盘后交易,V=波动性中断
   * 第1位：0=正常状态,1=全天停牌
   */
  char tradingPhaseCode[8];
  uint64_t timeStamp;                 //数据生成时间（切片时间），格式YYYYMMDDHHMMSSmmm，精确到毫秒。
                                      //示例：20190411102939120表示2019年04月11日10点29分39秒120毫秒
  uint64_t preClosePrice;             //昨收价（来源消息头），实际值除以10000
  uint64_t bidPrice;                  //买入价格，实际值除以1000000
  uint64_t bidQty;                    //买入数量，实际值除以1000000
  uint64_t askPrice;                  //卖出价格，实际值除以1000000
  uint64_t askQty;                    //卖出数量，实际值除以1000000
  uint64_t numTrades;                 //总成交笔数
  uint64_t totalVolumeTrade;          //总成交量，实际值除以100
  uint64_t totalValueTrade;           //总成交金额，实际值除以10000

  uint16_t channelNo;                 //频道代码
  uint16_t mdstreamid;                //行情类别
};

struct BondSubStatus {
    char     SubTradingPhaseCode[7];          //细分交易阶段(除byte0、byte1外，其余字节全部是0x00)
    uint8_t  tradingType;                     //交易方式
};

/*
 * 债券行情快照
 */
struct BondSnapshotSz {
  uint8_t  messageType;    //消息类型，债券快照为0x0C
  uint32_t sequence;       //udp输出包序号，从1开始
  uint8_t  exchangeID;     //交易所id，上交所：1，深交所：2
  char     securityID[9];  //证券代码
  uint8_t  resv[2];        //保留字段
  /*
   * 产品所处的交易阶段代码
   * 第0位：S=启动（开市前）,O=开盘集合竞价,T=连续竞价,B=休市,C=收盘集合竞价,E=已闭市,H=临时停牌,A=盘后交易,V=波动性中断
   * 第1位：0=正常状态,1=全天停牌
   */
  char tradingPhaseCode[8];

  uint8_t resv2[7];                          //保留字段
  uint64_t timeStamp;                        //数据生成时间（切片时间），格式YYYYMMDDHHMMSSmmm，精确到毫秒。
                                             //示例：20190411102939120表示2019年04月11日10点29分39秒120毫秒
  uint64_t preClosePrice;                    //昨收价（来源消息头），实际值除以10000
  uint64_t numTrades;                        //总成交笔数
  uint64_t totalVolumeTrade;                 //总成交量，实际值除以100
  uint64_t totalValueTrade;                  //总成交金额，实际值除以10000
  uint64_t lastPrice;                        //最近价，实际值除以1000000
  uint64_t openPrice;                        //开盘价，实际值除以1000000
  uint64_t resv3;                            //保留字段
  uint64_t highPrice;                        //最高价，实际值除以1000000
  uint64_t lowPrice;                         //最低价，实际值除以1000000
  uint64_t closePrice;                       //收盘价，实际值除以1000000
  uint64_t latestMatchPrice;                 //匹配成交最近价，实际值除以1000000
  uint64_t bidAvgPrice;                      //买入委托数量加权平均价，实际值除以1000000
  uint64_t bidTotalQty;                      //买入委托总数量，实际值除以100
  uint64_t askAvgPrice;                      //卖出委托数量加权平均价，实际值除以1000000
  uint64_t askTotalQty;                      //卖出委托总数量，实际值除以100
  BidAskPriceQtySz bidInfo[SNAPSHOT_LEVEL];  //申买信息
  BidAskPriceQtySz askInfo[SNAPSHOT_LEVEL];  //申卖信息
  BondSubStatus  subStatus[5];               //细分交易状态
  uint64_t auctionVolumeTrade;               //匹配成交成交量，实际值除以100
  uint64_t auctionValueTrade;                //匹配成交成交金额，实际值除以10000
  uint16_t channelNo;                        //频道代码
  uint16_t mdstreamid;                       //行情类别
  uint32_t resv4;                            //保留字段
  uint64_t IOPV;                             //基金实时参考值，实际值除以1000000
  uint64_t bidNumOfOrders[SNAPSHOT_LEVEL];   //买10档价位总委托笔数  
  uint64_t askNumOfOrders[SNAPSHOT_LEVEL];   //卖10档价位总委托笔数  
};

/*
 * 债券订单明细，最多揭示50笔,变长
 */
struct BondBestOrdersSz {
  uint8_t  messageType;                //消息类型，订单明细为0x0D
  uint32_t sequence;                   //udp输出包序号，从1开始
  uint8_t  exchangeID;                 //交易所id，上交所：1，深交所：2
  char     securityID[9];              //证券代码
  char     recv;                       //保留字段

  uint64_t timeStamp;                  //数据生成时间（切片时间），格式YYYYMMDDHHMMSSmmm，精确到毫秒。
                                       //示例：20190411102939120表示2019年04月11日10点29分39秒120毫秒
  uint8_t  side;                       //买卖标识：买：1，卖：2
  uint8_t  number;                     //明细个数
  uint64_t price;                      //委托价格，实际值除以1000000
  uint64_t orders;                     //申买/卖数量，实际值除以100
  uint16_t channelNo;                  //频道代码
  uint16_t mdstreamid;                 //行情类别
  char     resv[2];                    //保留字段
  uint64_t volume[0];                  //委托数量，变长数组，最多揭示50笔，实际值除以100
};


/*
 * 深交债券逐笔成交
 */
struct BondTradeSz {
  uint8_t  messageType;               //消息类型，债券逐笔成交为0x8
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

  uint16_t channelNo;                 //频道代码
  uint16_t mdstreamid;                //行情类别
};

/*
 * 深交债券大额逐笔成交
 */
struct BondBlockTradeSz {
  uint8_t  messageType;               //消息类型，债券大额逐笔成交为0x9
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
  uint8_t  settlePeriod;              //结算周期
  uint16_t settleType;                //结算方式

  uint16_t channelNo;                 //频道代码
  uint16_t mdstreamid;                //行情类别
};

/*
 * 深交债券竞买逐笔成交
 */
struct BondBidTradeSz {
  uint8_t  messageType;               //消息类型，债券大额逐笔成交为0x9
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
  uint64_t marginPrice;               //成交的边际价格，实际值除以10000
  char     secondaryOrderID[17];      //竞买场次编号
  uint8_t  settlePeriod;              //结算周期
  uint16_t settleType;                //结算方式
  uint16_t bidExecInstType;           //竞买成交方式
  uint16_t channelNo;                 //频道代码
  uint16_t mdstreamid;                //行情类别
};

/*
 * 深交所债券逐笔委托
 */
struct BondOrderSz {
  uint8_t  messageType;               //消息类型，债券逐笔委托为0x0A
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

/*
 * 深交所债券大额逐笔委托
 */
struct BondBlockOrderSz {
  uint8_t  messageType;               //消息类型，债券逐笔委托为0x0A
  uint32_t sequence;                  //udp输出包序号，从1开始
  uint8_t  exchangeID;                //交易所id，上交所：1，深交所：2
  char     securityID[9];             //证券代码
  char rsvd[2];                       //保留字段
  char side;                          //买卖方向：1=买，2=卖，G=借入，F=出借
  uint64_t applSeqNum;                //消息记录号
  uint64_t transactTime;              //委托时间，格式YYYYMMDDHHMMSSmmm，精确到毫秒。
                                      //示例：20190411102939120表示2019年04月11日10点29分39秒120毫秒
  uint64_t price;                     //价格，实际值除以10000
  uint64_t qty;                       //数量，实际值除以100
  char     quoteID[11];               //报价消息编号
  char     memberID[7];               //交易商代码
  char     investorType[3];           //交易主体类型
  char     investorID[11];            //交易主体代码
  char     investName[121];           //客户名称
  char     traderCode[9];             //交易员代码
  uint8_t  settlePeriod;              //结算周期
  uint16_t settleType;                //结算方式
  uint16_t channelNo;                 //频道代码
  uint16_t mdstreamid;                //行情类别
};

/*
 * 深交所债券竞买逐笔委托
 */
struct BondBidOrderSz {
  uint8_t  messageType;               //消息类型，债券逐笔委托为0x0A
  uint32_t sequence;                  //udp输出包序号，从1开始
  uint8_t  exchangeID;                //交易所id，上交所：1，深交所：2
  char     securityID[9];             //证券代码
  char rsvd[2];                       //保留字段
  char side;                          //买卖方向：1=买，2=卖，G=借入，F=出借
  uint64_t applSeqNum;                //消息记录号
  uint64_t transactTime;              //委托时间，格式YYYYMMDDHHMMSSmmm，精确到毫秒。
                                      //示例：20190411102939120表示2019年04月11日10点29分39秒120毫秒
  uint64_t price;                     //价格，实际值除以10000
  uint64_t qty;                       //数量，实际值除以100
  char     rsvd2[11];                 //保留字段
  char     memberID[7];               //交易商代码
  char     investorType[3];           //交易主体类型
  char     investorID[11];            //交易主体代码
  char     investName[121];           //客户名称
  char     traderCode[9];             //交易员代码
  uint16_t bidTransType;              //竞买业务类别
  uint16_t bidExecInstType;           //竞买成交方式
  uint64_t lowLimitPrice;             //价格下限，实际值除以10000
  uint64_t highLimitPrice;            //价格上限，实际值除以10000
  uint64_t minQty;                    //最低成交数量，实际值除以100
  char     secondaryOrderID[17];      //竞买场次编号
  uint32_t tradeData;                 //成交日期
  uint8_t  settlePeriod;              //结算周期
  uint16_t settleType;                //结算方式
  uint16_t channelNo;                 //频道代码
  uint16_t mdstreamid;                //行情类别
};


/*
 * 深交债券L1快照结构体
 */
struct L1BondSnapSz {
  uint8_t  messageType;               //消息类型，深交债券L1快照为0x0E
  uint32_t sequence;                  //udp输出包序号，从1开始
  uint8_t  exchangeID;                //交易所id，上交所：1，深交所：2
  char     securityID[9];             //证券代码
  uint8_t  resv[2];                   //保留字段

  /*
   * 产品所处的交易阶段代码
   * 深交所:
   * 第0位：S=启动（开市前）,O=开盘集合竞价,T=连续竞价,B=休市,C=收盘集合竞价,E=已闭市,H=临时停牌,A=盘后交易,V=波动性中断
   * 第1位：0=正常状态,1=全天停牌
   */
  char tradingPhaseCode[8];

  uint8_t resv2[7];                          //保留字段
  uint64_t timeStamp;                        //数据生成时间（切片时间），格式YYYYMMDDHHMMSSmmm，精确到毫秒。
                                             //示例：20190411102939120表示2019年04月11日10点29分39秒120毫秒
  uint64_t preClosePrice;                    //昨收价（来源消息头），实际值除以10000
  uint64_t numTrades;                        //总成交笔数
  uint64_t totalVolumeTrade;                 //总成交量，实际值除以100
  uint64_t totalValueTrade;                  //总成交金额，实际值除以10000
  uint64_t lastPrice;                        //最近价，实际值除以1000000
  uint64_t openPrice;                        //开盘价，实际值除以1000000
  uint64_t resv3;                            //保留字段
  uint64_t highPrice;                        //最高价，实际值除以1000000
  uint64_t lowPrice;                         //最低价，实际值除以1000000
  uint64_t upperlmtPrice;                    //涨停价，实际值除以1000000
  uint64_t lowerlmtPrice;                    //跌停价，实际值除以1000000
  uint64_t bidAvgPrice;                      //买入委托数量加权平均价，实际值除以1000000
  uint64_t bidTotalQty;                      //买入委托总数量，实际值除以100
  uint64_t askAvgPrice;                      //卖出委托数量加权平均价，实际值除以1000000
  uint64_t askTotalQty;                      //卖出委托总数量，实际值除以100
  BidAskPriceQtySz bidInfo[5];               //申买信息
  BidAskPriceQtySz askInfo[5];               //申卖信息

  BondSubStatus  subStatus[5];               //细分交易状态
  uint64_t auctionVolumeTrade;               //匹配成交成交量
  uint64_t auctionValueTrade;                //匹配成交成交金额
  uint16_t channelNo;                        //频道代码
  uint16_t mdstreamid;                       //行情类别
  uint32_t resv4;                            //保留字段
  uint64_t IOPV;                             //基金实时参考值，实际值除以1000000
  uint64_t bidNumOfOrders[5];                //买5档价位总委托笔数  
  uint64_t askNumOfOrders[5];                //卖5档价位总委托笔数  
};


#pragma pack(pop)

#endif
