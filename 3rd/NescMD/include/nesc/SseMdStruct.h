/*****************************************************************************
上交所行情结构体定义，字节序为小端
******************************************************************************/
#ifndef SSE_NESC_MD_STRUCT_H
#define SSE_NESC_MD_STRUCT_H

#include <stdint.h>

namespace NescForesight {

  const int MSG_TYPE_SNAPSHOT_SSE          =       0x01;   //上交快照
  const int MSG_TYPE_BEST_ORDERS_SSE       =       0x02;   //订单明细，最多揭示50笔
  const int MSG_TYPE_INDEX_SSE             =       0x03;   //指数行情
  const int MSG_TYPE_TRADE_SSE             =       0x04;   //逐笔成交
  const int MSG_TYPE_ORDER_SSE             =       0x05;   //逐笔委托

  const int MSG_TYPE_AFTER_SNAP_SSE        =       0x07;   //上交盘后快照
  const int MSG_TYPE_AFTER_TRADE_SSE       =       0x08;   //上交盘后逐笔成交
  const int MSG_TYPE_BOND_SNAPSHOT_SSE     =       0x0C;   //上交债券快照
  const int MSG_TYPE_BOND_TICK_SSE         =       0x0D;   //上交债券逐笔

  const int MSG_TYPE_STATIC_SSE            =       0x0F;   //上交静态信息
  const int MSG_TYPE_TICK_MERGE_SSE        =       0x11;   //上交逐笔合并消息
  const int MSG_TYPE_ORDER_BOOK_SNAP_SSE   =       0x22;   //上交订单簿行情

  const int MSG_TYPE_SNAPSHOT_L1_SSE       =       0x65;   //上交L1快照
  const int MSG_TYPE_INDEX_L1_SSE          =       0x67;   //L1指数行情
  const int MSG_TYPE_AFTER_SNAP_L1_SSE     =       0x6B;   //上交盘后快照
  const int MSG_TYPE_BOND_SNAPSHOT_L1_SSE  =       0x70;   //L1上交债券快照

  const int SNAPSHOT_LEVEL_SSE             =       10;
  const int BEST_ORDERS_LEVEL_SSE          =       50;



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
    uint8_t  messageType;    //消息类型，快照为0x1 | 0x65
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
    uint64_t IOPV;                             //ETF净值估值，实际值除以1000
    uint64_t resv4;                            //保留字段
    uint32_t bidAvgPrice;                      //买入委托加权平均价，实际值除以1000
    uint64_t bidTotalQty;                      //买入委托总数量，实际值除以1000
    uint32_t askAvgPrice;                      //卖出委托加权平均价，实际值除以1000
    uint64_t askTotalQty;                      //卖出委托总数量，实际值除以1000
    BidAskPriceQtySse bidInfo[SNAPSHOT_LEVEL_SSE]; //申买信息
    BidAskPriceQtySse askInfo[SNAPSHOT_LEVEL_SSE]; //申卖信息
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
    uint8_t  messageType;               //消息类型，指数为0x3 | 0x67
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
    uint8_t  messageType;              //消息类型，上交快照为0x7 | 0x6B
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
    uint8_t  messageType;                      //消息类型，债券快照为0x0C | 0x70
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
    BidAskPriceQtySse bidInfo[SNAPSHOT_LEVEL_SSE];  //申买信息
    BidAskPriceQtySse askInfo[SNAPSHOT_LEVEL_SSE];  //申卖信息
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
    uint32_t tickTime;       //订单或成交时间，精确到10毫秒，格式HHMMSSmm，例如14302501
    uint64_t buyOrderNo;     //买方订单号
    uint64_t sellOrderNo;    //卖方订单号
    uint32_t price;          //价格，实际值除以1000
    uint64_t qty;            //数量，实际值除以1000
    uint64_t tradeMoney;     //TickType=T成交时，代表成交金额，实际值除以100000；
                             //TickType=A新增委托时，代表已成交委托数量，实际值除以1000，其他无意义
    uint32_t msgSeqID;       //消息序号（仅定位调试使用）
    char     resv[7];        //保留字段
  };

  //上交静态信息结构体（查询接口）
  typedef struct StaticSseInfo_
  {
    uint32_t sequence;   //udp输出包序号，从1开始
    uint8_t exchangeID;  //交易所id，上交所：1，深交所：2
    char securityID[9];  //证券代码
    char securityName[19];//证券名称
    /** 
     * 证券类别
     * ‘ES’表示股票；‘EU’表示基金；‘D’表示债券； ‘RWS’表示权证；‘FF’表示期货；
     * 'CB'表示公募REITs。（参考ISO10962），集合资产管理计划、债券预发行、定向可转债取‘D’
    */
    char securityType[7];//证券类别
    /**
    * 交易所原始类别
    * ASH-以人民币交易的股票主板
    * KSH-以人民币交易的股票科创板
    ASH 以人民币交易的股票
    KSH 以人民币交易的股票
    BSH 以美元交易的股票
    OEQ 其它股票
    OPS 公开发行优先股
    PPS 非公开发行优先股
    CEF 封闭式基金
    OEF 开放式基金
    EBS 交易所交易基金（ETF）
    OFN 其它基金
    LOF LOF基金
    GBF 国债、地方债、政府支持债、政策性金融债
    DST 国债分销（仅用于分销阶段）
    DVP 公司债（地方债）分销
    CBF 企业债
    CCF 可转换企业债
    CPF 公司债、企业债、可交换债、政府支持债
    CRP 通用质押式回购
    OBD 其它债券
    WIT 国债预发行
    QRP 报价回购
    AMP 集合资产管理计划
    TCB 定向可转债
    RET 公募REITs
    CMD 控制指令（中登身份认证密码服务产品、指定登记、指定撤销复用CMD证券子类别）...
    */
    char securitySubType[4];//证券子类别
      /**
     涨跌停限制类型
    ‘N’表示交易规则（2013修订版）3.4.13规定的有涨跌幅限制类型或者权证管理办法第22条规定
    ‘R’表示交易规则（2013修订版）3.4.15和3.4.16规定的无涨跌幅限制类型
    ‘S’表示回购涨跌幅控制类型
    ‘F’表示基于参考价格的涨跌幅控制
    ‘P’表示IPO上市首日的涨跌幅控制类型
    ‘U’表示无任何价格涨跌幅控制类型
    */
    char priceLimitType;//涨跌停限制类型
    
    double upperLimitPrice;                   //涨停价
    double lowerLimitPrice;                   //跌停价
    double prevClosePx;                       //昨收价  
    double priceGear;                         //价格档位,申报价格的最小变动单位
    
    uint64_t upperQuantityLimitPriceDeclare;  //限价申报数量上限
    uint64_t lowerQuantityLimitPriceDeclare;  //限价申报数量下限 
    uint64_t buyUnit;                         //买数量单位
    uint64_t sellUnit;                        //卖数量单位
    uint64_t upperQuantityMarketPriceDeclare; //市价申报数量上限
    uint64_t lowerQuantityMarketPriceDeclare; //市价申报数量下限
    
    /**产品状态每位表示含义如下，无定义则填空格

    第 1 位对应：‘N’表示首日上市。
    第 2 位对应：‘D’表示除权。
    第 3 位对应：‘R’表示除息。
    第 4 位对应：’D’表示国内正常交易产品，’S’表示股票风险警示产品，’P’表示退市整理产品（主板），’T’表示退市转让产品，’U’表示优先股产品。
    第 5 位：‘Y’表示该产品为存托凭证。
    第 6 位对应：’L’表示债券投资者适当性要求类，’M’表示债券机构投资者适当性要求类。
    第 7 位对应：‘F’表示 15:00 闭市的产品， ‘S’表示15:30 闭市的产品，为空表示非竞价撮合平台且非新债券交易系统挂牌产品，无意义。
    第 8 位对应：‘U’表示上市时尚未盈利的发行人的股票（含存托凭证），发行人首次实现盈利后，取消该特别标识。该字段仅针对注册制股票（含存托凭证）有效。
    第 9 位对应：‘W’表示具有表决权差异安排的发行人的股票（含存托凭证）。该字段仅针对注册制股票（含存托凭证）有效。
    第 10 位对应：'Y'表示支持当日交易回转，'N'表示不支持当日交易回转。
    第 11 位对应：'Y'表示该产品为注册制股票（含存托凭证）。
    */
    
    char productStatus[21];  //产品状态
    char financeFlag;        //融资标的标志,‘T’表示是融资标的证券,‘F’表示不是融资标的证券
    char shortSaleFlag;      //融券标的标志,‘T’表示是融券标的证券,‘F’表示不是融券标的证券

  } StaticSseInfo;

  struct SseOrderbookSnap {
    uint8_t  messageType;            // 消息类型，订单簿为0x22
    uint32_t sequence;               // udp输出包序号，channelNo通道内递增保序，从1开始
    uint8_t  exchangeID;             // 交易所id，上交所：1，深交所：2
    char     securityID[9];          // 证券代码
    char     resv1[2];               // 保留字段
    char     resv2[8];               // 保留字段
    uint16_t channelNo;              // 通道号
    char     resv3[4];               // 保留字段
    uint64_t timeStamp;              // 最新订单时间，YYYYMMDDHHMMSSsss，例如20240124101010100
    uint32_t preClosePrice;          // 昨收盘价，实际值除以1000
    uint32_t numTrades;              // 总成交笔数
    uint64_t totalVolumeTrade;       // 总成交量，实际值除以1000
    uint64_t totalValueTrade;        // 总成交金额，实际值除以100000
    uint32_t lastPrice;              // 最近价，实际值除以1000
    uint32_t openPrice;              // 开盘价，实际值除以1000
    uint32_t resv4;                  // 保留字段
    uint32_t highPrice;              // 最高价，实际值除以1000
    uint32_t lowPrice;               // 最低价，实际值除以1000
    uint64_t upperlmtPrice;          // 涨停价，实际值除以1000
    uint64_t lowerlmtPrice;          // 跌停价，实际值除以1000
    uint32_t bidAvgPrice;            // 买入委托平均价，实际值除以1000。加权规则：所有档位计算平均值，买方所有价格档位的总成交金额/总成交量
    uint64_t bidTotalQty;            // 买入委托总数量，实际值除以1000
    uint32_t askAvgPrice;            // 卖出委托平均价，实际值除以1000。加权规则：所有档位计算平均值，卖方所有价格档位的总成交金额/总成交量
    uint64_t askTotalQty;            // 卖出委托总数量，实际值除以1000
    uint64_t buyPriceQueue[10];      // 十档申买价格，实际值除以1000
    uint64_t buyOrderQtyQueue[10];   // 十档申买数量，实际值除以1000
    uint64_t sellPriceQueue[10];     // 十档申卖价格，实际值除以1000
    uint64_t sellOrderQtyQueue[10];  // 十档申卖数量，实际值除以1000
    uint64_t buyNumOrdersQueue[10];  // 买10档价位总委托笔数
    uint64_t sellNumOrdersQueue[10]; // 卖10档价位总委托笔数
    uint64_t resv5;                  // 保留字段
  };

  #pragma pack(pop)

}
#endif
