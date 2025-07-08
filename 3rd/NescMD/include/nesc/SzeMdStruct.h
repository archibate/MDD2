/*****************************************************************************
深交所行情结构体定义，字节序为小端
******************************************************************************/
#ifndef SZE_NESC_MD_STRUCT_H
#define SZE_NESC_MD_STRUCT_H

#include <stdint.h>

namespace NescForesight {

  const int MSG_TYPE_SNAPSHOT_SZ          =           0x01;   //快照
  const int MSG_TYPE_BEST_ORDERS_SZ       =           0x02;   //订单明细，最多揭示50笔
  const int MSG_TYPE_INDEX_SZ             =           0x03;   //指数行情
  const int MSG_TYPE_TRADE_SZ             =           0x04;   //逐笔成交
  const int MSG_TYPE_ORDER_SZ             =           0x05;   //逐笔委托
  const int MSG_TYPE_AFTER_SNAP_SZ        =           0x06;   //深交盘后快照
  const int MSG_TYPE_BLOCKTRADE_SZ        =           0x07;   //深交盘后定价大宗交易
  const int MSG_TYPE_BOND_TRADE_SZ        =           0x08;   //债券逐笔成交
  const int MSG_TYPE_BOND_BLOCKTRADE_SZ   =           0x09;   //债券大额逐笔成交
  const int MSG_TYPE_BOND_ORDER_SZ        =           0x0A;   //债券逐笔委托
  const int MSG_TYPE_BOND_BLOCKORDER_SZ   =           0x0B;   //债券大额逐笔委托
  const int MSG_TYPE_BOND_SNAPSHOT_SZ     =           0x0C;   //债券快照
  const int MSG_TYPE_BOND_BEST_ORDERS_SZ  =           0x0D;   //订单明细，最多揭示50笔
  const int MSG_TYPE_BOND_BID_TRADE_SZ    =           0x11;   //债券竞买逐笔成交
  const int MSG_TYPE_BOND_BID_ORDER_SZ    =           0x12;   //债券竞买逐笔委托

  const int MSG_TYPE_ORDER_BOOK_SNAP_SZ   =           0x22;   //深交订单簿行情

  const int MSG_TYPE_SNAPSHOT_L1_SZ       =           0x65;   //L1快照
  const int MSG_TYPE_INDEX_L1_SZ          =           0x67;   //L1指数行情
  const int MSG_TYPE_AFTER_SNAP_L1_SZ     =           0x6A;   //L1深交盘后快照
  const int MSG_TYPE_BLOCKTRADE_L1_SZ     =           0x6B;   //L1深交盘后定价大宗交易
  const int MSG_TYPE_BOND_SNAPSHOT_L1_SZ  =           0x70;   //L1债券快照

  const int SNAPSHOT_LEVEL_SZ             =           10;
  const int BEST_ORDERS_LEVEL_SZ          =           50;



  #pragma pack(1)

  struct BidAskPriceQtySz {
    uint64_t price;    //申买、申卖价格，实际值除以1000000
    uint64_t qty;      //申买、申卖数量，实际值除以100
  };

  /*
  * 行情快照，期权复用该结构体
  */
  struct MarketDataSnapshotSz {
    uint8_t  messageType;    //消息类型，快照为0x1 | 0x65
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
    BidAskPriceQtySz bidInfo[SNAPSHOT_LEVEL_SZ];  //申买信息
    BidAskPriceQtySz askInfo[SNAPSHOT_LEVEL_SZ];  //申卖信息
    uint16_t channelNo;                        //频道代码
    uint16_t mdstreamid;                       //行情类别
    uint32_t resv3;                            //保留字段
    uint64_t IOPV;                             //基金实时参考值，实际值除以1000000
    uint64_t bidNumOfOrders[SNAPSHOT_LEVEL_SZ];   //买10档价位总委托笔数
    uint64_t askNumOfOrders[SNAPSHOT_LEVEL_SZ];   //卖10档价位总委托笔数
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
    uint8_t  messageType;               //消息类型，指数为0x3 | 0x67
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
    uint8_t  messageType;               //消息类型，深交盘后快照为0x6 | 0x6A
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
  * 深交盘后定价大宗交易
  */
  struct BlockTradeSz {
    uint8_t  messageType;               //消息类型，盘后定价大宗交易为0x7 | 0x6B
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
    uint8_t  messageType;    //消息类型，债券快照为0x0C | 0x70
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
    BidAskPriceQtySz bidInfo[SNAPSHOT_LEVEL_SZ];  //申买信息
    BidAskPriceQtySz askInfo[SNAPSHOT_LEVEL_SZ];  //申卖信息
    BondSubStatus  subStatus[5];               //细分交易状态
    uint64_t auctionVolumeTrade;               //匹配成交成交量，实际值除以100
    uint64_t auctionValueTrade;                //匹配成交成交金额，实际值除以10000
    uint16_t channelNo;                        //频道代码
    uint16_t mdstreamid;                       //行情类别
    uint32_t resv4;                            //保留字段
    uint64_t IOPV;                             //基金实时参考值，实际值除以1000000
    uint64_t bidNumOfOrders[SNAPSHOT_LEVEL_SZ];   //买10档价位总委托笔数
    uint64_t askNumOfOrders[SNAPSHOT_LEVEL_SZ];   //卖10档价位总委托笔数
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

  //深交静态信息结构体
  typedef struct StaticSzInfo_
  {
    uint32_t sequence;   //udp输出包序号，从1开始
    uint8_t exchangeID;  //交易所id，上交所：1，深交所：2
    char securityID[9];  //证券代码
    char securityName[41];//证券名称

    /**证券详细类别
    * 交易所原始类别
    1-主板 A 股
    3-创业板股票
    4-主板 B 股
    5-国债（含地方债）
    6-企业债
    7-公司债
    8-可转债
    9-私募债
    10-可交换私募债
    11-证券公司次级债
    12-质押式回购
    13-资产支持证券
    14-本市场股票 ETF
    15-跨市场股票 ETF
    16-跨境 ETF
    17-本市场实物债券ETF
    18-现金债券 ETF
    19-黄金 ETF
    20-货币 ETF
    21-杠杆 ETF
    22-商品期货 ETF
    23-标准 LOF
    24-分级子基金
    25-封闭式基金
    26-仅申赎基金
    28-权证
    29-个股期权
    30-ETF 期权
    33-优先股
    34-证券公司短期债
    35-可交换公司债
    36-主板存托凭证
    37-创业板存托凭证
    38-基础设施基金
    39-定向可转债
    40-跨银行间实物债券ETF
    ....
    */
    uint32_t securitySubType;//证券详细类别
  
    char tickerQualificationFlag;//合约适当性管理标志 Y-是 N-否
    
    /**合约适当性类别
    
    交易所原始类型枚举
    投资者适当性管理分类
    0=包括普通投资者、专业投资者在内的所有投资者
    1=仅专业投资者
    2=仅专业投资者中的机构投资者
  */
    uint8_t tickerQualificationClass;//合约适当性类别
    
    char registrationFlag;//是否注册制(仅适用创业板股票，创新企业股票及存托凭证)是否具有协议控制架构(仅适用创业板股票，创新企业股票及存托凭证)Y-是 N-否
    char vieFlag;//是否具有协议控制架构(仅适用创业板股票，创新企业股票及存托凭证)Y-是 N-否
    char noProfitFlag;//是否尚未盈利(仅适用创业板股票，创新企业股票及存托凭证)Y-是 N-否
    char weightedVotingRightsFlag;//是否存在投票权差异(仅适用创业板股票，创新企业股票及存托凭证)Y-是 N-否
    char priceLimitFlag;//是否有涨跌幅限制Y-是 N-否
    
    double upperLimitPrice;                   //涨停价
    double lowerLimitPrice;                   //跌停价
    double prevClosePx;                       //昨收价  
    double priceGear;                         //价格档位,申报价格的最小变动单位
    
    uint64_t buyQtyUpperLimit;//限价买委托数量上限
    uint64_t buyQtyLowerLimit;//限价买委托数量下限，暂未提供
    uint64_t buyQtyUnit;//限价买数量单位
    uint64_t sellQtyUpperLimit;//限价卖委托数量上限
    uint64_t sellQtyLowerLimit;//限价卖委托数量下限，暂未提供
    uint64_t sellQtyUnit;//限价卖数量单位
    uint64_t marketBuyQtyUpperLimit;//市价买委托数量上限
    uint64_t marketBuyQtyLowerLimit;//市价买委托数量下限，暂未提供
    uint64_t marketBuyQtyUnit;//市价买数量单位
    uint64_t marketSellQtyUpperLimit;//市价卖委托数量上限
    uint64_t marketSellQtyLowerLimit;//市价卖委托数量下限，暂未提供
    uint64_t marketSellQtyUnit;//市价卖数量单位
    
    /**
    证券状态代码:
    1-停牌
    2-除权
    3-除息
    4-ST
    5-*ST
    6-上市首日
    7-公司再融资
    8-恢复上市首日或重新上市首日
    9-网络投票
    10-退市整理期
    12-增发股份上市
    13-合约调整
    14-暂停上市后协议转让
    15-实施双转单调整
    16-特定债券转让
    17-上市初期
    18-退市整理期首日
    */
    
    uint8_t securityStatus[20];  //证券状态,多状态拼接
    
  } StaticSzInfo;

  struct SzOrderbookSnap {
    uint8_t	  messageType;             // 消息类型，订单簿为0x22
    uint32_t	sequence;                // udp输出包序号，channelNo通道内递增保序，从1开始
    uint8_t	  exchangeID;              // 交易所id，上交所：1，深交所：2
    char	    securityID[9];           // 证券代码
    char	    resv1[2];                // 保留字段
    char	    resv2[8];                // 保留字段
    uint16_t  channelNo;               // 通道号
    uint8_t	  resv3[5];                // 保留字段
    uint64_t	timeStamp;               // 最新订单时间，YYYYMMDDHHMMSSsss，例如20240124101010100
    uint64_t	preClosePrice;           // 昨收盘价，实际值除以10000
    uint64_t	numTrades;               // 总成交笔数
    uint64_t	totalVolumeTrade;        // 总成交量，实际值除以100
    uint64_t	totalValueTrade;         // 总成交金额，实际值除以10000
    uint64_t	lastPrice;               // 最近价，实际值除以1000000
    uint64_t	openPrice;               // 开盘价，实际值除以1000000
    uint64_t	resv4;                   // 保留字段
    uint64_t	highPrice;               // 最高价，实际值除以1000000
    uint64_t	lowPrice;                // 最低价，实际值除以1000000
    uint64_t	upperlmtPrice;           // 涨停价，实际值除以1000000
    uint64_t	lowerlmtPrice;           // 跌停价，实际值除以1000000
    uint64_t	bidAvgPrice;             // 买入委托平均价，实际值除以1000000。加权规则：所有档位计算平均值，买方所有价格档位的总成交金额/总成交量
    uint64_t	bidTotalQty;             // 买入委托总数量，实际值除以100
    uint64_t	askAvgPrice;             // 卖出委托平均价，实际值除以1000000。加权规则：所有档位计算平均值，卖方所有价格档位的总成交金额/总成交量
    uint64_t	askTotalQty;             // 卖出委托总数量，实际值除以100
    uint64_t	buyPriceQueue[10];       // 十档申买价格，实际值除以1000000
    uint64_t	buyOrderQtyQueue[10];    // 十档申买数量，实际值除以100
    uint64_t	sellPriceQueue[10];      // 十档申卖价格，实际值除以1000000
    uint64_t	sellOrderQtyQueue[10];   // 十档申卖数量，实际值除以100
    uint64_t	buyNumOrdersQueue[10];   // 买10档价位总委托笔数
    uint64_t	sellNumOrdersQueue[10];  // 卖10档价位总委托笔数
    uint64_t	resv5;                   // 保留字段
  };

  #pragma pack(8)

}
#endif
