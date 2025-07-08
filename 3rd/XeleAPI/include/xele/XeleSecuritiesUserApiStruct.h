/////////////////////////////////////////////////////////////////////////
///@system xele-trade-securities
///@company 南京艾科朗克信息科技有限公司
///@file XeleSecuritiesUserApiStruct.h
///@brief 业务数据结构
/////////////////////////////////////////////////////////////////////////

#ifndef _XELE_API_STRUCT_H
#define _XELE_API_STRUCT_H

#include "XeleSecuritiesUserApiData.h"

#pragma pack(push, 1)

///心跳
struct CXeleHeartBeat {
  ///预留
  char                            Reserved[1];
};

///响应信息
struct CXeleRspInfo {
  ///错误编号
  TXeleErrorIDType                ErrorID;
  ///错误信息
  TXeleErrorMsgType               ErrorMsg;
};

///Fair协议头
struct CXeleFairReqHeadField {
  ///协议号
  TXeleFairProtocolType           ProtocolId;
  ///消息编号
  TXeleMessageIDType              MessageID;
  ///消息长度
  TXeleMessageLenType             MessageLength;
  //会话代码
  TXeleSessionIDType              SessionId;
  //校验用
  TXeleTokenType                  Token;
  ///序列号
  TXeleSeqNoType                  SeqNo;
  ///请求编号
  TXeleRequestIDType              RequestId;
};

///Fair响应协议头
struct CXeleFairRspHeadField {
  ///协议号
  TXeleFairProtocolType           ProtocolId;
  ///消息编号
  TXeleMessageIDType              MessageID;
  ///消息长度
  TXeleMessageLenType             MessageLength;
  ///请求编号
  TXeleRequestIDType              RequestId;
  ///序列号
  TXeleSeqNoType                  SeqNo;
  ///流序号
  TXeleSeqSeriesType              SeqSeries;
  ///结束标识
  TXeleIsLastType                 IsLast;
  ///错误编号
  TXeleErrorIdType                ErrorId;
  ///错误类型
  TXeleErrorTypeType              ErrorType;
  ///交易所类型
  TXeleMarketType                 Market;
  ///预留
  char                            Reserved[2];
};

////////Ctrl_F_System//////////
///////////////////////////////
/////// 系统相关 //////////////
//////////////////////////////

///登录请求
struct CXeleReqUserLoginField {
  ///资金账户
  TXeleUserIDType                 AccountID;
  ///交易用户密码
  TXeleUserPasswordType           Password;
  ///终端软件AppID
  TXeleAppIDType                  AppID;
  ///终端软件授权码(暂不使用)
  TXeleAuthCodeType               AuthCode;
  ///客户端IP,自动获取
  TXeleClientIpType               ClientIp;
  ///客户端MAC,自动获取
  TXeleClientMacType              ClientMac;
  ///客户端登录子节点
  TXeleSubClientIndexType         SubClientIndex;
  ///客户端报单IP(暂未使用)
  TXeleClientIpType               OrderIp;
  ///委托方式
  TXeleOperwayType                Operway;
  ///流水重构标志
  /// =0表示不进行流水重构
  /// =1表示只进行资金流水重构；
  /// =2表示只进行报文流水重构;
  /// =3表示资金和报文流水重构;
  TXeleFlowRebuildType            FlowRebuildFlag;
  ///TXeleProtocolFlag Api协议号标记字段 ‘0’表示2.5（含）之前的协议 ‘1’表示3.0协议 用户无需填写
  TXeleProtocolFlag               ProtocolFlag;
  ///软件节点是否接收其他节点回报信息
  TXeleSoftRspRcvType             SoftRspRcv;
  ///站点前缀信息
  TXelePcPreFixType               PcPrefix;
  ///硬盘分区信息
  TXeleHardDiskPartitionType      HdPartitionInfo;
  ///营业部代码
  TXeleOrgIDType                  BusinessCode;
  ///重连校验字段(内部使用)
  TXeleDateType                   TradingDay;
  ///终端软件AppID扩展字段
  TXeleAppIDExtType               AppIDExt;
  ///硬件节点是否接收其他节点回报信息
  TXeleFpgaRspRcvType             FpgaRspRcv;
  ///预留
  char                            Reserved[19];
};

///登录应答
struct CXeleRspUserLoginField {
  ///交易日
  TXeleDateType                   TradingDay;
  ///登录成功时间
  TXeleShortTimeType              LoginTime;
  ///用户本地最大报单号
  TXeleOrderIDType                MaxUserLocalID;
  ///资金账户
  TXeleUserIDType                 AccountID;
  ///会话代码
  TXeleSessionIDType              SessionId;
  ///校验用
  TXeleTokenType                  Token;
  ///客户端登录子节点
  TXeleSubClientIndexType         SubClientIndex;
  ///交易ip
  TXeleTradeIPType                TradeDestIp;
  ///交易端口
  TXeleTradePortType              TradeDestPort;
  ///交易通道有效标志, 0:无效，1:有效
  TXeleTradeFlagType              TradeFlag;
  ///委托方式
  TXeleOperwayType                Operway;
  ///交易所类型
  TXeleMarketType                 Market;
  ///报单通道,'0':硬件通道 '1':软件通道
  TXeleTradeType                  TradeType;
  ///心跳时间
  TXeleHeartBeatInterval          HeartBeatInterval;
  ///心跳超时时间
  TXeleHeartBeatTimeout           HeartBeatTimeout;
  ///登录的是互联网端口(内部使用)
  TXeleInternetType               IsInternetConnect;
  ///预留
  char                            Reserved[97];
};

///添加交易链路请求
struct CXeleReqInitTraderField {
  ///资金账户
  TXeleUserIDType                 AccountID;
  ///交易ip
  TXeleTradeIPType                TradeDestIp;
  ///交易端口
  TXeleTradePortType              TradeDestPort;
  ///源ip
  TXeleTradeIPType                TradeSrcIp;
  ///源端口
  TXeleTradePortType              TradeSrcPort;
  /// 预留
  char                            Reserved[94];
};

///添加交易链路请求响应
struct CXeleRspInitTraderField {
  ///资金账户
  TXeleUserIDType                 AccountID;
  // 预留
  char                            Reserved[113];
};

///用户密码修改请求
struct CXeleReqUserPasswordUpdateField {
  ///资金账户
  TXeleUserIDType                 AccountID;
  ///新密码
  TXeleUserPasswordType           NewPassword;
  ///旧密码
  TXeleUserPasswordType           OldPassword;
  ///预留
  TXeleReserved1Type              Reserved;
};

///用户密码修改应答
struct CXeleRspUserPasswordUpdateField {
  ///资金账户
  TXeleUserIDType                 AccountID;
  ///新密码
  TXeleUserPasswordType           NewPassword;
  ///旧密码
  TXeleUserPasswordType           OldPassword;
  ///预留
  TXeleReserved1Type              Reserved;
};

///用户登出请求
struct CXeleReqUserLogoutField {
  ///资金账户
  TXeleUserIDType                 AccountID;
  ///预留
  TXeleReserved1Type              Reserved;
};

///用户登出应答
struct CXeleRspUserLogoutField {
  ///交易日
  TXeleDateType                   TradingDay;
  ///登出时间
  TXeleShortTimeType              LogoutTime;
  ///本地最大报单号
  TXeleOrderIDType                MaxUserLocalID;
  ///资金账户
  TXeleUserIDType                 AccountID;
  //会话代码
  TXeleSessionIDType              SessionId;
  ///预留
  TXeleReserved1Type              Reserved;
};

////////Ctrl_F_Public//////////
///////////////////////////////
/////// 证券、期权公用 /////////
//////////////////////////////

///报单请求
struct CXeleReqOrderInsertField {
  ///用户本地报单编号
  TXeleOrderIDType                UserLocalID;
  ///证券代码
  TXeleSecuritiesIDType           SecuritiesID;
  ///买卖方向
  TXeleDirectionType              Direction;
  ///开平标志(股票不填)
  TXeleOffsetFlagType             CmbOffsetFlag;
  ///限价价格
  TXelePriceType                  LimitPrice;
  ///报单数量
  TXeleVolumeType                 Volume;
  ///报单价格条件
  TXeleOrderTypeType              OrderType;
  ///有效期类型(股票填0x30)
  TXeleTimeConditionType          TimeCondition;
  ///合约类型(暂不使用)
  TXeleSecuritiesType             SecuritiesType;
  ///备兑标志(期权使用)
  TXeleCoveredFlagType            CoveredOrUncovered;
  ///预留
  char                            Reserved2[2];
  ///业务单元(用户定义)
  TXeleBusinessUnitType           BusinessUnit;
  ///委托方式
  TXeleOperwayType                Operway;
  ///交易前置id,默认或者未填写为0网关轮询,查询接口返回的网关需要转换成整型，如'1'转换成1后填充该字段
  TXeleExchangeIDType             ExchangeFrontID;
  ///内部使用
  unsigned char                   ReservedNum;
  ///预留
  char                            Reserved[3];
  ///预留
  TXeleErrorIdType                ErrorId;
};

///批量报单请求
struct CXeleReqBatchOrderInsertField {
  ///批量报单类型 ('1'表示等量拆单 '2'表示递减拆单 '3'表示多证券委托组合)
  TXeleBatchType                  BatchType;
  ///单笔上限(等量拆单和递减拆单类型使用,等量拆单时表示单笔报单数量，递减拆单时表示递减起始值)
  TXeleVolumeType                 MaxOrderQty;
  ///递减数量(递减拆单类型使用)
  TXeleVolumeType                 DecreaseQty;
  ///多证券委托组合数量，表示多证券委托的数量（多证券委托组合使用）
  TXeleVolumeType                 BatchOrderQty;
  ///批量报单用户起始本地报单编号(等量拆单和递减拆单类型使用)
  TXeleOrderIDType                UserLocalID;
  ///证券代码(等量拆单和递减拆单类型使用)
  TXeleSecuritiesIDType           SecuritiesID;
  ///买卖方向(等量拆单和递减拆单类型使用)
  TXeleDirectionType              Direction;
  ///限价价格(等量拆单和递减拆单类型使用)
  TXelePriceType                  LimitPrice;
  ///报单数量(等量拆单和递减拆单类型使用)
  TXeleVolumeType                 Volume;
  ///报单价格条件(等量拆单和递减拆单类型使用)
  TXeleOrderTypeType              OrderType;
  ///业务单元(用户定义)(等量拆单和递减拆单类型使用)
  TXeleBusinessUnitType           BusinessUnit;
  ///多证券委托组合(多证券委托组合使用)
  CXeleReqOrderInsertField        ReqOrderInsertField[BatchMuiltMax];
  ///本次批量报单中，被拆分成子单数量，无需填写，接口内部填写,可以用来更新UserLocalID字段
  TXeleVolumeType                 SplitOrderVolume;
  ///预留
  char                            Reserved[13];
};

///报单应答
struct CXeleRspOrderInsertField {
  ///柜台报单编号int类型
  TXeleOrderIDType                OrderSysID;
  ///用户本地报单编号
  TXeleOrderIDType                UserLocalID;
  ///证券代码
  TXeleSecuritiesIDType           SecuritiesID;
  ///买卖方向
  TXeleDirectionType              Direction;
  ///开平标志（股票不填）
  TXeleOffsetFlagType             CmbOffsetFlag;
  ///限价价格
  TXelePriceType                  LimitPrice;
  ///报单数量
  TXeleVolumeType                 Volume;
  ///报单类型
  TXeleOrderTypeType              OrderType;
  ///有效期类型
  TXeleTimeConditionType          TimeCondition;
  ///交易所类型
  TXeleMarketType                 Market;
  ///预埋单标记 '0':非预埋单 '1':预埋单
  TXelePreOrderFlag               PreOrderFlag;
  ///错误编号，拒单使用
  TXeleErrorIdType                ErrorId;
  ///客户端登录子节点
  TXeleSubClientIndexType         SubClientIndex;
  ///业务单元(用户定义)
  TXeleBusinessUnitType           BusinessUnit;
  ///柜台报单编号str类型
  TXeleStrOrderSysIDType          StrOrderSysID;
  ///投资者账号
  TXeleUserIDType                 AccountID;
  ///交易所报单编号
  TXeleOrderExchangeIDType        OrderExchangeID;
  ///交易前置id(暂不使用)
  TXeleExchangeIDType             ExchangeFrontID;
  ///备兑标志(期权使用)
  TXeleCoveredFlagType            CoveredOrUncovered;
  ///预留
  char                            Reserved[12];
};

///撤单请求
struct CXeleReqOrderActionField {
  ///用户本地报单编号
  TXeleOrderIDType                UserLocalID;
  ///撤单报单编号
  TXeleOrigSysIDType              OrigSysID;
  ///预留
  char                            Reserved1[24];
  ///业务单元，(用户定义)
  TXeleBusinessUnitType           BusinessUnit;
  ///订单所有类型(暂不使用)
  TXeleTradeOwnerType	          OwnerType;
  ///委托方式
  TXeleOperwayType                Operway;
  ///交易前置id(暂不使用)
  TXeleExchangeIDType             ExchangeFrontID;
  ///预留
  char                            Reserved2[3];
  ///错误编号，拒单使用
  TXeleErrorIdType                ErrorId;
};

///撤单应答
struct CXeleRspOrderActionField {
  ///柜台报单编号int类型
  TXeleOrderIDType                OrderSysID;
  ///用户本地报单编号
  TXeleOrderIDType                UserLocalID;
  ///被撤单柜台报单编号
  TXeleOrigSysIDType              OrigSysID;
  ///被撤单用户本地报单编号(柜台2.5以及2.5以上版本才支持该字段)
  TXeleOrderIDType                OrigUserLocalID;
  ///被撤单柜台报单编号str类型(柜台2.5以及2.5以上版本才支持该字段)
  TXeleStrOrderSysIDType          OrigStrOrderSysID;
  ///预埋单标记 '0':非预埋单 '1':预埋单
  TXelePreOrderFlag               PreOrderFlag;
  ///预留
  char                            Reserved0[7];
  ///错误编号
  TXeleErrorIdType                ErrorId;
  ///客户端登录子节点
  TXeleSubClientIndexType         SubClientIndex;
  ///业务单元(用户定义)
  TXeleBusinessUnitType           BusinessUnit;
  ///订单所有类型
  TXeleTradeOwnerType	          OwnerType;
  ///柜台报单编号str类型
  TXeleStrOrderSysIDType          StrOrderSysID;
  ///投资者账号
  TXeleUserIDType                 AccountID;
  ///交易所类型
  TXeleMarketType                 Market;
  ///交易所报单编号
  TXeleOrderExchangeIDType        OrderExchangeID;
  ///交易前置id(暂不使用)
  TXeleExchangeIDType             ExchangeFrontID;
  ///预留
  char                            Reserved1[11];
};

///报单回报
struct CXeleRtnOrderField {
  ///用户本地报单编号
  TXeleOrderIDType                UserLocalID;
  ///证券代码
  TXeleSecuritiesIDType           SecuritiesID;
  ///买卖方向
  TXeleDirectionType              Direction;
  ///开平标志
  TXeleOffsetFlagType             CmbOffsetFlag;
  ///价格(期权精度0.0001, 股票精度0.001)
  TXelePriceType                  LimitPrice;
  ///报单数量
  TXeleVolumeType                 Volume;
  ///报单类型
  TXeleOrderTypeType              OrderType;
  ///有效期类型
  TXeleTimeConditionType          TimeCondition;
  ///交易所类型
  TXeleMarketType                 Market;
  ///备兑标志(期权使用)
  TXeleCoveredFlagType            CoveredOrUncovered;
  ///柜台报单编号int类型
  TXeleOrderIDType                OrderSysID;
  ///交易所报单编号
  TXeleOrderExchangeIDType        OrderExchangeID;
  ///接受请求时间
  TXeleTimeType                   TransactTime;
  ///对应申报市价转限价的订单，这里填写转为限价订单的价格，
  ///单位：元（期权精度0.0001, 股票精度0.001）
  TXelePriceType                  DiscretionPrice;
  ///累计成交数量
  TXeleVolumeType                 TradeVolume;
  ///未成交手数
  ///如果状态是撤单或者部撤 ，leavesVolume是已经成功撤单的数量；
  ///如果报单状态是部分成交 ，leavesVolume表示未成交数量 = 报单数量 - 累计成交数量
  TXeleVolumeType                 LeavesVolume;
  ///订单状态
  TXeleOrderStatusType            OrderStatus;
  ///保证金，暂未使用
  TXeleMoneyType                  Margin;
  ///冻结权利金，暂未使用
  TXeleMoneyType                  FrozenPremium;
  ///冻结手续费，暂未使用
  TXeleMoneyType                  FrozenFee;
  ///客户端登录子节点
  TXeleSubClientIndexType         SubClientIndex;
  ///业务单元(用户定义)
  TXeleBusinessUnitType           BusinessUnit;
  // 流水重构报文标记
  TXeleRecoveryFlagType           RecoveryFlag;
  ///柜台报单编号str类型
  TXeleStrOrderSysIDType          StrOrderSysID;
  ///投资者账号
  TXeleUserIDType                 AccountID;
  ///交易前置id,多网关版本使用
  ///返回值为实际报单的网关id + 1
  ///返回值为1代表是0号网关,2代表1号网关
  TXeleExchangeIDType             ExchangeFrontID;
  ///被撤单用户本地报单编号(柜台2.5以及2.5以上版本才支持该字段)
  TXeleOrderIDType                OrigUserLocalID;
  ///被撤单柜台报单编号int类型(柜台2.5以及2.5以上版本才支持该字段)
  TXeleOrigSysIDType              OrigOrderSysID;
  ///被撤单柜台报单编号str类型(带号段)(柜台2.5以及2.5以上版本才支持该字段)
  TXeleStrOrderSysIDType          OrigStrOrderSysID;
  ///预留
  char                            Reserved[74];
};

///成交回报
struct CXeleRtnTradeField {
  ///用户本地报单编号
  TXeleOrderIDType                UserLocalID;
  ///证券代码
  TXeleSecuritiesIDType           SecuritiesID;
  ///买卖方向
  TXeleDirectionType              Direction;
  ///开平标志，股票不填
  TXeleOffsetFlagType             CombOffsetFlag;
  ///限价价格(期权精度0.0001, 股票精度0.001)
  TXelePriceType                  LimitPrice;
  ///数量
  TXeleVolumeType                 Volume;
  ///报单类型
  TXeleOrderTypeType              OrderType;
  ///有效期类型
  TXeleTimeConditionType          TimeCondition;
  ///交易所类型
  TXeleMarketType                 Market;
  ///备兑标志(期权使用)
  TXeleCoveredFlagType            CoveredOrUncovered;
  ///柜台报单编号int类型
  TXeleOrderIDType                OrderSysID;
  ///交易所报单编号
  TXeleOrderExchangeIDType        OrderExchangeID;
  ///暂不使用
  TXeleOrderIDType                TradeID;
  ///成交价格(期权精度0.0001, 股票精度0.001)
  TXelePriceType                  TradePrice;
  ///成交数量
  TXeleVolumeType                 TradeVolume;
  ///未成交手数（报单数量-累计成交数量）
  TXeleVolumeType                 LeavesVolume;
  ///订单执行时间
  TXeleTimeType                   TransactTime;
  ///原有订单接受时间(上交使用)
  TXeleTimeType                   OrigTime;
  ///订单状态
  TXeleOrderStatusType            OrderStatus;
  ///成交金额，精度0.01
  TXeleMoneyType                  TotalValueTraded;
  ///佣金
  TXeleSecuritiesCommissionType   Commission;
  ///印花税
  TXeleStampTaxRateType           StampTax;
  ///过户费
  TXeleTransferFeeRateType        Transfer;
  ///总手续费
  TXeleTotalFeeType               TotalFee;
  ///保证金，暂未使用
  TXeleMoneyType                  Margin;
  ///权利金，暂未使用
  TXeleMoneyType                  Premium;
  ///客户端登录子节点
  TXeleSubClientIndexType         SubClientIndex;
  ///业务单元(用户定义)
  TXeleBusinessUnitType           BusinessUnit;
  // 流水重构报文标记
  TXeleRecoveryFlagType           RecoveryFlag;
  ///柜台报单编号str类型
  TXeleStrOrderSysIDType          StrOrderSysID;
  ///投资者账号
  TXeleUserIDType                 AccountID;
  /// 交易所执行编号
  TXeleOrderExchangeIDType        ExecID;
  ///交易前置id,多网关版本使用
  ///返回值为实际报单的网关id + 1
  ///返回值为1代表是0号网关,2代表1号网关
  TXeleExchangeIDType             ExchangeFrontID;
  /// 累计成交手数
  TXeleVolumeType                 CumQty;
  ///预留
  char                            Reserved[72];
};

///报单查询请求
struct CXeleReqQryOrderField {
  ///柜台报单编号int类型，不为0表示精确查询，本地报单编号、合约和时间条件都无效
  TXeleOrderIDType                OrderSysID;
  ///证券代码 范围查询，配合分页查询变量使用
  TXeleSecuritiesIDType           SecuritiesID;
  ///本地报单编号，不为0表示精确查询，合约和时间条件无效
  TXeleOrderIDType                UserLocalID;
  ///开始时间   格式：HHMMSSmmm(时分秒毫秒) 范围查询
  TXeleShortTimeType              TimeStart;
  ///结束时间   格式：HHMMSSmmm(时分秒毫秒) 范围查询
  TXeleShortTimeType              TimeEnd;
  ///分页查询起始值 不填，默认从第一条开始
  TXeleVolumeType                 StartNum;
  ///单次分页查询数量 不填，使用系统参数表配置分页数量
  TXeleVolumeType                 Num;
  ///排序类型
  TXeleSortType                   SortType;
  /*
   * 订单状态位图(支持 XTS-3.1.1149-1104dd4_7.9 之后版本)
   * 默认为0, 全量查询
   * 支持指定状态
   * QryOrderStatus = QRYSTAT_REPORTING|QRYSTAT_REPORTED|QRYSTAT_PTRADE;  查询正报、已报和部分成交单
   */
  TXeleQryStatusType              QryOrderStatus;
  /*
   * 订单业务位图(期权使用)
   * 默认为0, 全量查询
   * 支持指定状态业务类型查询
   * QryBusinessType = QRYBUS_ORDER_INSERT|QRYBUS_ORDER_ACTION;  查询正普通报单和普通撤单
   */
  TXeleQryBusinessType            QryBusinessType;
  ///预留
  char                            Reserved[115];
};

///报单查询应答
struct CXeleRspQryOrderField {
  ///用户报单编号
  TXeleOrderIDType                UserLocalID;
  ///证券代码
  TXeleSecuritiesIDType           SecuritiesID;
  ///买卖方向
  TXeleDirectionType              Direction;
  ///开平标志
  TXeleOffsetFlagType             CmbOffsetFlag;
  ///价格
  TXelePriceType                  LimitPrice;
  ///数量
  TXeleVolumeType                 Volume;
  ///报单类型
  TXeleOrderTypeType              OrderType;
  ///有效期类型
  TXeleTimeConditionType          TimeCondition;
  ///交易所类型
  TXeleMarketType                 Market;
  ///委托方式
  TXeleOperwayType                Operway;
  ///柜台报单编号int类型
  TXeleOrderIDType                OrderSysID;
  ///交易所报单编号
  TXeleOrderExchangeIDType        OrderExchangeID;
  ///接受请求时间
  TXeleTimeType                   TransactTime;
  ///对应申报市价转限价的订单，这里填写转为限价订单的价格，单位：元
  TXelePriceType                  DiscretionPrice;
  ///累计成交数量
  TXeleVolumeType                 TradeVolume;
  ///未成交手数
  ///如果状态是撤单或者部撤 ，leavesVolume是已经成功撤单的数量；
  ///如果报单状态是部分成交 ，leavesVolume表示未成交数量 = 报单数量 - 累计成交数量
  TXeleVolumeType                 LeavesVolume;
  ///订单状态
  TXeleOrderStatusType            OrderStatus;
  ///佣金
  TXeleSecuritiesCommissionType   Commission;
  ///印花税
  TXeleStampTaxRateType           StampTax;
  ///过户费
  TXeleTransferFeeRateType        Transfer;
  ///流量费
  TXeleTrafficFeeType             TrafficFee;
  ///总手续费
  TXeleTotalFeeType               TotalFee;
  ///保证金，暂未使用
  TXeleMoneyType                  Margin;
  ///冻结权利金，暂未使用
  TXeleMoneyType                  FrozenPremium;
  ///投资者账号
  TXeleUserIDType                 AccountID;
  ///客户端登录子节点
  TXeleSubClientIndexType         SubClientIndex;
  ///业务单元(用户定义)
  TXeleBusinessUnitType           BusinessUnit;
  ///柜台报单编号str类型
  TXeleStrOrderSysIDType          StrOrderSysID;
  ///报单类型
  TXeleMessageIDType              OrderMessageId;
  ///证券类别代码
  TXeleSecuritiesType             SecuritiesType;
  ///预埋单标记 '0':非预埋单 '1':预埋单
  TXelePreOrderFlag               PreOrderFlag;
  ///报单错误编号
  TXeleErrorIdType                ErrorId;
  ///分页查询结束值 用来作为下次查询的起始值，来实现分页
  TXeleVolumeType                 EndNum;
  ///分页查询条件下，是否可以再次进行查询 只在最后一条回报中有效
  TXeleIsLastType                 QryAgain;
  ///当前条件下查询到的回报总数
  TXeleVolumeType                 TotalNum;
  ///投资者股东账号
  TXeleInvestorIDType             InvestorID;
  ///客户代码
  TXeleUserIDType                 CustID;
  ///成交金额
  TXelePriceType                  TradeAmount;
  ///主柜台未同步到备机的委托类型
  TXeleUnSyncOrderFlag            UnSyncOrderFlag;
  ///备兑标志(期权使用)
  TXeleCoveredFlagType            CoveredOrUncovered;
  ///被撤单柜台报单编号str类型(带号段)(期权使用)
  TXeleStrOrderSysIDType          OrigStrOrderSysID;
  ///报单来源,支持接入网关的柜台,此字段有效
  TXeleOrderSourceType            OrderSource;
  ///预留
  char                            Reserved[34];
};

///成交查询请求
struct CXeleReqQryTradeField {
  ///柜台报单编号int类型，不为0表示精确查询，本地报单编号、合约和时间条件都无效
  TXeleOrderIDType                OrderSysID;
  ///证券代码 范围查询，配合分页查询变量使用
  TXeleSecuritiesIDType           SecuritiesID;
  ///开始时间  格式：HHMMSSmmm(时分秒毫秒) 范围查询，配合分页查询变量使用
  TXeleShortTimeType              TimeStart;
  ///结束时间  格式：HHMMSSmmm(时分秒毫秒) 范围查询，配合分页查询变量使用
  TXeleShortTimeType              TimeEnd;
  ///用户本地报单编号，不为0表示精确查询，合约和时间条件无效
  TXeleOrderIDType                UserLocalID;
  ///分页查询起始值 不填，默认从第一条开始
  TXeleVolumeType                 StartNum;
  ///单次分页查询数量 不填，使用系统参数表配置分页数量
  TXeleVolumeType                 Num;
  ///排序类型
  TXeleSortType                   SortType;
  ///预留
  char                            Reserved[115];
};

///成交查询应答
struct CXeleRspQryTradeField {
  ///用户本地报单编号
  TXeleOrderIDType                UserLocalID;
  ///交易日
  TXeleDateType                   TradingDay;
  ///成交数量
  TXeleVolumeType                 TradeVolume;
  ///未成交手数（报单数量-累计成交数量）
  TXeleVolumeType                 LeavesVolume;
  //证券代码
  TXeleSecuritiesIDType           SecuritiesID;
  //申报时间
  TXeleShortTimeType              OrderTime;
  //成交时间
  TXeleShortTimeType              TradeTime;
  //成交价格
  TXelePriceType                  TradePrice;
  //成交金额
  TXelePriceType                  TradeAmount;
  ///暂不使用
  TXeleOrderIDType                TradeID;
  ///交易所类型
  TXeleMarketType                 Market;
  ///备兑标志(期权使用)
  TXeleCoveredFlagType            CoveredOrUncovered;
  ///柜台报单编号int类型
  TXeleOrderIDType                OrderSysID;
  ///交易所报单编号
  TXeleOrderExchangeIDType        OrderExchangeID;
  ///买卖方向
  TXeleDirectionType              Direction;
  ///开平标志，股票不填
  TXeleOffsetFlagType             CmbOffsetFlag;
  ///佣金
  TXeleSecuritiesCommissionType   Commission;
  ///印花税
  TXeleStampTaxRateType           StampTax;
  ///过户费
  TXeleTransferFeeRateType        Transfer;
  ///总手续费
  TXeleTotalFeeType               TotalFee;
  ///保证金，暂未使用
  TXeleMoneyType                  Margin;
  ///权利金，暂未使用
  TXeleMoneyType                  Premium;
  ///投资者账号
  TXeleUserIDType                 AccountID;
  ///订单状态
  TXeleOrderStatusType            OrderStatus;
  ///柜台报单编号str类型
  TXeleStrOrderSysIDType          StrOrderSysID;
  /// 交易所执行编号
  TXeleOrderExchangeIDType        ExecID;
  ///客户端登录子节点
  TXeleSubClientIndexType         SubClientIndex;
  ///分页查询结束值 用来作为下次查询的起始值，来实现分页
  TXeleVolumeType                 EndNum;
  ///分页查询条件下，是否可以再次进行查询 只在最后一条回报中有效
  TXeleIsLastType                 QryAgain;
  ///当前条件下查询到的回报总数
  TXeleVolumeType                 TotalNum;
  ///投资者股东账号
  TXeleInvestorIDType             InvestorID;
  ///客户代码
  TXeleUserIDType                 CustID;
  ///报单来源,支持接入网关的柜台,此字段有效
  TXeleOrderSourceType            OrderSource;
  ///预留
  char                            Reserved[42];
};

///费率查询(印花税率、过户费率、佣金率、流量费、最小佣金、多冻值)请求
struct CXeleReqQryStockFeeField {
  ///资金账户
  TXeleUserIDType                 AccountID;
  ///证券代码
  TXeleSecuritiesIDType           SecuritiesID;
  ///营业部代码，暂未使用
  TXeleDepartmentIDType           DepartID;
  ///证券类型
  TXeleSecuritiesType             SecuritiesType;
  ///预留
  char                            Reserved[127];
};

///费率查询(印花税率、过户费率、佣金率、流量费、最小佣金、多冻值)应答
struct CXeleRspQryStockFeeField {
  ///资金账户
  TXeleUserIDType                 AccountID;
  ///证券代码
  TXeleSecuritiesIDType           SecuritiesID;
  ///营业部代码
  TXeleDepartmentIDType           DepartID;
  ///印花税率
  TXeleStampTaxRateType           StampTaxRate;
  ///过户费率
  TXeleTransferFeeRateType        TransferFee;
  ///佣金率
  TXeleSecuritiesCommissionType   CommRate;
  ///流量费
  TXeleTrafficFeeType             TrafficFee;
  ///交易所类型
  TXeleMarketType                 Market;
  ///证券类型
  TXeleSecuritiesType             SecuritiesType;
  ///最小佣金
  TXeleMinCommissionType          MinCommission;
  ///多冻值
  TXeleMoreFreeze                 MoreFreeze;
  ///预留
  char                            Reserved[110];
};

////////Ctrl_F_Stock///////////
///////////////////////////////
/////// 证券相关 //////////////
//////////////////////////////

//集中交易资金调拨艾科柜台请求
struct CXeleReqCapTransferField {
  ///机构代码（客户不填）
  TXeleOrgIDType                  OrgID;
  ///资金账号
  TXeleUserIDType                 AccountID;
  ///币种（客户不填）
  TXeleCurrencyType               Currency;
  ///返还日期（客户不填）
  TXeleRtnDateType                RtnDate;
  ///冻结,解冻金额
  TXeleMoneyType                  Fundamt;
  ///备注(客户不填)
  TXeleRemarkType                 RemarkMsg;
  ///执行标记(内部使用,客户不填)
  TXeleCallOneceFlag              CallOnce;
  ///分支机构（内部使用,客户不填）
  TXeleBranchNoType               BranchNo;
  ///客户代码（内部使用,客户不填）
  TXeleUserIDType                 CustID;
  ///预留
  char                            Reserved[44];
};

//集中交易资金调拨艾科柜台应答
struct CXeleRspCapTransferField {
  ///机构代码
  TXeleOrgIDType                  OrgID;
  ///资金账号
  TXeleUserIDType                 AccountID;
  ///币种
  TXeleCurrencyType               Currency;
  ///操作流水号
  TXeleSnoType                    Sno;
  ///调取前可用(恒生柜台：该字段暂未使用)
  TXeleMoneyType                  BefFundavl;
  ///资金余额
  TXeleMoneyType                  Fundbal;
  ///调整后可用(恒生柜台：该字段暂未使用)
  TXeleMoneyType                  AftFundavl;
  ///调整前可取(恒生柜台：该字段暂未使用)
  TXeleMoneyType                  BefCashbal;
  ///调整后可取(恒生柜台：该字段暂未使用)
  TXeleMoneyType                  AftCashbal;
  ///调入资金,调取资金
  TXeleMoneyType                  Fundamt;
  ///集中交易柜台响应错误码
  TXeleCentralTradingErrorIdType  ctErrorId;
  ///集中交易柜台响应错误信息
  TXeleCentralTradingErrorMsgType ctErrorMsg;
  ///fpga资金是否更新
  TXeleIsUpdateFpgaFundType       IsUpdateFpgaFund;
  ///预留
  char                            Reserved[11];
};

///集中柜台资金调拨记录查询请求
struct CXeleReqQryCapTransferRecordField {
  ///机构代码
  TXeleOrgIDType                  OrgID;
  ///资金账号
  TXeleUserIDType                 AccountID;
  ///预留
  TXeleReserved1Type              Reserved;
};

///集中柜台资金调拨记录查询应答
struct CXeleRspQryCapTransferRecordField {
  ///机构代码
  TXeleOrgIDType                  OrgID;
  ///资金账号
  TXeleUserIDType                 AccountID;
  ///冻结/解冻金额
  TXeleMoneyType                  Fundamt;
  ///操作方向, '1'：冻结，'2':解冻
  TXeleDirectionType              Direction;
  ///操作流水号
  TXeleSnoType                    Sno;
  ///操作时间
  TXeleShortTimeType              Time;
  ///交易所类型
  TXeleMarketType                 Market;
  ///预留
  char                            Reserved[63];
};

///资金账户查询请求(股票、期权共用)
struct CXeleReqQryClientAccountField {
  ///资金账户
  TXeleUserIDType                 AccountID;
  ///预留
  TXeleReservedType               Reserved;
};

///证券资金账户查询应答
struct CXeleRspQryStockClientAccountField {
  ///资金账户
  TXeleUserIDType                 AccountID;
  ///交易账户状态(暂不使用)
  TXeleAcctStatusType             AcctStatus;
  ///冻结资金
  TXeleFrozeMarginType            FrozeCapital;
  ///冻结手续费
  TXeleFrozenFeeType              FrozenFee;
  ///已付手续费
  TXeleUsedFeeType                UsedFee;
  ///初始上场资金（不变）
  TXeleTotalFundType              InitTotalFund;
  ///上场资金（可变）； 初始上场资金 + 出入金额 ，可能为负（建议客户不使用）
  TXeleTotalFundType              TotalFund;
  ///总卖出
  TXeleSellFund                   SellFund;
  ///总买入
  TXeleBuyFund                    BuyFund;
  ///当日盈亏(暂不使用)
  TXeleCurrGainLossType           CurrGainLoss;
  ///总盈亏（暂不使用）
  TXeleTotalGainLossType          TotalGainLoss;
  ///可用资金
  TXeleAvailableFundType          AvailableFund;
  ///交易所类型
  TXeleMarketType                 Market;
  ///投资者股东账号
  TXeleInvestorIDType             InvestorID;
  ///客户代码
  TXeleUserIDType                 CustID;
  ///被证券公司冻结的资金
  TXeleFrozenFundType             FrozenFund;
  ///可取金额(暂不使用)
  TXeleWithdrawableFundType       WithdrawableFund;
  ///预留
  char                            Reserved[86];
};

///持仓查询请求(股票、期权共用)
struct CXeleReqQryPositionField {
  ///资金账户
  TXeleUserIDType                 AccountID;
  ///证券代码
  TXeleSecuritiesIDType           SecuritiesID;
  ///预留
  TXeleReservedType               Reserved;
};

///证券持仓查询应答
struct CXeleRspQryStockPositionField {
  ///资金账户
  TXeleUserIDType                 AccountID;
  ///证券代码
  TXeleSecuritiesIDType           SecuritiesID;
  ///证券类别代码
  TXeleSecuritiesType             SecuritiesType;
  ///预留
  char                            Reserved1[2];
  ///证券子类别代码，暂不使用
  TXeleSecuritiesSubTypeType      SecuritiesSubType;
  ///今买仓
  TXeleTdBuyPositionType          TdBuyPosition;
  ///今卖仓
  TXeleTdSellPositionType         TdSellPosition;
  ///在途买仓
  TXeleUnTdPositionType           UnTdBuyPosition;
  ///在途卖仓
  TXeleUnTdPositionType           UnTdSellPosition;
  ///在途冻结资金，暂不使用
  TXeleUnTdFrozenCapType          UnTdFrozenCap;
  ///昨持仓（不变）
  TXeleYdPositionType             YdPosition;
  ///买入成本，暂不使用
  TXeleYdPositionCostType         YdTotalCost;
  ///昨持仓剩余，暂不使用
  TXeleYdPositionLeftType         YdPositionLeft;
  ///总持仓 = 老仓 + 今买仓  - 今卖仓
  TXeleTotalPositionType          TotalPosition;
  ///持仓成本
  TXeleTotalCostType              TotalCost;
  ///持仓均价，暂不使用
  TXeleAvgPxType                  AvgPrice;
  ///备兑锁定持仓
  TXeleCoveredFrozenPositionType  CoveredFrozenPosition;
  ///交易所类型
  TXeleMarketType                 Market;
  ///现有持仓数量（含未卖持仓）=老仓 + 今买仓 (-+)出入仓 - 今卖仓
  TXeleRemainingPosition          RemainingPosition;
  ///可卖持仓数量
  TXeleAvailablePosition          AvailablePosition;
  ///日内可转交易类型,Y表示T+0，N表示T+1
  TXeleDayTradingType             DayTrading;
  ///股东账户
  TXeleInvestorIDType             InvestorID;
  ///客户代码
  TXeleUserIDType                 CustID;
  //被证券公司冻结的持仓
  TXeleFrozenPositionType         FrozenPosition;
  ///可备兑锁定持仓,允许备兑锁定的持仓数量
  TXeleCoveredFrozenPositionType  AvailableCoverLockPosition;
  ///预留
  char                            Reserved[73];
};

///合约查询请求(股票、期权共用)
struct CXeleReqQrySecuritiesField {
  ///证券代码
  TXeleSecuritiesIDType           SecuritiesID;
  ///预留
  TXeleReservedType               Reserved;
};

///证券合约查询应答
struct CXeleRspQryStockSecuritiesField {
  ///证券代码
  TXeleSecuritiesIDType           SecuritiesID;
  ///合约名称
  TXeleSecuritiesNameType         SecuritiesName;
  ///证券类别代码
  TXeleSecuritiesType             SecuritiesType;
  ///预留
  char                            Reserved1[2];
  ///证券子类别代码，暂不使用
  TXeleSecuritiesSubTypeType      SecuritiesSubType;
  ///最小变动价位
  TXeleMinTickPriceType           TickPrice;
  ///币种，暂不使用
  TXeleCurrencyType               Currency;
  ///证券面值，暂不使用
  TXeleSecuritiesParValue         ParValue;
  ///昨结算价
  TXelePreSettlePriceType         PreSettlePrice;
  ///涨跌幅限制类型
  TXeleLimitPriceClassType        LimitPriceClass;
  ///限价单最大买单量
  TXeleMaxLimitOrderVolumeType    MaxLimitBuyVolume;
  ///限价单最大卖单量
  TXeleMaxLimitOrderVolumeType    MaxLimitSellVolume;
  ///限价单最小买单量
  TXeleMinLimitOrderVolumeType    MinLimitBuyVolume;
  ///限价单最小卖单量
  TXeleMinLimitOrderVolumeType    MinLimitSellVolume;
  ///市价单最大买单量
  TXeleMaxMarketOrderVolumeType   MaxMarketBuyVolume;
  ///市价单最大卖单量
  TXeleMaxMarketOrderVolumeType   MaxMarketSellVolume;
  ///市价单最小买单量
  TXeleMinMarketOrderVolumeType   MinMarketBuyVolume;
  ///市价单最小卖单量
  TXeleMinMarketOrderVolumeType   MinMarketSellVolume;
  ///跌停板价
  TXeleLowerPriceType             LowerPrice;
  ///涨停板价
  TXeleUpperPriceType             UpperPrice;
  ///交易所类型
  TXeleMarketType                 Market;
  ///日内可转交易类型,Y表示T+0，N表示T+1
  TXeleDayTradingType             DayTrading;
  ///股票风险级别 , 0 正常, 1 风险警示, 2 退市整理期
  TXeleStockLevel                StockLevel;
  ///最小报单数量单位
  TXeleLotSize                    LotSize;
  ///预留
  char                            Reserved[121];
};

///证券新股配股额度查询请求
struct CXeleReqQryStockQuotaField{
  ///资金账号
  TXeleUserIDType              AccountID;
  ///合约类型
  TXeleSecuritiesType          SecuritiesType;
  ///证券代码
  TXeleSecuritiesIDType        SecuritiesID;
  ///预留
  TXeleReservedType            Reserved;
};

///证券新股配股额度查询响应  
struct CXeleRspQryStockQuotaField {
  ///资金账号
  TXeleUserIDType              AccountID;
  ///合约类型
  TXeleSecuritiesType          SecuritiesType;
  ///证券代码
  TXeleSecuritiesIDType        SecuritiesID;
  ///新股配股额度
  TXeleStockQuotaType          StockQuota;
  ///新股配股已申购额度
  TXeleStockQuotaType          StockHoldQuota;
  ///新股配股剩余额度
  TXeleStockQuotaType          StockAvlQuota;
  ///交易所类型
  TXeleMarketType              Market;
  ///账户级主板新股配股总额度
  TXeleStockQuotaType          MainBoardStockQuota;
  ///账户级科创板新股配股总额度
  TXeleStockQuotaType          TechBoardStockQuota;
  ///预留
  char                        Reserved[111];
};

///证券集中交易资金查询请求
struct CXeleReqQryCentralTradingFundField{
  ///机构代码（客户不填）
  TXeleOrgIDType OrgID;
  ///客户代码（客户不填）
  TXeleUserIDType CustId;
  ///资金账号
  TXeleUserIDType AccountID;
  ///币种（客户不填）
  TXeleCurrencyType Currency;
  ///备注
  TXeleRemarkType RemarkMsg;
  ///预留
  TXeleReservedType Reserved;
};

///证券集中交易资金查询响应  
struct CXeleRspQryCentralTradingFundField {
  ///机构代码
  TXeleOrgIDType OrgID;
  ///客户代码
  TXeleUserIDType CustId;
  ///资金账号 
  TXeleUserIDType AccountID;
  ///币种
  TXeleCurrencyType Currency;
  ///资金余额
  TXeleMoneyType Fundbal;
  ///资金可用
  TXeleMoneyType Fundavl;
  ///资金资产
  TXeleMoneyType Fund;
  ///主资金标志
  TXeleFundType Fundseq;
  ///预留   128字节
  TXeleReservedType Reserved;
};

///出入金明细查询请求
struct CXeleReqQryCapTransferDetailsField {
  ///资金账号
  TXeleUserIDType                 AccountID;
  ///预留
  char                            Reserved[32];
};

///出入金明细查询应答
struct CXeleRspQryCapTransferDetailsField {
  ///资金账号
  TXeleUserIDType                 AccountID;
  ///流水数量
  TXeleFlowCountType              FlowCount;
  ///交易所类型
  TXeleMarketType                 Market;
  ///预留
  char 						      Reserved[63];
};

///出入金明细回报
struct CXeleRtnCapTransferDetailsField {
  ///资金账号
  TXeleUserIDType                 AccountID;
  ///调拨金额
  TXeleMoneyType                  Fundamt;
  ///操作方向，'1'：调入，'2':调出
  TXeleTransferDirectionType      Direction;
  ///操作时间
  TXeleShortTimeType              Time;
  ///当前快速柜台系统ID(柜台间调拨使用)
  TXeleTradeSystemIDType          SystemID;
  ///另一个快速柜台系统ID(柜台间调拨使用)
  TXeleTradeSystemIDType          SystemID1;
  ///可用资金
  TXeleMoneyType                  Available;
  ///客户端登录子节点
  TXeleSubClientIndexType         SubClientIndex;
  ///流水重构报文标记
  TXeleRecoveryFlagType           RecoveryFlag;
  ///交易所类型
  TXeleMarketType                 Market;
  ///预留
  char                            Reserved[31];
};

////////Ctrl_F_Option///////////
///////////////////////////////
/////// 期权相关 //////////////
//////////////////////////////

///流水重构结束响应
struct CXeleRspRebuildFinishField {
  ///资金账户
  TXeleUserIDType                 AccountID;
};

///期权成份合约扩展结构体
struct CXeleCombLegField {
  ///成份证券代码
  TXeleSecuritiesIDType           LegSecuritiesID;
  ///成份合约方向
  TXeleLegSideType                LegSide;
  ///备兑标志(期权使用)
  TXeleCoveredFlagType            CoveredOrUncovered;
  ///成分合约数量
  TXeleShortVolumeType            Volume;
};

///期权多腿扩展结构体
struct CXeleCombOrderField {
  ///申报类型, '1':组合, '2':解组
  TXeleCombOrderType              CmbOrderType;
  ///组合策略类型
  TXeleStrategyCombType           StgyCmbType;
  ///组合策略流水号
  TXeleSecondaryOrderType         SecondaryOrderID;
  ///成分合约个数（最多4腿）
  TXeleSumLegsType                SumLegs;
  ///成分合约扩展结构体
  CXeleCombLegField               CombLeg[4];
  ///预留
  char                            Reserved[16];
};

///单腿行权请求
struct CXeleReqExerciseInsertField {
  ///用户本地报单编号
  TXeleOrderIDType				  UserLocalID;
  ///合约编码
  TXeleSecuritiesIDType			  SecuritiesID;
  ///行权数量
  TXeleVolumeType				  Volume;
  ///订单所有类型
  TXeleTradeOwnerType	          OwnerType;
  ///业务单元(用户定义)
  TXeleBusinessUnitType		      BusinessUnit;
  ///委托方式
  TXeleOperwayType                Operway;
  ///预留
  char							  Reserved[20];
  ///错误编号，拒单使用
  TXeleErrorIdType                ErrorId;

};

///单腿行权应答
struct CXeleRspExerciseInsertField {
  ///柜台报单编号int类型
  TXeleOrderIDType                OrderSysID;
  ///用户本地报单编号
  TXeleOrderIDType                UserLocalID;
  ///合约编码
  TXeleSecuritiesIDType           SecuritiesID;
  ///行权数量
  TXeleVolumeType                 Volume;
  ///订单所有类型
  TXeleTradeOwnerType	          OwnerType;
  ///客户端登录子节点
  TXeleSubClientIndexType         SubClientIndex;
  ///业务单元(用户定义)
  TXeleBusinessUnitType           BusinessUnit;
  ///柜台报单编号str类型
  TXeleStrOrderSysIDType          StrOrderSysID;
  ///交易所类型
  TXeleMarketType                 Market;
  ///交易前置id(暂不使用)
  TXeleExchangeIDType             ExchangeFrontID;
  ///交易所报单编号
  TXeleOrderExchangeIDType        OrderExchangeID;
  ///投资者账号
  TXeleUserIDType                 AccountID;
  ///预留
  char                            Reserved[32];
};

///单腿行权报单回报
struct CXeleRtnExerciseOrderField {
  ///柜台报单编号int类型
  TXeleOrderIDType                OrderSysID;
  ///用户本地报单编号
  TXeleOrderIDType				  UserLocalID;
  ///合约编码
  TXeleSecuritiesIDType			  SecuritiesID;
  ///行权数量
  TXeleVolumeType				  Volume;
  ///交易所报单编号
  TXeleOrderExchangeIDType        OrderExchangeID;
  ///订单所有类型
  TXeleTradeOwnerType	          OwnerType;
  ///订单状态
  TXeleOrderStatusType            OrderStatus;
  ///客户端登录子节点
  TXeleSubClientIndexType         SubClientIndex;
  ///业务单元(用户定义)
  TXeleBusinessUnitType           BusinessUnit;
  ///接受请求时间
  TXeleTimeType                   TransactTime;
  ///流水重构报文标记
  TXeleRecoveryFlagType           RecoveryFlag;
  ///柜台报单编号str类型
  TXeleStrOrderSysIDType          StrOrderSysID;
  ///交易所类型
  TXeleMarketType                 Market;
  ///交易前置id(暂不使用)
  TXeleExchangeIDType             ExchangeFrontID;
  ///投资者账号
  TXeleUserIDType                 AccountID;
  ///预留
  char                            Reserved[32];
};

///组合行权报单请求
struct CXeleReqExerciseCombInsertField {
  ///用户本地报单编号
  TXeleOrderIDType				  UserLocalID;
  ///成分合约编码1
  TXeleSecuritiesIDType			  LegSecuritiesID1;
  ///成分合约编码2
  TXeleSecuritiesIDType			  LegSecuritiesID2;
  ///成分合约数量1
  TXeleShortVolumeType		      LegVolume1;
  ///成分合约数量2
  TXeleShortVolumeType			  LegVolume2;
  ///订单所有类型
  TXeleTradeOwnerType	          OwnerType;
  ///行权指令合并申报数量
  TXeleShortVolumeType			  OfferVolume;
  ///业务单元(用户定义)
  TXeleBusinessUnitType		      BusinessUnit;
  ///委托方式
  TXeleOperwayType                Operway;
  ///交易前置id(暂不使用)
  TXeleExchangeIDType             ExchangeFrontID;
  ///合约标的证券代码(深交期权使用)
  TXeleSecuritiesIDType           UnderlyingSecuritiesID;
  ///预留
  char							  Reserved;
  ///错误编号，拒单使用
  TXeleErrorIdType                ErrorId;
};

///组合行权报单应答
struct CXeleRspExerciseCombInsertField {
  ///柜台报单编号int类型
  TXeleOrderIDType                OrderSysID;
  ///用户本地报单编号
  TXeleOrderIDType				  UserLocalID;
  ///成分合约编码1
  TXeleSecuritiesIDType			  LegSecuritiesID1;
  ///成分合约编码2
  TXeleSecuritiesIDType			  LegSecuritiesID2;
  ///成分合约数量1
  TXeleShortVolumeType		      LegVolume1;
  ///成分合约数量2
  TXeleShortVolumeType			  LegVolume2;
  ///订单所有类型
  TXeleTradeOwnerType	          OwnerType;
  ///行权指令合并申报单位数量
  TXeleShortVolumeType			  OfferVolume;
  ///错误编号，拒单使用
  TXeleErrorIdType                ErrorId;
  ///客户端登录子节点
  TXeleSubClientIndexType         SubClientIndex;
  ///业务单元(用户定义)
  TXeleBusinessUnitType           BusinessUnit;
  ///柜台报单编号str类型
  TXeleStrOrderSysIDType          StrOrderSysID;
  ///交易所类型
  TXeleMarketType                 Market;
  ///交易前置id(暂不使用)
  TXeleExchangeIDType             ExchangeFrontID;
  ///交易所报单编号
  TXeleOrderExchangeIDType        OrderExchangeID;
  ///投资者账号
  TXeleUserIDType                 AccountID;
  ///预留
  char                            Reserved[32];
};

///组合行权报单回报
struct CXeleRtnExerciseCombOrderField {
  ///柜台报单编号int类型
  TXeleOrderIDType                OrderSysID;
  ///用户本地报单编号
  TXeleOrderIDType				  UserLocalID;
  ///成分合约编码1
  TXeleSecuritiesIDType			  LegSecuritiesID1;
  ///成分合约编码2
  TXeleSecuritiesIDType			  LegSecuritiesID2;
  ///成分合约数量1
  TXeleShortVolumeType		      LegVolume1;
  ///成分合约数量2
  TXeleShortVolumeType			  LegVolume2;
  ///订单所有类型
  TXeleTradeOwnerType	          OwnerType;
  ///行权指令合并申报数量
  TXeleShortVolumeType			  OfferVolume;
  ///交易所报单编号
  TXeleOrderExchangeIDType        OrderExchangeID;
  ///成交数量
  TXeleVolumeType                 TradeVolume;
  ///未成交手数（报单数量-累计成交数量）
  TXeleVolumeType                 LeavesVolume;
  ///订单状态
  TXeleOrderStatusType            OrderStatus;
  ///客户端登录子节点
  TXeleSubClientIndexType         SubClientIndex;
  ///业务单元(用户定义)
  TXeleBusinessUnitType           BusinessUnit;
  ///接受请求时间
  TXeleTimeType                   TransactTime;
  ///流水重构报文标记
  TXeleRecoveryFlagType           RecoveryFlag;
  ///柜台报单编号str类型
  TXeleStrOrderSysIDType          StrOrderSysID;
  ///交易所类型
  TXeleMarketType                 Market;
  ///交易前置id(暂不使用)
  TXeleExchangeIDType             ExchangeFrontID;
  ///投资者账号
  TXeleUserIDType                 AccountID;
  ///预留
  char                            Reserved[20];
};

///期权组合及解组报单请求
struct CXeleReqOptionCombInsertField {
  ///用户本地报单编号
  TXeleOrderIDType                UserLocalID;
  ///报单模式
  TXeleOrderMode	              OrderMode;
  ///订单所有类型
  TXeleTradeOwnerType	          OwnerType;
  ///报单数量
  TXeleShortVolumeType            CombVolume;
  ///申报类型, '1':组合, '2':解组
  TXeleCombOrderType              CmbOrderType;
  ///组合策略类型
  TXeleStrategyCombType           StgyCmbType;
  ///组合策略流水号
  ///组合时,填全0
  ///解组时,若为上交，则填写组合成交回报中期权多腿组合信息中的SecondaryOrderID, 详参demo
  ///解组时,若为深交，则填写组合报单回报中的OrderExchangeID, 详参demo
  TXeleSecondaryOrderType         SecondaryOrderID;
  ///成分合约个数（最多4腿）, 深交拆分时填0
  TXeleSumLegsType                SumLegs;
  ///成分合约扩展结构体, 深交拆分无需填写
  CXeleCombLegField               CombLeg[4];
  ///业务单元，供客户自身业务使用
  TXeleBusinessUnitType           BusinessUnit;
  ///委托方式
  TXeleOperwayType                Operway;
  ///交易前置id(暂不使用)
  TXeleExchangeIDType             ExchangeFrontID;
  ///合约标的证券代码(深交期权使用)
  TXeleSecuritiesIDType           UnderlyingSecuritiesID;
  ///预留
  char                            Reserved[19];
};

///期权组合及解组报单应答
struct CXeleRspOptionCombInsertField {
  ///柜台报单编号int类型
  TXeleOrderIDType                OrderSysID;
  ///用户本地报单编号
  TXeleOrderIDType                UserLocalID;
  ///报单模式
  TXeleOrderMode	              OrderMode;
  ///订单所有类型
  TXeleTradeOwnerType	          OwnerType;
  ///报单数量
  TXeleShortVolumeType            CombVolume;
  ///申报类型, '1':组合, '2':解组
  TXeleCombOrderType              CmbOrderType;
  ///组合策略类型
  TXeleStrategyCombType           StgyCmbType;
  ///暂不使用
  TXeleSecondaryOrderType         SecondaryOrderID;
  ///成分合约个数（最多4腿）
  TXeleSumLegsType                SumLegs;
  ///成分合约扩展结构体
  CXeleCombLegField               CombLeg[4];
  ///客户端登录子节点
  TXeleSubClientIndexType         SubClientIndex;
  ///业务单元，供客户自身业务使用
  TXeleBusinessUnitType           BusinessUnit;
  ///柜台报单编号str类型
  TXeleStrOrderSysIDType          StrOrderSysID;
  ///交易所类型
  TXeleMarketType                 Market;
  ///交易前置id(暂不使用)
  TXeleExchangeIDType             ExchangeFrontID;
  ///交易所报单编号
  TXeleOrderExchangeIDType        OrderExchangeID;
  ///投资者账号
  TXeleUserIDType                 AccountID;
  ///预留
  char                            Reserved[32];
};

///多腿组合策略报单回报
struct CXeleRtnOptionCombOrderField {
  ///柜台报单编号int类型
  TXeleOrderIDType                OrderSysID;
  ///用户本地报单编号
  TXeleOrderIDType                UserLocalID;
  ///报单模式
  TXeleOrderMode	              OrderMode;
  ///订单所有类型
  TXeleTradeOwnerType	          OwnerType;
  ///报单数量
  TXeleShortVolumeType            CombVolume;
  ///接受请求时间
  TXeleTimeType                   TransactTime;
  ///成交数量
  TXeleVolumeType                 TradeVolume;
  ///未成交手数（报单数量-累计成交数量）
  TXeleVolumeType                 LeavesVolume;
  ///订单状态
  TXeleOrderStatusType            OrderStatus;
  ///保证金，暂未使用
  TXeleMoneyType                  Margin;
  ///冻结权利金，暂未使用
  TXeleMoneyType                  FrozenPremium;
  ///冻结手续费，暂未使用
  TXeleMoneyType                  FrozenFee;
  ///客户端登录子节点
  TXeleSubClientIndexType         SubClientIndex;
  ///业务单元，供客户自身业务使用
  TXeleBusinessUnitType           BusinessUnit;
  ///期权多腿组合信息
  CXeleCombOrderField             CombOrderInfo;
  // 流水重构报文标记
  TXeleRecoveryFlagType           RecoveryFlag;
  ///柜台报单编号str类型
  TXeleStrOrderSysIDType          StrOrderSysID;
  ///交易所类型
  TXeleMarketType                 Market;
  ///交易前置id(暂不使用)
  TXeleExchangeIDType             ExchangeFrontID;
  /// 交易所报单编号
  TXeleOrderExchangeIDType        OrderExchangeID;
  ///投资者账号
  TXeleUserIDType                 AccountID;
  ///预留
  char                            Reserved[75];
};

///多腿组合策略成交回报
struct CXeleRtnOptionCombTradeField {
  ///柜台报单编号int类型
  TXeleOrderIDType                OrderSysID;
  ///用户本地报单编号
  TXeleOrderIDType                UserLocalID;
  ///报单模式
  TXeleOrderMode	              OrderMode;
  ///订单所有类型
  TXeleTradeOwnerType	          OwnerType;
  ///报单数量
  TXeleShortVolumeType            CombVolume;
  ///接受请求时间
  TXeleTimeType                   TransactTime;
  ///成交数量
  TXeleVolumeType                 TradeVolume;
  ///未成交手数（报单数量-累计成交数量）
  TXeleVolumeType                 LeavesVolume;
  ///订单状态
  TXeleOrderStatusType            OrderStatus;
  ///保证金，暂未使用
  TXeleMoneyType                  Margin;
  ///冻结权利金，暂未使用
  TXeleMoneyType                  FrozenPremium;
  ///冻结手续费，暂未使用
  TXeleMoneyType                  FrozenFee;
  ///客户端登录子节点
  TXeleSubClientIndexType         SubClientIndex;
  ///业务单元，供客户自身业务使用
  TXeleBusinessUnitType           BusinessUnit;
  ///期权多腿组合信息
  CXeleCombOrderField             CombOrderInfo;
  // 流水重构报文标记
  TXeleRecoveryFlagType           RecoveryFlag;
  ///柜台报单编号str类型
  TXeleStrOrderSysIDType          StrOrderSysID;
  ///交易所类型
  TXeleMarketType                 Market;
  ///交易前置id(暂不使用)
  TXeleExchangeIDType             ExchangeFrontID;
  ///投资者账号
  TXeleUserIDType                 AccountID;
  ///预留
  char                            Reserved[91];
};


///(期权)证券锁定/解锁请求
struct CXeleReqSecuritiesLockField {
  ///用户本地报单编号
  TXeleOrderIDType                UserLocalID;
  ///合约标的证券代码
  TXeleSecuritiesIDType           UnderlyingSecuritiesID;
  ///现货持仓数量
  TXeleVolumeType                 OrderQty;
  ///锁定/解锁
  TXeleDirectionType              Side;
  ///业务单元，供客户自身业务使用
  TXeleBusinessUnitType           BusinessUnit;
  ///委托方式
  TXeleOperwayType                Operway;
  ///预留
  char                            Reserved[20];
  ///预留
  TXeleErrorIdType                ErrorId;
};

///(期权)证券锁定/解锁应答
struct CXeleRspSecuritiesLockField {
  ///柜台报单编号int类型
  TXeleOrderIDType                OrderSysID;
  ///用户本地报单编号
  TXeleOrderIDType                UserLocalID;
  ///合约标的证券代码
  TXeleSecuritiesIDType           UnderlyingSecuritiesID;
  ///现货持仓数量
  TXeleVolumeType                 OrderQty;
  ///锁定/解锁
  TXeleDirectionType              Side;
  ///错误编号，拒单使用
  TXeleErrorIdType                ErrorId;
  ///客户端登录子节点
  TXeleSubClientIndexType         SubClientIndex;
  ///业务单元(用户定义)
  TXeleBusinessUnitType           BusinessUnit;
  ///柜台报单编号str类型
  TXeleStrOrderSysIDType          StrOrderSysID;
  ///投资者账号
  TXeleUserIDType                 AccountID;
  ///交易所类型
  TXeleMarketType                 Market;
  ///预留
  char                            Reserved[42];
};

///(期权)证券锁定/解锁回报
struct CXeleRtnSecuritiesLockField {
  ///用户本地报单编号
  TXeleOrderIDType                UserLocalID;
  ///柜台报单编号int类型
  TXeleOrderIDType                OrderSysID;
  ///交易所报单编号
  TXeleOrderExchangeIDType        OrderExchangeID;
  ///合约标的证券代码
  TXeleSecuritiesIDType           UnderlyingSecuritiesID;
  ///现货持仓数量
  TXeleVolumeType                 OrderQty;
  ///锁定/解锁
  TXeleDirectionType              Side;
  ///接受请求时间
  TXeleTimeType                   TransactTime;
  ///订单状态
  TXeleOrderStatusType            OrderStatus;
  ///客户端登录子节点
  TXeleSubClientIndexType         SubClientIndex;
  ///业务单元(用户定义)
  TXeleBusinessUnitType           BusinessUnit;
  ///投资者账号
  TXeleUserIDType                 AccountID;
  ///交易所类型
  TXeleMarketType                 Market;
  ///柜台报单编号str类型
  TXeleStrOrderSysIDType          StrOrderSysID;
  ///预留
  char                            Reserved[54];
};

///期权保证金费率查询请求
struct CXeleReqQryOptionMarginFeeField {
  ///资金账户
  TXeleUserIDType                 AccountID;
  ///证券代码
  TXeleSecuritiesIDType           SecuritiesID;
  ///营业部代码，暂不使用
  TXeleDepartmentIDType           DepartID;
  ///预留
  TXeleReservedType               Reserved;
};

///期权保证金费率查询应答
struct CXeleRspQryOptionMarginFeeField {
  ///资金账户
  TXeleUserIDType                 AccountID;
  ///证券代码
  TXeleSecuritiesIDType           SecuritiesID;
  ///营业部代码，暂不使用
  TXeleDepartmentIDType           DepartID;
  ///佣金费率
  TXeleMoneyType                  CommissionRate;
  ///结算费，暂不使用
  TXeleMoneyType                  SettlementFee;
  ///行权费，暂不使用
  TXeleMoneyType                  ExerciseFee;
  ///保证金率
  TXeleMoneyType                  MarginRate;
  ///TXeleEntrustTypeType委托类型
  TXeleEntrustTypeType            EntrustType;
  ///TXeleChargingTypeType收费方式
  TXeleChargingTypeType           ChangingType;
  ///交易所类型
  TXeleMarketType                 Market;
  ///预留
  char                            Reserved[133];
};

///期权组合持仓查询请求
struct CXeleReqQryOptionCombPositionField {
  ///资金账号
  TXeleUserIDType                 AccountID;
  ///证券代码
  TXeleSecuritiesIDType           SecuritiesID;
  ///预留
  char                            Reserved[120];
};

///期权组合持仓查询应答
struct CXeleRspQryOptionCombPositionField {
  ///资金账号
  TXeleUserIDType                 AccountID;
  ///组合策略流水号,若为空,表示组合未成交
  TXeleSecondaryOrderType         SecondaryOrderID;
  ///组合策略类型
  TXeleStrategyCombType           StgyCmbType;
  ///组合保证金,暂不使用
  TXeleMoneyType                  Margin;
  ///组合成功数量
  TXeleVolumeType                 CombVolume;
  ///解组成功数量
  TXeleVolumeType                 UnCombVolume;
  ///初始组合保证金,暂不使用
  TXeleMoneyType                  InitMargin;
  ///初始组合数量(仅老仓有值，可用来标识新老仓)
  TXeleVolumeType                 InitCombVolume;
  ///成分合约个数（最多4腿, 暂时支持2腿）
  TXeleSumLegsType                SumLegs;
  ///成份合约扩展
  CXeleCombLegField               CombLeg[4];
  ///交易所类型
  TXeleMarketType                 Market;
  ///在途组合数量
  TXeleVolumeType                 UnTdCombVolume;
  ///在途解组数量
  TXeleVolumeType                 UnTdUnCombVolume;
  ///柜台报单编号str类型, 组合未成交时填写
  TXeleStrOrderSysIDType          StrOrderSysID;
  ///预留
  char                            Reserved[85];
};

///期权资金账户查询应答
struct CXeleRspQryOptionClientAccountField {
  ///资金账户
  TXeleUserIDType                 AccountID;
  ///交易账户状态
  TXeleAcctStatusType             AcctStatus;
  ///保证金
  TXeleMoneyType                  Margin;
  ///冻结保证金
  TXeleMoneyType                  FrozenMargin;
  ///权利金
  TXeleMoneyType                  Premium;
  ///冻结权利金
  TXeleMoneyType                  FrozenPremium;
  ///手续费
  TXeleMoneyType                  Fee;
  ///冻结手续费
  TXeleMoneyType                  FrozenFee;
  ///持仓盈亏
  TXeleMoneyType                  PosGainLoss;
  ///平仓盈亏
  TXeleMoneyType                  CloseGainLoss;
  ///可用资金
  TXeleMoneyType                  Available;
  ///出金
  TXeleMoneyType                  Withdraw;
  ///入金
  TXeleMoneyType                  Deposit;
  ///初始上场资金（不变）
  TXeleTotalFundType              InitTotalFund;
  ///上场资金（可变）
  TXeleTotalFundType              TotalFund;
  ///初始保证金（不变）
  TXeleMoneyType                  DayMargin;
  ///交易所类型
  TXeleMarketType                 Market;
  ///总限额
  TXeleMoneyType                  TotalQuota;
  ///已用额度
  TXeleMoneyType                  UsedQuota;
  ///预留
  char                            Reserved[112];
};

///期权持仓查询应答
struct CXeleRspQryOptionPositionField {
  ///资金账户
  TXeleUserIDType                 AccountID;
  ///证券代码
  TXeleSecuritiesIDType           SecuritiesID;
  ///期权类型，暂不使用
  TXeleOptionsTypeType            OptionType;
  ///标的证券代码，暂不使用
  TXeleSecuritiesIDType           UnderlySecuritiesID;
  ///标的证券名称，暂不使用
  TXeleSecuritiesNameType         UnderlySecuritiesName;
  ///行权日期，暂不使用
  TXeleExerciseDayType            ExerciseDay;
  ///权利仓日初持仓
  TXelePositionType               InitLongYdPosition;
  ///义务仓日初持仓
  TXelePositionType               InitShortYdPosition;
  ///期权市值，暂不使用
  TXeleMoneyType                  OptionMarketValue;
  ///当前数量，暂不使用
  TXelePositionType               CurrentPosition;
  ///可用数量，暂不使用
  TXelePositionType               AvailablePosition;
  ///权利仓日初持仓剩余
  TXelePositionType               LongYdPosition;
  ///权利仓今日持仓
  TXelePositionType               LongPosition;
  ///义务仓日初持仓剩余
  TXelePositionType               ShortYdPosition;
  ///义务仓今日持仓
  TXelePositionType               ShortPosition;
  ///开仓成交数量，暂不使用
  TXelePositionType               TdOpenPosition;
  ///在途开仓数量
  TXeleUnTdPositionType           UnTdOpenPosition;
  ///平仓成交数量，暂不使用
  TXelePositionType               TdClosePosition;
  ///在途平仓数量
  TXeleUnTdPositionType           UnTdClosePosition;
  ///保证金，暂不使用
  TXeleMoneyType                  Margin;
  ///今日开仓保证金，暂不使用
  TXeleMoneyType                  OpenMargin;
  ///今日平仓保证金，暂不使用
  TXeleMoneyType                  CloseMargin;
  ///开仓均价，暂不使用
  TXeleAveragePriceType           AvgPrice;
  ///在途冻结资金，暂不使用
  TXeleUnTdFrozenCapType          UnTdFrozenCap;
  ///在途冻结权利金，暂不使用
  TXeleUnTdFrozenPremiumType      UnTdFrozenPrem;
  ///交易所类型
  TXeleMarketType                 Market;
  ///成交持仓比(标的物查询使用)
  TXeleSpeculationRatioType       SpecRatio;
  ///权力仓成本价
  TXeleAveragePriceType           AvgBoughtPrice;
  ///义务仓成本价
  TXeleAveragePriceType           AvgSoldPrice;
  ///在途买开数量
  TXelePositionType               UnTdOpenBoughtPos;
  ///在途卖平数量
  TXelePositionType               UnTdCloseSellPos;
  ///组合权利仓
  TXelePositionType               LongCombPosition;
  ///组合义务仓
  TXelePositionType               ShortCombPosition;
  ///备兑持仓
  TXelePositionType               CoveredPos;
  ///在途备兑开仓
  TXelePositionType               UnTdOpenCoveredPos;
  ///在途备兑平仓
  TXelePositionType               UnTdCloseCoveredPos;
  ///预留
  char                            Reserved[47];
};

///期权合约查询应答
struct CXeleRspQryOptionSecuritiesField {
  ///证券代码
  TXeleSecuritiesIDType           SecuritiesID;
  ///合约交易代码
  TXeleTradeOptionCodeType        TradeOptionCode;
  ///合约名称
  TXeleSecuritiesNameType         SecuritiesName;
  ///期权类型
  TXeleOptionsTypeType            OptionType;
  ///标的证券代码
  TXeleSecuritiesIDType           UnderlySecuritiesID;
  ///标的证券名称
  TXeleSecuritiesNameType         UnderlySecuritiesName;
  ///标的交易品种
  TXeleUnderlyingType             UnderlySecuritiesType;
  ///标的交易子品种，暂不使用
  TXeleSecuritiesSubTypeType      UnderlySecuritiesSubType;
  ///涨跌标志（C/P）
  TXeleCPFlagType                 CPFlag;
  ///期权行权价
  TXeleExercisePriceType          ExercisePrice;
  ///期权行权日
  TXeleShortTimeType              ExerciseDate;
  ///期权交割日
  TXeleShortTimeType              DeliveryDate;
  ///合约乘数
  TXeleMultiUnitType              MultiUnit;
  ///产品代码，暂不使用
  TXeleProductIDType              ProductID;
  ///最小变动价位
  TXeleMinTickPriceType           TickPrice;
  ///当前合约未平仓数
  TXeleUncoveredPositionType      UncoveredPosition;
  ///昨结算价
  TXelePreSettlePriceType         PreSettlePrice;
  ///标的证券前收盘价
  TXeleUnderlyPreClosePriceType   UnderlyPreClosePrice;
  ///单位保证金
  TXeleMarginUnitType             MarginUnit;
  ///涨跌幅限制类型(无限制，有限制)
  TXeleLimitPriceClassType        LimitPriceClass;
  ///限价单最大买单量
  TXeleMaxLimitOrderVolumeType    MaxLimitBuyVolume;
  ///限价单最大卖单量
  TXeleMaxLimitOrderVolumeType    MaxLimitSellVolume;
  ///限价单最小买单量
  TXeleMinLimitOrderVolumeType    MinLimitBuyVolume;
  ///限价单最小卖单量
  TXeleMinLimitOrderVolumeType    MinLimitSellVolume;
  ///市价单最大买单量
  TXeleMaxMarketOrderVolumeType   MaxMarketBuyVolume;
  ///市价单最大卖单量
  TXeleMaxMarketOrderVolumeType   MaxMarketSellVolume;
  ///市价单最小买单量
  TXeleMinMarketOrderVolumeType   MinMarketBuyVolume;
  ///市价单最小卖单量
  TXeleMinMarketOrderVolumeType   MinMarketSellVolume;
  ///昨收盘价
  TXeleYdClosePxType              YdClosePrice;
  ///昨结算价
  TXeleYdSettlePxType             YdSettlePrice;
  ///跌停板价
  TXeleLowerPriceType             LowerPrice;
  ///涨停板价
  TXeleUpperPriceType             UpperPrice;
  ///首交易日
  TXeleDateType                   StartDate;
  ///最后交易日
  TXeleDateType                   EndDate;
  ///过期日
  TXeleDateType                   ExpireDate;
  ///交易所类型
  TXeleMarketType                 Market;
  ///资金账号
  TXeleUserIDType                 AccountID;
  ///预留
  char                            Reserved[113];
};

////////Ctrl_F_Opetion_2///////
//////////////////////////////
/////// 当前版本暂不支持(期权) //
//////////////////////////////
/*
 *待版本支持后，请移动到期权标记中
 * */

///期权被撤销订单信息
struct CXeleCancelledOrderInfo {
  ///柜台报单编号int类型
  TXeleOrderIDType                OrderSysID;
  ///撤单成功数量
  TXeleVolumeType                 Volume;
  ///买卖方向， 1表示买 2表示卖
  TXeleDirectionType              Side;
};

///(期权)会员申请转处置证券账户报单 (当前版本暂不支持)
struct CXeleReqOptionDisposalField {
  ///用户本地报单编号
  TXeleOrderIDType                UserLocalID;
  ///合约标的证券代码
  TXeleSecuritiesIDType           UnderlyingSecuritiesID;
  ///申报数量
  TXeleVolumeType                 Volume;
  ///处理类别
  TXeleExerciseMethodType         Method;
  ///订单所有类型
  TXeleTradeOwnerType	          OwnerType;
  ///业务单元，供客户自身业务使用
  TXeleBusinessUnitType           BusinessUnit;
  ///委托方式
  TXeleOperwayType                Operway;
  ///预留
  char                            Reserved[19];
  ///预留
  TXeleErrorIdType                ErrorId;
};

///(期权)会员申请转处置证券账户响应 (当前版本暂不支持)
struct CXeleRspOptionDisposalField {
  ///柜台报单编号int类型
  TXeleOrderIDType                OrderSysID;
  ///用户本地报单编号
  TXeleOrderIDType                UserLocalID;
  ///合约标的证券代码
  TXeleSecuritiesIDType           UnderlyingSecuritiesID;
  ///申报数量
  TXeleVolumeType                 Volume;
  ///处理类别
  TXeleExerciseMethodType         Method;
  ///订单所有类型
  TXeleTradeOwnerType	          OwnerType;
  ///错误编号，拒单使用
  TXeleErrorIdType                ErrorId;
  ///业务单元，供客户自身业务使用
  TXeleBusinessUnitType           BusinessUnit;
  ///客户端登录子节点
  TXeleSubClientIndexType         SubClientIndex;
  ///柜台报单编号str类型
  TXeleStrOrderSysIDType          StrOrderSysID;
  ///投资者账号
  TXeleUserIDType                 AccountID;
  ///交易所类型
  TXeleMarketType                 Market;
  ///预留
  char                            Reserved[64];
};

///(期权)会员申请转处置证券账户回报 (当前版本暂不支持)
struct CXeleRtnOptionDisposalField {
  ///用户本地报单编号
  TXeleOrderIDType                UserLocalID;
  ///柜台报单编号int类型
  TXeleOrderIDType                OrderSysID;
  ///交易所报单编号
  TXeleOrderExchangeIDType        OrderExchangeID;
  ///申报数量
  TXeleVolumeType                 Volume;
  ///处理类别
  TXeleExerciseMethodType         Method;
  ///接受请求时间
  TXeleTimeType                   TransactTime;
  ///订单状态
  TXeleOrderStatusType            OrderStatus;
  ///客户端登录子节点
  TXeleSubClientIndexType         SubClientIndex;
  ///业务单元(用户定义)
  TXeleBusinessUnitType           BusinessUnit;
  ///投资者账号
  TXeleUserIDType                 AccountID;
  ///交易所类型
  TXeleMarketType                 Market;
  ///预留
  char                            Reserved[64];
};

///期权双边报价请求 (当前版本暂不支持)
struct CXeleReqBilateralOrderInsertField {
  ///用户本地报单编号
  TXeleOrderIDType                UserLocalID;
  ///合约代码
  TXeleSecuritiesIDType           SecuritiesID;
  ///订单所有类型
  TXeleTradeOwnerType	          OwnerType;
  ///买报价
  TXelePriceType                  BidPx;
  ///卖报价
  TXelePriceType                  OfferPx;
  ///申报买数量
  TXeleVolumeType                 BidVolume;
  ///申报卖数量
  TXeleVolumeType                 OfferVolume;
  ///买开平标志
  TXeleOffsetFlagType             BidEffectFlag;
  ///卖开平标志
  TXeleOffsetFlagType             OfferEffectFlag;
  ///业务单元，供客户自身业务使用
  TXeleBusinessUnitType           BusinessUnit;
  ///委托方式
  TXeleOperwayType                Operway;
  ///预留
  char                            Reserved[64];
  ///预留
  TXeleErrorIdType                ErrorId;
};

///期权双边报价响应 (当前版本暂不支持)
struct CXeleRspBilateralOrderInsertField {
  ///柜台报单编号int类型
  TXeleOrderIDType                OrderSysID;
  ///用户本地报单编号
  TXeleOrderIDType                UserLocalID;
  ///合约代码
  TXeleSecuritiesIDType           SecuritiesID;
  ///订单所有类型
  TXeleTradeOwnerType	          OwnerType;
  ///买报价
  TXelePriceType                  BidPx;
  ///卖报价
  TXelePriceType                  OfferPx;
  ///申报买数量
  TXeleVolumeType                 BidVolume;
  ///申报卖数量
  TXeleVolumeType                 OfferVolume;
  ///买开平标志
  TXeleOffsetFlagType             BidEffectFlag;
  ///卖开平标志
  TXeleOffsetFlagType             OfferEffectFlag;
  ///错误编号，拒单使用
  TXeleErrorIdType                ErrorId;
  ///业务单元，供客户自身业务使用
  TXeleBusinessUnitType           BusinessUnit;
  ///客户端登录子节点
  TXeleSubClientIndexType         SubClientIndex;
  ///柜台报单编号str类型
  TXeleStrOrderSysIDType          StrOrderSysID;
  ///投资者账号
  TXeleUserIDType                 AccountID;
  ///交易所类型
  TXeleMarketType                 Market;
  ///预留
  char                            Reserved[64];
};

///期权双边报价回报 (当前版本暂不支持)
struct CXeleRtnBilateralOrderField {
  ///用户本地报单编号
  TXeleOrderIDType                UserLocalID;
  ///柜台报单编号int类型
  TXeleOrderIDType                OrderSysID;
  ///交易所报单编号
  TXeleOrderExchangeIDType        OrderExchangeID;
  ///证券代码
  TXeleSecuritiesIDType           SecuritiesID;
  ///订单所有类型
  TXeleTradeOwnerType	          OwnerType;
  ///买报价
  TXelePriceType                  BidPx;
  ///卖报价
  TXelePriceType                  OfferPx;
  ///申报买数量
  TXeleVolumeType                 BidVolume;
  ///申报卖数量
  TXeleVolumeType                 OfferVolume;
  ///买开平标志
  TXeleOffsetFlagType             BidEffectFlag;
  ///卖开平标志
  TXeleOffsetFlagType             OfferEffectFlag;
  ///被撤销买订单信息
  CXeleCancelledOrderInfo         CancelledBidOrder;
  ///被撤销卖订单信息
  CXeleCancelledOrderInfo         CancelledOfferOrder;
  ///订单状态
  TXeleOrderStatusType            OrderStatus;
  ///客户端登录子节点
  TXeleSubClientIndexType         SubClientIndex;
  ///业务单元(用户定义)
  TXeleBusinessUnitType           BusinessUnit;
  ///投资者账号
  TXeleUserIDType                 AccountID;
  ///交易所类型
  TXeleMarketType                 Market;
  ///预留
  char                            Reserved[64];
};

///(期权)保证金查询请求 (当前版本暂不支持)
struct CXeleReqOptionMarginField {
  ///用户本地报单编号
  TXeleOrderIDType                UserLocalID;
  ///保证金账号
  TXeleMarginAcctType             MarginAcct;
  ///业务单元，供客户自身业务使用
  TXeleBusinessUnitType           BusinessUnit;
  ///委托方式
  TXeleOperwayType                Operway;
  ///预留
  char                            Reserved[8];
  ///预留
  TXeleErrorIdType                ErrorId;
};

///(期权)保证金查询响应 (当前版本暂不支持)
struct CXeleRspOptionMarginField {
  ///柜台报单编号int类型
  TXeleOrderIDType                OrderSysID;
  ///用户本地报单编号
  TXeleOrderIDType                UserLocalID;
  ///保证金账号
  TXeleMarginAcctType             MarginAcct;
  ///错误编号，拒单使用
  TXeleErrorIdType                ErrorId;
  ///业务单元，供客户自身业务使用
  TXeleBusinessUnitType           BusinessUnit;
  ///客户端登录子节点
  TXeleSubClientIndexType         SubClientIndex;
  ///柜台报单编号str类型
  TXeleStrOrderSysIDType          StrOrderSysID;
  ///投资者账号
  TXeleUserIDType                 AccountID;
  ///交易所类型
  TXeleMarketType                 Market;
  ///预留
  char                            Reserved[64];
};

///(期权)保证金查询回报 (当前版本暂不支持)
struct CXeleRtnOptionMarginField {
  ///用户本地报单编号
  TXeleOrderIDType                UserLocalID;
  ///柜台报单编号int类型
  TXeleOrderIDType                OrderSysID;
  ///保证金账号
  TXeleMarginAcctType             MarginAcct;
  ///总金额
  TXelePriceType                  TotalMarginAmt;
  ///可用金额
  TXelePriceType                  AvailabeMarginAmt;
  ///订单状态
  TXeleOrderStatusType            OrderStatus;
  ///客户端登录子节点
  TXeleSubClientIndexType         SubClientIndex;
  ///业务单元(用户定义)
  TXeleBusinessUnitType           BusinessUnit;
  ///投资者账号
  TXeleUserIDType                 AccountID;
  ///交易所类型
  TXeleMarketType                 Market;
  ///预留
  char                            Reserved[64];
};

////////Ctrl_F_Other///////////
///////////////////////////////
/////// 其他 //////////////////
//////////////////////////////

///快速登陆请求
struct CXeleFastTcpLoginField {
  ///用户报单编号
  XTSDWORD                           session;
};

///回报流水查询请求
struct CXeleReqQryOrderFlowField {
  ///资金账号
  TXeleUserIDType                 AccountID;
  ///预留
  char                            Reserved[32];
};

///回报流水查询应答
struct CXeleRspQryOrderFlowField {
  ///资金账号
  TXeleUserIDType                 AccountID;
  ///流水数量
  TXeleFlowCountType              FlowCount;
  ///交易所类型
  TXeleMarketType                 Market;
  ///预留
  char 						      Reserved[63];
};

////////Ctrl_F_Manager/////////
///////////////////////////////
/////// 其他 //////////////////
//////////////////////////////
///Manager心跳
struct CXeleHeartBeatManager {
  //心跳间隔
  TXeleTimeIntervalType           Interval;
  //超时时间
  TXeleTimeOutType                Timeout;
  ///预留
  char                            Reserved[1];
};

///Manager登录请求
struct CXeleReqUserLoginManagerField {
  ///资金账户
  TXeleUserIDType                 AccountID;
  ///交易用户密码
  TXeleUserPasswordType           Password;
  ///终端软件AppID
  TXeleAppIDType                  AppID;
  ///终端软件授权码（暂不使用）
  TXeleAuthCodeType               AuthCode;
  ///客户端IP,自动获取
  TXeleClientIpType               ClientIp;
  ///客户端MAC,自动获取
  TXeleClientMacType              ClientMac;
  ///客户端报单IP(暂未使用)
  TXeleClientIpType               OrderIp;
  ///委托方式
  TXeleOperwayType                Operway;
  ///交易所类型
  TXeleMarketType                 Market;
  ///客户端登录子节点
  TXeleSubClientIndexType         SubClientIndex;
  ///站点前缀信息
  TXelePcPreFixType               PcPrefix;
  ///硬盘分区信息
  TXeleHardDiskPartitionType      HdPartitionInfo;
  ///营业部代码
  TXeleOrgIDType                  BusinessCode;
  ///重连校验字段(内部使用)
  TXeleDateType                   TradingDay;
  ///终端软件AppID扩展字段
  TXeleAppIDExtType               AppIDExt;
  ///预留
  char                            Reserved[20];
};

///Manager登录应答
struct CXeleRspUserLoginManagerField {
  ///交易日
  TXeleDateType                   TradingDay;
  ///登录成功时间
  TXeleShortTimeType              LoginTime;
  ///资金账户
  TXeleUserIDType                 AccountID;
  ///会话代码
  TXeleSessionIDType              SessionId;
  ///校验用
  TXeleTokenType                  Token;
  ///委托方式
  TXeleOperwayType                Operway;
  ///柜台连接地址
  TXeleCounterURL                 CounterUrl;
  ///客户端登录子节点
  TXeleSubClientIndexType         SubClientIndex;
  ///心跳时间
  TXeleHeartBeatInterval          HeartBeatInterval;
  ///心跳超时时间
  TXeleHeartBeatTimeout           HeartBeatTimeout;
  ///登录的是互联网端口(内部使用)
  TXeleInternetType               IsInternetConnect;
  ///预留
  char                            Reserved[69];
};

///Manager用户登出请求
struct CXeleReqUserLogoutManagerField {
  ///资金账户
  TXeleUserIDType                 AccountID;
  ///交易所类型
  TXeleMarketType                 Market;
  ///预留
  char                            Reserved[63];
};

///Manager用户登出应答
struct CXeleRspUserLogoutManagerField {
  ///交易日
  TXeleDateType                   TradingDay;
  ///登出时间
  TXeleShortTimeType              LogoutTime;
  ///资金账户
  TXeleUserIDType                 AccountID;
  //会话代码
  TXeleSessionIDType              SessionId;
  ///预留
  TXeleReserved1Type              Reserved;
};

///Manager跨柜台资金调拨请求
struct CXeleReqBTCapTransferManagerField {
  ///机构代码,该字段无用,用户无需填写
  TXeleOrgIDType                  OrgID;
  ///资金账号
  TXeleUserIDType                 AccountID;
  ///币种,该字段无用，默认使用人民币
  TXeleCurrencyType               Currency;
  ///调拨金额
  TXeleMoneyType                  Fundamt;
  ///操作方向,1：上交柜台调往深交柜台；2：深交柜台调往上交柜台
  TXeleDirectionType              Direction;
  ///预留
  char                            Reserved[63];
};

///Manager跨柜台资金调拨应答
struct CXeleRspBTCapTransferManagerField {
  ///全局唯一消息编号 从1开始递增
  TXeleUniqueNumberType           UniqueNumber;
  ///机构代码
  TXeleOrgIDType                  OrgID;
  ///资金账号
  TXeleUserIDType                 AccountID;
  ///币种
  TXeleCurrencyType               Currency;
  ///调拨金额
  TXeleMoneyType                  Fundamt;
  ///操作方向,1：上交柜台调往深交柜台；2：深交柜台调往上交柜台
  TXeleDirectionType              Direction;
  ///预留
  char                            Reserved[63];
};

///Manager跨柜台资金调拨结果回报
struct CXeleRtnBtCapTransferManagerField {
  ///全局唯一消息编号
  TXeleUniqueNumberType           UniqueNumber;
  ///机构代码
  TXeleOrgIDType                  OrgID;
  ///资金账号
  TXeleUserIDType                 AccountID;
  ///币种
  TXeleCurrencyType               Currency;
  ///调拨金额
  TXeleMoneyType                  Fundamt;
  ///操作方向,1：上交柜台调往深交柜台；2：深交柜台调往上交柜台
  TXeleDirectionType              Direction;
  ///操作结果, '0':调拨失败,'1':调拨成功
  char                            Status;
  ///预留
  char                            Reserved[63];
};

///Manager跨柜台资金调拨记录查询请求
struct CXeleReqQryBTCapTransferManagerField {
  ///机构代码,该字段无用,用户无需填写
  TXeleOrgIDType                  OrgID;
  ///资金账号
  TXeleUserIDType                 AccountID;
  ///预留
  char                            Reserved[63];
};

///Manager跨柜台资金调拨记录查询应答
struct CXeleRspQryBTCapTransferManagerField {
  ///全局唯一消息编号
  TXeleUniqueNumberType           UniqueNumber;
  ///机构代码
  TXeleOrgIDType                  OrgID;
  ///资金账号
  TXeleUserIDType                 AccountID;
  ///币种
  TXeleCurrencyType               Currency;
  ///调拨金额
  TXeleMoneyType                  Fund;
  ///操作方向,1：上交柜台调往深交柜台；2：深交柜台调往上交柜台
  TXeleDirectionType              Direction;
  ///操作时间
  TXeleShortTimeType              Time;
  ///错误编号
  TXeleErrorIDType                ErrorID;
  ///操作结果
  TXeleOrderStatusType            Status;
  ///资金冲正操作标记 ‘0’：非资金冲正操作，‘1’：资金冲正操作
  TXeleIsFundReversalType         IsFundReversal;
  ///资金冲正柜台 SS表示上交柜台，SZ表示深交柜台 ‘--’表示无效
  TXeleReversalCounterType        ReversalCounter;
  ///资金冲正结果标记 ‘1’：正在处理中 ‘2’：处理成功 ‘3’：处理失败 ‘-’表示无效
  TXeleReversalResultType         ReversalResult;
  ///资金自动调拨 ‘0’:非自动调拨，‘1’:自动调拨
  TXeleIsAutoTransFlag            isAutoTrans;
  ///预留
  char                            Reserved[61];
};

///Manager沪深柜台资金账户查询请求
struct CXeleReqQryClientAccountFundManagerField{
  ///资金账号
  TXeleUserIDType                 AccountID;
  ///交易所类型
  TXeleMarketType                 Market;
  ///预留
  char                            Reserved[48];
};

///Manager沪深柜台资金账户查询应答
struct CXeleRspQryClientAccountFundManagerField{
  ///资金账号
  TXeleUserIDType                 AccountID;
  ///可用余额
  TXeleAvailableFundType          AvailableFund;
  ///可取余额  暂不使用
  TXeleAvailableCashType          AvailableCash;
  ///冻结资金
  TXeleFrozeMarginType            FrozeCapital;
  ///冻结手续费
  TXeleFrozenFeeType              FrozenFee;
  ///已付手续费
  TXeleUsedFeeType                UsedFee;
  ///初始上场资金（不变
  TXeleTotalFundType              InitTotalFund;
  ///总卖出
  TXeleSellFund                   SellFund;
  ///总买入
  TXeleBuyFund                    BuyFund;
  ///交易所类型
  TXeleMarketType                 Market;
  ///预留
  char                            Reserved[48];
};

struct ExternInfoField {
  TXeleApiVersion       api_version;
  TXeleApiMode          mode;
};

struct SysLocalInfoField {
  TXeleMacAddrType mac_addr;
  ///mac地址
  TXeleHardDiskSerialType hard_disk_serial;
  ///硬盘序列号
  TXeleIpAddrType ip_addr;
  ///ip地址
  TXeleCpuSerialType cpu_serial;
  ///cpu序列号
  TXeleHostNameType hostname;
  ///主机名称
}__attribute__((packed));

struct SuperviseExtraInfoField {
  char data[128];
};

struct AllSuperviseInfoField {
  char data[384];
};

///用户查询自己的股东账号等其他相关信息请求
struct CXeleReqQryInvestorInfoField {
  ///资金账户
  TXeleUserIDType                AccountID;
  ///预留
  TXeleReservedType              Reserved;
};

///用户查询自己的股东账号等其他相关信息应答
struct CXeleRspQryInvestorInfoField {
  ///资金账户
  TXeleUserIDType                AccountID;
  ///市场
  TXeleMarketType                Market;
  ///股东账户
  TXeleInvestorIDType            InvestorID;
  ///股东姓名
  TXeleInvestorNameType          InvestorName;
  ///股东权限，0.表示普通股票交易权限，1.SS:科创板权限  SZ:核准制创业板权限，2为可买入可转债券，3.SZ:为可买入注册制创业板，4.为可买入退市整理期股票'
  TXeleInvestorRightsType        InvestorRights;
  ///席位编号
  TXeleInvestorPbuType           InvestorPbu;
  ///客户代码
  TXeleUserIDType                CustID;
  ///预留
  char                           Reserved[113];
};

///持仓划拨请求(中信柜台专用，其他券商不支持)
struct CXeleReqPosTransferField {
  ///机构代码(可选)
  TXeleOrgIDType                  OrgID;
  ///资金账号
  TXeleUserIDType                 AccountID;
  ///股东账户(不使用)
  TXeleInvestorIDType             InvestorID;
  ///市场代码(可选)
  TXeleMarketIDType               MarketID;
  ///证券代码
  TXeleSecuritiesIDType           SecuritiesID;
  ///股份发生数
  TXeleSignedVolumeType           Volume;
  ///划拨方向 :1 冻结 , 2 解冻(可选)
  TXeleDirectionType              Direction;
  ///备注(不使用)
  TXeleRemarkType                 RemarkMsg;
  ///只执行一次集中柜台冻结(不使用)
  TXeleCallOneceFlag              CallOnce;
  ///分支机构(不使用)
  TXeleBranchNoType               BranchNo;
  ///客户代码(不使用)
  TXeleUserIDType                 CustID;
  ///预留
  char                            Reserved[44];
};
///持仓划拨应答(中信柜台专用，其他券商不支持)
struct CXeleRspPosTransferField {
  ///机构代码
  TXeleOrgIDType                  OrgID;
  ///资金账号
  TXeleUserIDType                 AccountID;
  ///股东账户
  TXeleInvestorIDType             InvestorID;
  ///证券代码
  TXeleSecuritiesIDType           SecuritiesID;
  ///fpga仓位是否更新
  TXeleIsUpdateFpgaFundType       IsUpdateFpgaStock;
  ///操作流水号
  TXeleSnoType                    Sno;
  ///股份发生数
  TXeleSignedVolumeType           stkeffect;
  ///错误信息
  TXeleCentralTradingErrorMsgType ptErrorMsg;
  ///预留
  char                            Reserved[44];
};

///集中交易持仓调拨艾科柜台明细查询请求(中信柜台专用，其他券商不支持)
struct CXeleReqQryPositionTransferRecordField {
  ///资金账户
  TXeleUserIDType                 AccountID;
  ///证券代码
  TXeleSecuritiesIDType           SecuritiesID;
  ///预留
  TXeleReservedType               Reserved;
};
///集中交易持仓调拨艾科柜台明细查询应答(中信柜台专用，其他券商不支持)
struct CXeleRspQryPositionTransferRecordField {
  ///机构代码
  TXeleOrgIDType                  OrgID;
  ///资金账户
  TXeleUserIDType                 AccountID;
  ///证券代码
  TXeleSecuritiesIDType           SecuritiesID;
  ///冻结/解冻仓位
  TXeleVolumeType                 Stkeffect;
  ///操作方向, '1'：冻结，'2':解冻
  TXeleDirectionType              Direction;
  ///fpga仓位是否更新
  TXeleIsUpdateFpgaFundType       IsUpdateFpgaFund;
  ///执行信息
  TXeleCentralTradingErrorMsgType posTransMsg;
  ///操作时间
  TXeleShortTimeType              Time;
  ///预留
  char                            Reserved[63];
};

///持仓划拨请求(部分券商使用)
struct CXeleReqInOutPositionField {
  ///资金账号(必填)
  TXeleUserIDType                 AccountID;
  ///证券代码(必填)
  TXeleSecuritiesIDType           SecuritiesID;
  ///划拨数量(必填)
  TXeleVolumeType                 Volume;
  ///划拨方向 :'1' 划入 , '2' 划出(必填)
  TXeleInOutDirectionType         InOutDirection;
  ///预留(客户不填)
  char                            Reserved[128];
};

///持仓划拨应答(部分券商使用)
struct CXeleRspInOutPositionField {
  ///机构代码
  TXeleOrgIDType                  OrgID;
  ///资金账号
  TXeleUserIDType                 AccountID;
  ///证券代码
  TXeleSecuritiesIDType           SecuritiesID;
  ///股东账户
  TXeleInvestorIDType             InvestorID;
  ///fpga仓位是否更新,暂不使用
  TXeleIsUpdateFpgaFundType       IsUpdateFpgaStock;
  ///操作流水号
  TXeleSnoType                    Sno;
  ///划拨数量
  TXeleVolumeType                 Volume;
  ///划拨方向 :'1' 划入 , '2' 划出
  TXeleInOutDirectionType         InOutDirection;
  ///集中交易柜台响应错误码,若没有错误码时，返回-1，代表无效值
  TXeleCentralTradingErrorIdType  ctErrorId;
  ///集中交易柜台响应错误信息
  TXeleCentralTradingErrorMsgLongType ctErrorMsg;
  ///预留
  char                            Reserved[128];
};

///集中交易持仓调拨艾科柜台明细查询请求(部分券商使用)
struct CXeleReqQryInOutPositionDetailsField {
  ///资金账户
  TXeleUserIDType                 AccountID;
  ///证券代码(选填)
  TXeleSecuritiesIDType           SecuritiesID;
  ///预留
  char Reserved[128];
};

///集中交易持仓调拨艾科柜台明细查询应答(部分券商使用)
struct CXeleRspQryInOutPositionDetailsField {
  ///机构代码
  TXeleOrgIDType                  OrgID;
  ///资金账户
  TXeleUserIDType                 AccountID;
  ///证券代码
  TXeleSecuritiesIDType           SecuritiesID;
  ///仓位变动数量
  TXeleVolumeType                 Stkeffect;
  ///划拨方向 :'1' 划入 , '2' 划出
  TXeleInOutDirectionType         InOutDirection;
  ///fpga仓位是否更新
  TXeleIsUpdateFpgaFundType       IsUpdateFpgaStock;
  ///艾科柜台响应错误码
  TXeleErrorIDType                ErrorId;
  ///艾科柜台响应错误信息
  TXeleErrorMsgType               ErrorMsg;
  ///集中交易柜台响应错误码,若没有错误码时，返回-1，代表无效值
  TXeleCentralTradingErrorIdType  ctErrorId;
  ///集中交易柜台响应错误信息,
  TXeleCentralTradingErrorMsgLongType ctErrorMsg;
  ///操作时间
  TXeleShortTimeType              Time;
  ///预留
  char                            Reserved[128];
};

///用户可用网关信息查询请求(部分券商使用)
struct CXeleReqQryAvailableGateWayRecordField {
  ///资金账户
  TXeleUserIDType                 AccountID;
  ///预留
  char                            Reserved0;
  ///预留
  TXeleReservedType               Reserved;
};

struct CXeleRspQryAvailableGateWayRecordField {
  ///资金账户
  TXeleUserIDType                 AccountID;
  ///交易所类型
  TXeleMarketType                 Market;
  ///竞价网关,报单时需要转换成整型，如'1'转换成1后填充指定网关字段
  TXeleGateWay                    BidGateWays;
  ///债券网关,报单时需要转换成整型，如'1'转换成1后填充指定网关字段
  TXeleGateWay                    BondGateWays;
  ///预留
  TXeleReserved3Type               Reserved;
};

//////////////////////////////
/////// API通用接口系列      //
//////////////////////////////

///上场前写入用户密码请求
struct CXeleReqAccountInitField{
  ///资金账户
  TXeleUserIDType                 AccountID;
  ///交易用户密码
  TXeleUserPasswordType           Password;
  ///交易所类型
  TXeleMarketType                 Market;
  ///命令号
  TXeleCommandNum                 CommandNum;
  ///委托方式 内部使用
  TXeleOperwayType                Operway;
  ///柜台ip 内部使用
  TXeleTradeIPType                QueryURL;
  ///预留
  char                            Reserved[66];
};

struct CXeleRspCommonFuncField{
  ///命令号
  TXeleCommandNum                 CommandNum;
  ///预留
  char                            Reserved[108];
};

///查询程序化风控信息请求(部分券商使用)
struct CXeleReqQryPrcProgramInfoField {
  ///资金账户
  TXeleUserIDType                AccountID;
  ///程序化风控类别
  ///0 返回全部, 1 全天报撤单笔数限制风控, 2 流速风控
  TXelePrcProgramType            PrcProgramType;
  ///预留
  char                           Reserved[20];
};

struct CXeleRspQryRateLimitInfoField {
  ///单位时间(毫秒)
  TXeleTimeUnitType           timeUnit;
  ///单位时间流速阈值
  TXelePrcProgramTresholdType RateLimit;
  ///证券类别
  ///1 股票, 2 基金, 3 债券(若柜台多种类型合并配置，则以逗号分隔返回，如2,3)
  TXelePrcSecuritiesType      PrcSecuritiesType;
  ///预留
  char                        Reserved[30];
};

struct CXeleRspQryDailyLimitInfoField {
  ///全天委托笔数阈值
  TXelePrcProgramTresholdType  OrderCountLimit;
  ///当前委撤笔数 (程序化风控类别为全天报撤笔数限制风控时有效)
  TXeleCurrentOrderCountType   CurrentOrderCount;
  ///证券类别
  ///1 股票, 2 基金, 3 债券 (若柜台多种类型合并配置，则以逗号分隔返回，如2,3)
  TXelePrcSecuritiesType       PrcSecuritiesType;
  ///预留
  char                         Reserved[30];
};

///查询程序化风控信息应答(部分券商使用)
struct CXeleRspQryPrcProgramInfoField {
  ///资金账户
  TXeleUserIDType                 AccountID;
  ///市场
  TXeleMarketType                 Market;
  ///程序化风控类别
  ///1 全天报撤笔数限制风控，2 流速风控
  TXelePrcProgramType             PrcProgramType;
  ///风控开关
  ///0 关，1 开
  TXelePrcProgramEnableType       Enable;
  union {
    CXeleRspQryRateLimitInfoField  orderRateLimit;
    CXeleRspQryDailyLimitInfoField dailyLimit;
  } prcProgrmData;
  ///预留
  char                            Reserved[60];
};

#pragma pack(pop)

#endif
