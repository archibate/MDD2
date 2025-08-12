#include "config.h"
#if XC || NE
#include "OES.h"
#include "MDD.h"
#include "heatZone.h"
#include "strXele.h"
#include "OrderRefLut.h"
#include <string9811.h>
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#include <xele/XeleSecuritiesTraderApi.h>
#include <fstream>
#include <cstdio>
#include "LOG.h"

namespace
{

// void fmtSecurityId(TXeleSecuritiesIDType securityID, int32_t stock)
// {
//     securityID[5] = '0' + stock % 10;
//     stock /= 10;
//     securityID[4] = '0' + stock % 10;
//     stock /= 10;
//     securityID[3] = '0' + stock % 10;
//     stock /= 10;
//     securityID[2] = '0' + stock % 10;
//     stock /= 10;
//     securityID[1] = '0' + stock % 10;
//     stock /= 10;
//     securityID[0] = '0' + stock % 10;
// }

class XeleTdSpi final : public XeleSecuritiesTraderSpi
{
public:
    XeleTdSpi() = default;
    ~XeleTdSpi() override = default;

    //表明具有资金调拨的相关权限
    bool canFundTransfer{false};
    //表明具有柜台查询权限
    bool canQuery{false};
    //表明具有柜台报、撤单权限
    bool canOrder{false};

private:
    ///艾科管理中心登录应答,当只有登录管理中心的需求时，收到该回报即可进行管理中心相关接口操作
    void onRspLoginManager(CXeleRspUserLoginManagerField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) override;

    ///艾科柜台登录应答,当需要管理中心接口可用，但是只需求艾科柜台查询接口可用时，收到该回报即可进行操作
    void onRspLogin(CXeleRspUserLoginField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) override;

    ///添加交易链路应答,当收到该回报时，标记艾科柜台报、撤单接口可用
    void onRspInitTrader(CXeleRspInitTraderField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) override;

    ///艾科管理中心登出应答
    void onRspLogoutManager(CXeleRspUserLogoutManagerField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) override;

    ///艾科柜台登出应答
    void onRspLogout(CXeleRspUserLogoutField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) override;

    ///当客户端与管理中心服务端查询通信连接断开时，该方法被调用。
    ///该回报触发条件为客户主动发起登出请求，或者链路发生了断连。
    ///出现以上情况时，若不是主动发起登出请求，建议先发起登出请求，再重新登录，即可再次使用相关接口
    void onFrontManagerQueryDisconnected(int nReason) override;

    ///当客户端与服务端查询通信连接断开时，该方法被调用。
    ///该回报触发条件为客户主动发起登出请求，或者链路发生了断连。
    ///出现以上情况时，若不是主动发起登出请求，建议先发起登出请求，再重新登录，即可再次使用相关接口
    void onFrontQueryDisconnected(int nReason) override;

    ///当客户端与服务端交易通信连接断开时，该方法被调用。
    ///当收到该回调，表示当前连接失去了报、撤单功能.
    ///如需恢复，参考OnFrontQueryDisconnected处理方法
    void onFrontTradeDisconnected(int nReason) override;

    ///api内部消息打印回调
    void onApiMsg(int ret, const char *strFormat, ...) override;

    ///////////////////////////////
    /////// 证券、期权公用 ////////
    //////////////////////////////

    ///报单应答
    void onRspInsertOrder(CXeleRspOrderInsertField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) override;

    ///报单错误回报
    void onErrRtnInsertOrder(CXeleRspOrderInsertField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) override;

    ///撤单应答
    void onRspCancelOrder(CXeleRspOrderActionField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) override;

    ///撤单错误回报
    void onErrRtnCancelOrder(CXeleRspOrderActionField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) override;

    ///报单回报
    void onRtnOrder(CXeleRtnOrderField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) override;

    ///成交回报
    void onRtnTrade(CXeleRtnTradeField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) override;

    ///报单查询应答
    void onRspQryOrder(CXeleRspQryOrderField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) override;

    ///成交查询应答
    void onRspQryTrade(CXeleRspQryTradeField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) override;

    ///////////////////////////////
    /////// 证券相关 //////////////
    //////////////////////////////

    ///证券资金查询应答
    void onRspQryFund(CXeleRspQryStockClientAccountField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) override;

    ///证券持仓查询应答
    void onRspQryPosition(CXeleRspQryStockPositionField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) override;
};

XeleTdSpi *g_userSpi;
XeleSecuritiesTraderApi *g_tradeApi;
std::atomic<uint32_t> g_requestID{1};
TXeleOrderIDType g_maxUserLocalID;
OrderRefLut g_orderRefLut;

std::string g_username;
std::string g_password;

struct ConfigParam98 {
  ///当前Api是否只连接管理中心
  bool IsJustConnectManager = false;
  ///当前Api支持柜台版本，默认支持一代柜台（2.5及以下版本），后续根据版本部署情况修改默认支持版本
  ///当需要修改Api支持版本时，请确认当前艾科柜台版本
  ///随意修改版本，或者格式异常时，可能会导致无法连接至艾科柜台或无法向艾科柜台报单
  ///当柜台升级到二代柜台（3.0及以上版本）后，此参数需要修改为2（DependentCounterVersion=2）
  int DependentCounterVersion = 2;
  ///艾科管理中心地址，使用统一的格式tcp://地址:端口号
  ///示例1：tcp://192.168.4.216:50002;tcp://192.168.4.216:50003;tcp://192.168.4.216:50004
  ///示例2：tcp://192.168.4.216:50000
  ///Api用来连接管理中心的参数，可以配置多个manager参数,使用;来分隔,多个地址表示逐个轮询连接
  std98string ManagerURL;
  ///艾科柜台查询链路连接地址，使用统一的格式tcp://地址:端口号
  ///此参数为空，使用管理中心默认分配的地址，此参数为合法地址时，使用配置的值。地址、端口映射同理。配置格式如下示例：
  ///示例1：tcp://192.168.4.67:30000
  ///示例2：tcp://192.168.4.67:30000;tcp://192.168.4.68:30000
  ///支持配置主备柜台地址，使用;来分隔，当主柜台连接失败会轮询连接备柜台
  ///Api自动重连也会轮询连接主备柜台
  ///当同时配置了ManagerURL和QueryURL的情况下，Api连接manager成功之后会以配置项为准来连接柜台查询链路
  std98string QueryURL;
  ///艾科报单链路连接地址，使用统一的格式tcp://地址:端口号
  ///此参数为空，使用管理中心默认分配的地址，此参数为合法地址时，使用配置的值。地址、端口映射同理。配置格式如下示例：
  ///示例：tcp://10.128.123.209:30005
  ///场景：例如原报单链路连接地址为tcp://10.128.123.209:30005，由于组网限制，需要将该地址映射为tcp://192.168.0.100:33333
  ///      此时，将TradeURL参数改为：TradeURL=tcp://192.168.0.100:33333；如果没有此种情况只需要如下不填即可。
  ///场景：例如从管理中心查询到的地址为tcp://192.168.4.67:29999，现需要修改改默认地址，
  ///      此时，将TradeURL参数改为：TradeURL=tcp://192.168.4.216:29999，如果没有这种需求，不填即可。
  std98string TradeURL;
  ///用户账号
  std98string AccountId;
  ///用户密码
  std98string Password;
  ///Api与艾科柜台心跳间隔，此参数无特殊需求，不要进行修改
  ///示例：=6表示Api与柜台间每6s发送进行一次心跳交互
  uint64_t HeartBeatInterval = 6;
  ///Api与艾科柜台心跳超时判定次数，此参数无特殊需求，不要进行修改
  ///示例：=3表示Api与柜台间超过三次心跳未收到，就判定tcp断连，需要重新登录来获取权限
  uint64_t HeartBeatTimeOutCnt = 3;
  ///Api报单链路warm开关，默认打开，关闭此参数时，冷态报单时延可能会产生抖动
  ///此参数在sfc网卡上使用会减轻链路数据处理压力，在其他网卡上使用时，会增加网卡数据处理压力，建议使用sfc网卡
  ///示例 :=1表示默认打开，=0表示关闭
  int OrderWarm = 0;
  ///Api登录软件节点后是否接收其他节点的回报信息，默认不接收
  ///此参数对登录硬件节点的Api不生效
  ///示例 :=0表示默认关闭，=1表示打开
  int SoftRspRcv = 0;
  //Api登录硬件节点后是否接收其他节点的回报信息，默认接收
  //此参数对登录软件节点的Api不生效
  //此字段为字符类型，枚举值'0','1'
  //'0'代表不抄送，'1'代表抄送
  //默认值为'1'
  //仅部分券商支持配置为不抄送
  char FpgaRspRcv = '1';
  ///Api报单发送、报单回报线程绑定核心，在配置此参数前，需要先进行隔核操作
  ///绑核只能在Api初始化的过程中进行修改，在单次登录、登出操作中无法修改绑定核心
  ///示例：
  ///=-1,-1表示不进行绑核操作
  ///=3,4表示Api报单发送线程绑定核心3，报单回报线程绑定核心4
  ///以上参数可以选择其中一个进行绑定(例如：=3,-1)，也可以选择都不绑定(例如：=-1,-1)，都不绑定会影响Api性能.
  ///如果可用核心数不够，请尽量选择Api的报单发送线程进行绑定，这样可以尽可能保证报单速率
  int CpuCore[2]{-1,-1};
  ///Api回报接收socket是否阻塞标记(无用，改用ApiMode参数)
  ///=0表示非阻塞socket，该模式下，cpu占用率高，时延相对较低
  ///=1表示阻塞socket，该模式下，cpu占用率低，时延相对较高
  int SocketBlockFlag = 0;
  ///Api运行模式（高性能模式、普通模式）
  ///=0表示普通模式，在该模式下，api内存的消耗会有明显降低，同时报单的速率会受到影响
  ///=1表示高性能模式，在该模式下，api会占用较多内存，同时报单的速率会有提升
  ///默认Api运行在高性能模式下，如果没有特殊需求，不需要进行修改
  ///修改了此参数后，需要重启Api
  int ApiMode = 1;
  ///是否详细打印Api日志（详细日志包括请求的输入值、响应接口的输出值以及必要的Api日志，非详细的日志只有必要的Api日志），其中必要的Api日志不会影响Api性能
  ///=0表示非详细打印
  ///=1表示详细打印
  ///此参数默认关闭
  ///注意：详细的Api日志会影响Api性能，建议在调试或者定位时打开此参数
  int SuperLog=0;
  ///Api是否使用rtnTrade报文构造rtnOrder报文for_ci
  ///该参数开启时，首先要保证柜台系统参数表第一百零四个参数处于关闭状态，这时柜台不会在回rtnTrade报文时先推送rtnOrder报文，Api端在收到rtnTrade报文时会在Api端构造rtnOrder报文进行推送for_ci
  ///=0表示关闭for_ci
  ///=1表示开启for_ci
  ///此参数默认关闭for_ci
  ///注意：当开启该参数时，会对Api的性能产生影响；开启参数时如果柜台的参数没有关闭，会导致Api端收到两条同样的rtnOrder报文，可能会对客户程序产生影响for_ci
  int CreateRtnOrderByRtnTrade = 0;
  ///艾科柜台类型,用该字段标记api需要从管理中心查询哪种柜台地址，该字段只有连接管理中心时有效
  /// =1表示艾科上交股票柜台;
  /// =2表示艾科深交股票柜台;
  /// =3表示艾科上交期权柜台;
  /// =4表示艾科深交期权柜台,暂不支持;
  char Market = '1';
  ///委托方式校验字段
  ///当有报单委托方式校验需求时填写，没有时无需修改(填写时具体取值需要和券商沟通)
  char OperWay = '1';
  ///正式站点前缀，目前中信、海通用(填写时具体取值需要和券商沟通)
  std98string PcPrefix;
  ///交易终端软件名称及版本
  std98string AppID;
  ///用户登录节点号，0~7共八个节点可供选择，其中7号节点用作web报单
  int SubClientIndex = 0;
  ///流水重构标志，期权使用，上交和深交柜台不使用该字段，无需做修改
  ///=0表示不进行流水重构
  ///=1表示只进行资金流水重构;
  ///=2表示只进行报文流水重构;
  ///=3表示资金和报文流水重构;
  unsigned char FlowRebuildFlag = 0;
  ///是否xbot登录，普通客户填写false
  ///如果没有Xbot的登录需求，填false
  bool isXbot = false;
  ///是否自动获取穿透式监管信息，普通客户填true
  ///如果不需要使用传入的穿透式监管信息，该字段填写true;
  ///当该字段为false时，表示使用sysInfo中传入的穿透式监管信息
  bool isAutoGetSysInfo = true;
  ///穿透式监管信息
  ///配合isAutoGetSysInfo使用
  ///当isAutoGetSysInfo字段为false时，该字段有效,使用该字段的监管信息
  SysLocalInfoField sysInfo;
  ///营业部代码
  std98string businessCode;
  ///当链接超时之后是否需要重连 0=不自动重连 1=自动重连
  int AutoReLogin = 0;
  ///第一次登录连接失败是否需要一直循环连接manager
  ///=0不循环连接3秒后断开
  ///=1循环连接直到手动结束
  int LoopRepeatConnect = 0;
  ///Api收发线程是否分开
  ///参数开启时，api的收发线程分离，用户调用报撤单接口所发送的消息会直接通过网卡发往柜台；
  ///在参数开启时，将调用reqInsertOrder、reqCancelOrder接口的线程绑核，另外使用CpuCore参数的第二个变量将回报线程绑核，这样能设置api运行在高性能模式下。
  ///参数开启时，OrderWarm、ApiMode以及CpuCore的第一个参数无效
  ///此参数重启生效
  ///示例：
  ///=0表示关闭，=1表示开启；默认关闭
  int RecvSendDetach = 0;
  ///sfc网卡名,用于交易链路通讯加速
  ///若不配置或环境异常采用原生socket通讯
  std98string solarfareTradeEthName;
  //是否开启登录的时候进行用户名和密码加密
  //api登录manger和柜台的时候是否发送的账户和密码是密文
  // =0 表示关闭
  // =1 表示打开
  // 默认关闭
  int openEncryption = 0;
  //公网信息
  std98string SuperviseExtraInfo;
  //版本校验, XeleSecuritiesTraderApi.h头文件位置
  //校验当前.so内的版本是否和头文件中的版本号匹配
  //填写值了会去获取指定目录下的相关文件,为空不校验
  //填写绝对路径, 默认值为空
  //例如 ApiIncludePath=/home/xele/api/
  std98string ApiIncludePath;
  //此字段填写后,PcPrefix, APPID, SuperviseExtraInfo配置项均不生效
  //客户登录时填写的完整监管信息,此信息不做校验,直接透传至柜台落库
  //最大支持到384字节,不支持中文格式字符,默认为空
  //柜台默认支持长度255，若超过则需要跟券商确认柜台是否支持
  //仅部分券商支持，使用前请确认
  std98string AllSuperviseInfo;
  ///是否捕获异常，默认捕获
  ///=0表示不捕获异常
  ///=1表示捕获异常
  ///说明，此处的异常是指程序中抛出的信号，Api中捕获SIGSEGV和SIGABRT两个信号，当关闭时，Api不处理任何信号
  int CaptureSignal = 1;
};

int callReqLogin()
{
    ConfigParam98 param{};
    param.IsJustConnectManager = false;
    param.DependentCounterVersion = 2;
#if NE
    param.ManagerURL = "tcp://10.101.58.32:50000;tcp://10.101.58.33:50000";
#endif
#if XC
    param.ManagerURL = "tcp://10.208.48.27:50000;tcp://10.208.48.28:50000";
#endif
    param.QueryURL = "";
    param.TradeURL = "";
    param.AccountId = g_username;
    param.Password = g_password;
    param.HeartBeatInterval = 6;
    param.HeartBeatTimeOutCnt = 3;
    param.OrderWarm = 1;
    param.SoftRspRcv = 0;
    param.FpgaRspRcv = '1';
    param.CpuCore[0] = kOESSendCpu;
    param.CpuCore[1] = kOESRecvCpu;
    param.SocketBlockFlag = 0;
    param.ApiMode = 1;
    param.SuperLog = 0;
    param.CreateRtnOrderByRtnTrade = 0;
    param.Market = '0' + MARKET_ID;
#if NE
    param.OperWay = '7';
#endif
#if XC
    param.OperWay = 'E';
#endif
    param.PcPrefix = "";
#if NE
    param.AppID = "DQsig:V1.0";
#else
    param.AppID = "xele_dev_1.0";
#endif
    param.FlowRebuildFlag = 0;
    param.AutoReLogin = 0;
    param.LoopRepeatConnect = 0;
    param.RecvSendDetach = 1;
#if NE && SH
    param.solarfareTradeEthName = "enp1s0f1";
#endif
#if NE && SZ
#if SZ2
    param.solarfareTradeEthName = "enp1s0f0";
#else
    param.solarfareTradeEthName = "enp1s0f1";
#endif
#endif
#if XC && SH
    param.solarfareTradeEthName = "enp1s0f0";
#endif
#if XC && SZ
    param.solarfareTradeEthName = "enp1s0f1";
#endif
    param.openEncryption = 0;
    param.CaptureSignal = 0;
    return g_tradeApi->reqLoginEx((const ConfigParam *)&param, g_requestID.fetch_add(1, std::memory_order_relaxed));
    // return g_tradeApi->reqLogin(g_xeleConfigFile.c_str(), g_username.c_str(), g_password.c_str(), g_xeleTradeNode, '0' + MARKET_ID, g_requestID.fetch_add(1, std::memory_order_relaxed));
}


/// 艾科管理中心登录应答,当只有登录管理中心的需求时，收到该回报即可进行管理中心相关接口操作
void XeleTdSpi::onRspLoginManager(CXeleRspUserLoginManagerField* pRspField, CXeleRspInfo* pRspInfo, int nRequestID, bool bIsLast)
{
    if (pRspInfo->ErrorID == 0)
    {
        canFundTransfer = true;
        LOGf(DEBUG, "now can use manager interface\n");
    }
    else
    {
        LOGf(ERROR, "login manager error,ErrID[%d],ErrMsg[%s]\n", pRspInfo->ErrorID, pRspInfo->ErrorMsg);
    }
};

/// 艾科柜台登录应答,当需要管理中心接口可用，但是只需求艾科柜台查询接口可用时，收到该回报即可进行操作
void XeleTdSpi::onRspLogin(CXeleRspUserLoginField* pRspField, CXeleRspInfo* pRspInfo, int nRequestID, bool bIsLast)
{
    if (pRspInfo->ErrorID == 0)
    {
        canQuery = true;
        g_maxUserLocalID = pRspField->MaxUserLocalID;
        LOGf(DEBUG, "now can use query interface, maxUserLocalID:%d\n", pRspField->MaxUserLocalID);
    }
    else
    {
        LOGf(ERROR, "login counter error,ErrID[%d],ErrMsg[%s]\n", pRspInfo->ErrorID, pRspInfo->ErrorMsg);
    }
};

/// 添加交易链路应答,当收到该回报时，标记艾科柜台报、撤单接口可用
void XeleTdSpi::onRspInitTrader(CXeleRspInitTraderField* pRspField, CXeleRspInfo* pRspInfo, int nRequestID, bool bIsLast)
{
    if (pRspInfo->ErrorID == 0)
    {
        canOrder = true;
        LOGf(DEBUG, "now can use order interface\n");
    }
    else
    {
        LOGf(ERROR, "create order link error,ErrID[%d],ErrMsg[%s]\n", pRspInfo->ErrorID, pRspInfo->ErrorMsg);
    }
};

/// 艾科管理中心登出应答
void XeleTdSpi::onRspLogoutManager(CXeleRspUserLogoutManagerField* pRspField, CXeleRspInfo* pRspInfo, int nRequestID, bool bIsLast)
{
    LOGf(DEBUG, "  Test LogoutManager pass\n");
    LOGf(DEBUG, "=======================");
    if (pRspInfo->ErrorID == 0)
    {
        canFundTransfer = false;
        LOGf(DEBUG, "now can't use manager interface\n");
    }
    else
    {
        LOGf(ERROR, "logout manager error,ErrID[%d],ErrMsg[%s]\n", pRspInfo->ErrorID, pRspInfo->ErrorMsg);
    }
};

/// 艾科柜台登出应答
void XeleTdSpi::onRspLogout(CXeleRspUserLogoutField* pRspField, CXeleRspInfo* pRspInfo, int nRequestID, bool bIsLast)
{
    LOGf(DEBUG, "  Test Logout pass\n");
    LOGf(DEBUG, "=======================\n");
    if (pRspInfo->ErrorID == 0)
    {
        canQuery = false;
        LOGf(DEBUG, "now can't use query interface\n");
    }
    else
    {
        LOGf(ERROR, "logout counter error,ErrID[%d],ErrMsg[%s]\n", pRspInfo->ErrorID, pRspInfo->ErrorMsg);
    }
};

/// 当客户端与管理中心服务端查询通信连接断开时，该方法被调用。
/// 该回报触发条件为客户主动发起登出请求，或者链路发生了断连。
/// 出现以上情况时，若不是主动发起登出请求，建议先发起登出请求，再重新登录，即可再次使用相关接口
void XeleTdSpi::onFrontManagerQueryDisconnected(int nReason)
{
    canFundTransfer = false;
    LOGf(DEBUG, "need relogin\n");
};

/// 当客户端与服务端查询通信连接断开时，该方法被调用。
/// 该回报触发条件为客户主动发起登出请求，或者链路发生了断连。
/// 出现以上情况时，若不是主动发起登出请求，建议先发起登出请求，再重新登录，即可再次使用相关接口
void XeleTdSpi::onFrontQueryDisconnected(int nReason)
{
    canQuery = false;
    LOGf(DEBUG, "query disconnect, need re-login\n");
};

/// 当客户端与服务端交易通信连接断开时，该方法被调用。
/// 当收到该回调，表示当前连接失去了报、撤单功能.
/// 如需恢复，参考OnFrontQueryDisconnected处理方法
void XeleTdSpi::onFrontTradeDisconnected(int nReason)
{
    canOrder = false;
    LOGf(DEBUG, "trade disconnect, need re-login\n");

#if 0
    ///***当盘中发生了断连，可以采取如下的处理方式，来重新获取操作权限***///
    /// 当这三种回调被调用时onFrontManagerQueryDisconnected、onFrontQueryDisconnected、onFrontTradeDisconnected可以进行如下异常处理
    /// 可以选择三种回调中某一种进行处理
    /// 可以在回调函数中处理，也可以进行实时的异常检测另起线程处理
    ///*********************************///
    /// 先发送登出请求，取消用户注册权限
    g_tradeApi->reqLogout(g_username.c_str(), g_requestID.fetch_add(1, std::memory_order_relaxed));
    LOGf(DEBUG, "reqLogout\n");
    sleep(2);
    /// 此处等待所有连接都断开后，再进行下一步处理
    while (true)
    {
        LOGf(DEBUG, "wait !canOrder && !canQuery && !canFundTransfer\n");
        sleep(1);
        if (!canOrder && !canQuery && !canFundTransfer)
            break;
    }
    LOGf(DEBUG, "now api disconnect,start re-login\n");

    /// 发送登录请求，重新获取账户权限
    while (callReqLogin() != 0)
    {
        LOGf(ERROR, "call reqLogin fail,try again\n");
        sleep(3);
    }
    LOGf(DEBUG, "relogin send success,now wait a moment,then you can reuse other Api interface\n");
    ///***登录请求发送完成后，就可以等待onRspLoginManager、onRspLogin、onRspInitTrader回调响应，再调用其他接口***///
#endif
};

/// api内部消息打印回调 1:error, 0:normal
void XeleTdSpi::onApiMsg(int ret, const char* strFormat, ...)
{
    char strLog[2048];
    va_list argList;
    va_start(argList, strFormat);
    strLog[std::vsnprintf(strLog, sizeof strLog, strFormat, argList)] = 0;
    va_end(argList);
    if (ret) {
        SPDLOG_ERROR("(API) {}", strLog);
    } else {
        SPDLOG_DEBUG("(API) {}", strLog);
    }
};

///////////////////////////////
/////// 证券、期权公用 /////////
//////////////////////////////

/// 报单应答
HEAT_ZONE_RSPORDER void XeleTdSpi::onRspInsertOrder(CXeleRspOrderInsertField* pRspField, CXeleRspInfo* pRspInfo, int nRequestID, bool bIsLast)
{
    OES::RspOrder rsp;
    rsp.rspType = OES::RspOrder::XeleRspOrderInsert;
    rsp.userLocalID = g_orderRefLut.orderRefLookup(nRequestID);
    rsp.requestID = nRequestID;
    rsp.errorID = pRspField->ErrorId;
    rsp.xeleRspOrderInsert = pRspField;
    MDD::handleRspOrder(rsp);

    // extern CStrategyTrade* g_pStrategy; g_pStrategy->perfTick(PerfTickRspInsertOrder);
    //
    // // SPDLOG_TRACE("onRspInsertOrder {}", nRequestID);
    // Td_RtnOrder rtnOrder;
    // std::memset(&rtnOrder, 0, sizeof(Td_RtnOrder));
    // std::memcpy(rtnOrder.accountId, pRspField->AccountID, sizeof(TXeleUserIDType));
    // rtnOrder.msgType = MsgType_RspOrder;
    // rtnOrder.bsType = pRspField->Direction;
    // rtnOrder.symbolId = atoi(pRspField->SecuritiesID);
    // rtnOrder.orderSysId = pRspField->OrderSysID;
    // rtnOrder.userLocalId = pRspField->UserLocalID;
    // rtnOrder.origOrdPrice = pRspField->LimitPrice * 10000;
    // rtnOrder.origOrdQty = pRspField->Volume;
    // if (pRspInfo->ErrorID == 0)
    //     rtnOrder.ordStatus = ODRSTAT_REPORTED;
    // else
    //     rtnOrder.ordStatus = ODRSTAT_ERROR;
    // rtnOrder.errorId = pRspInfo->ErrorID;
    // rtnOrder.requestId = nRequestID;
    //
    // uint32_t index = stoi(pRspField->SecuritiesID) % 1000000;
    // uint32_t channelIndex = CContext::getInstance()->m_SymobolList[index].channelNo % MyConfigure::getInstance()->ChannelNum;
    // sCacheDataType sCachData;
    // sCachData.FnType = MsgType_RspOrder;
    // std::memcpy(sCachData.FnParam1, &rtnOrder, sizeof(Td_RtnOrder));
    // MarketMaker::getInstance()->m_ChannelPool[channelIndex]->tryPush(sCachData);
};

/// 报单错误回报
void XeleTdSpi::onErrRtnInsertOrder(CXeleRspOrderInsertField* pRspField, CXeleRspInfo* pRspInfo, int nRequestID, bool bIsLast)
{
    OES::RspOrder rsp;
    rsp.rspType = OES::RspOrder::XeleRspOrderInsert;
    rsp.userLocalID = g_orderRefLut.orderRefLookup(nRequestID);
    rsp.requestID = nRequestID;
    rsp.errorID = pRspField->ErrorId;
    rsp.xeleRspOrderInsert = pRspField;
    MDD::handleRspOrder(rsp);

    // Td_RtnOrder rtnOrder;
    // std::memset(&rtnOrder, 0, sizeof(Td_RtnOrder));
    // std::memcpy(rtnOrder.accountId, pRspField->AccountID, sizeof(TXeleUserIDType));
    // rtnOrder.msgType = MsgType_RspOrder;
    // rtnOrder.bsType = pRspField->Direction;
    // rtnOrder.symbolId = atoi(pRspField->SecuritiesID);
    // rtnOrder.orderSysId = pRspField->OrderSysID;
    // rtnOrder.userLocalId = pRspField->UserLocalID;
    // rtnOrder.origOrdPrice = pRspField->LimitPrice * 10000;
    // rtnOrder.origOrdQty = pRspField->Volume;
    // rtnOrder.ordStatus = ODRSTAT_ERROR;
    // rtnOrder.errorId = pRspInfo->ErrorID;
    // rtnOrder.requestId = nRequestID;
    //
    // uint32_t index = stoi(pRspField->SecuritiesID) % 1000000;
    // uint32_t channelIndex = CContext::getInstance()->m_SymobolList[index].channelNo % MyConfigure::getInstance()->ChannelNum;
    // sCacheDataType sCachData;
    // sCachData.FnType = MsgType_RspOrder;
    // std::memcpy(sCachData.FnParam1, &rtnOrder, sizeof(Td_RtnOrder));
    // MarketMaker::getInstance()->m_ChannelPool[channelIndex]->tryPush(sCachData);
};

/// 撤单应答
HEAT_ZONE_RSPORDER void XeleTdSpi::onRspCancelOrder(CXeleRspOrderActionField* pRspField, CXeleRspInfo* pRspInfo, int nRequestID, bool bIsLast)
{
    OES::RspOrder rsp;
    rsp.rspType = OES::RspOrder::XeleRspOrderAction;
    rsp.userLocalID = g_orderRefLut.orderRefLookup(nRequestID);
    rsp.requestID = nRequestID;
    rsp.errorID = pRspField->ErrorId;
    rsp.xeleRspOrderAction = pRspField;
    MDD::handleRspOrder(rsp);

    // Td_RtnOrder rtnOrder;
    // std::memset(&rtnOrder, 0, sizeof(Td_RtnOrder));
    // std::memcpy(rtnOrder.accountId, pRspField->AccountID, sizeof(TXeleUserIDType));
    // rtnOrder.msgType = MsgType_RspCancel;
    // rtnOrder.orderSysId = pRspField->OrderSysID;
    // rtnOrder.userLocalId = pRspField->UserLocalID;
    // if (pRspInfo->ErrorID == 0)
    //     rtnOrder.ordStatus = ODRSTAT_REPORTED;
    // else
    //     rtnOrder.ordStatus = ODRSTAT_ERROR;
    // rtnOrder.errorId = pRspInfo->ErrorID;
    // rtnOrder.requestId = nRequestID;
    //
    // uint32_t channelNo = CContext::getInstance()->RegisterLoaclID(pRspField->OrigUserLocalID, 0);
    // uint32_t channelIndex = channelNo % MyConfigure::getInstance()->ChannelNum;
    // sCacheDataType sCachData;
    // sCachData.FnType = MsgType_RspCancel;
    // std::memcpy(sCachData.FnParam1, &rtnOrder, sizeof(Td_RtnOrder));
    // MarketMaker::getInstance()->m_ChannelPool[channelIndex]->tryPush(sCachData);
};

/// 撤单错误回报
void XeleTdSpi::onErrRtnCancelOrder(CXeleRspOrderActionField* pRspField, CXeleRspInfo* pRspInfo, int nRequestID, bool bIsLast)
{
    OES::RspOrder rsp;
    rsp.rspType = OES::RspOrder::XeleRspOrderAction;
    rsp.userLocalID = g_orderRefLut.orderRefLookup(nRequestID);
    rsp.requestID = nRequestID;
    rsp.errorID = pRspField->ErrorId;
    rsp.xeleRspOrderAction = pRspField;
    MDD::handleRspOrder(rsp);

    // Td_RtnOrder rtnOrder;
    // std::memset(&rtnOrder, 0, sizeof(Td_RtnOrder));
    // std::memcpy(rtnOrder.accountId, pRspField->AccountID, sizeof(TXeleUserIDType));
    // rtnOrder.msgType = MsgType_RspCancel;
    // rtnOrder.orderSysId = pRspField->OrderSysID;
    // rtnOrder.userLocalId = pRspField->UserLocalID;
    // rtnOrder.ordStatus = ODRSTAT_ERROR;
    // rtnOrder.errorId = pRspInfo->ErrorID;
    // rtnOrder.requestId = nRequestID;
    //
    // uint32_t channelNo = CContext::getInstance()->RegisterLoaclID(pRspField->OrigUserLocalID, 0);
    // uint32_t channelIndex = channelNo % MyConfigure::getInstance()->ChannelNum;
    // sCacheDataType sCachData;
    // sCachData.FnType = MsgType_RspCancel;
    // std::memcpy(sCachData.FnParam1, &rtnOrder, sizeof(Td_RtnOrder));
    // MarketMaker::getInstance()->m_ChannelPool[channelIndex]->tryPush(sCachData);
};

/// 报单回报
HEAT_ZONE_RSPORDER void XeleTdSpi::onRtnOrder(CXeleRtnOrderField* pRspField, CXeleRspInfo* pRspInfo, int nRequestID, bool bIsLast)
{
    OES::RspOrder rsp;
    rsp.rspType = OES::RspOrder::XeleRtnOrder;
    rsp.userLocalID = g_orderRefLut.orderRefLookup(nRequestID);
    rsp.requestID = nRequestID;
    rsp.errorID = 0;
    rsp.xeleRtnOrder = pRspField;
    MDD::handleRspOrder(rsp);

    // if (pRspField->OrderStatus == ODRSTAT_REPORTED) {
    //     extern CStrategyTrade* g_pStrategy; g_pStrategy->perfTick(PerfTickRtnOrder);
    // }
    //
    // Td_RtnOrder rtnOrder;
    // std::memset(&rtnOrder, 0, sizeof(Td_RtnOrder));
    // std::memcpy(rtnOrder.accountId, pRspField->AccountID, sizeof(TXeleUserIDType));
    // rtnOrder.msgType = MsgType_RtnOrder;
    // rtnOrder.bsType = pRspField->Direction;
    // rtnOrder.symbolId = atoi(pRspField->SecuritiesID);
    // rtnOrder.orderSysId = pRspField->OrderSysID;
    // rtnOrder.userLocalId = pRspField->UserLocalID;
    // rtnOrder.origOrdPrice = pRspField->LimitPrice * 10000;
    // rtnOrder.origOrdQty = pRspField->Volume;
    // rtnOrder.origUserLocalId = pRspField->OrigUserLocalID;
    // rtnOrder.origOrderSysId = pRspField->OrigOrderSysID;
    // rtnOrder.ordStatus = pRspField->OrderStatus;
    // rtnOrder.trdQty = pRspField->TradeVolume;
    // rtnOrder.leaveQty = pRspField->LeavesVolume;
    // rtnOrder.errorId = pRspInfo->ErrorID;
    // rtnOrder.requestId = nRequestID;
    //
    // uint32_t index = stoi(pRspField->SecuritiesID) % 1000000;
    // uint32_t channelIndex = CContext::getInstance()->m_SymobolList[index].channelNo % MyConfigure::getInstance()->ChannelNum;
    // sCacheDataType sCachData;
    // sCachData.FnType = MsgType_RtnOrder;
    // std::memcpy(sCachData.FnParam1, &rtnOrder, sizeof(Td_RtnOrder));
    // MarketMaker::getInstance()->m_ChannelPool[channelIndex]->tryPush(sCachData);
};

/// 成交回报
HEAT_ZONE_RSPORDER void XeleTdSpi::onRtnTrade(CXeleRtnTradeField* pRspField, CXeleRspInfo* pRspInfo, int nRequestID, bool bIsLast)
{
    OES::RspOrder rsp;
    rsp.rspType = OES::RspOrder::XeleRtnTrade;
    rsp.userLocalID = g_orderRefLut.orderRefLookup(nRequestID);
    rsp.requestID = nRequestID;
    rsp.errorID = 0;
    rsp.xeleRtnTrade = pRspField;
    MDD::handleRspOrder(rsp);

    // Td_RtnOrder rtnOrder;
    // std::memset(&rtnOrder, 0, sizeof(Td_RtnOrder));
    // std::memcpy(rtnOrder.accountId, pRspField->AccountID, sizeof(TXeleUserIDType));
    // rtnOrder.msgType = MsgType_RtnTrade;
    // rtnOrder.bsType = pRspField->Direction;
    // rtnOrder.symbolId = atoi(pRspField->SecuritiesID);
    // rtnOrder.orderSysId = pRspField->OrderSysID;
    // rtnOrder.userLocalId = pRspField->UserLocalID;
    // rtnOrder.origOrdPrice = pRspField->LimitPrice * 10000;
    // rtnOrder.origOrdQty = pRspField->Volume;
    // rtnOrder.tradeId = pRspField->TradeID;
    // rtnOrder.trdPrice = pRspField->TradePrice * 10000;
    // rtnOrder.trdQty = pRspField->TradeVolume;
    // rtnOrder.cumQty = pRspField->CumQty;
    // rtnOrder.leaveQty = pRspField->LeavesVolume;
    // rtnOrder.ordStatus = pRspField->OrderStatus;
    // rtnOrder.errorId = pRspInfo->ErrorID;
    // rtnOrder.requestId = nRequestID;
    //
    // uint32_t index = stoi(pRspField->SecuritiesID) % 1000000;
    // uint32_t channelIndex = CContext::getInstance()->m_SymobolList[index].channelNo % MyConfigure::getInstance()->ChannelNum;
    // sCacheDataType sCachData;
    // sCachData.FnType = MsgType_RtnTrade;
    // std::memcpy(sCachData.FnParam1, &rtnOrder, sizeof(Td_RtnOrder));
    // MarketMaker::getInstance()->m_ChannelPool[channelIndex]->tryPush(sCachData);
};

/// 报单查询应答
void XeleTdSpi::onRspQryOrder(CXeleRspQryOrderField* pRspField, CXeleRspInfo* pRspInfo, int nRequestID, bool bIsLast)
{
    // Td_RspQryOrder RspQryOrder;
    // std::memset(&RspQryOrder, 0, sizeof(Td_RspQryOrder));
    // RspQryOrder.msgType = MsgType_RspQryOrder;
    // RspQryOrder.symbolId = atoi(pRspField->SecuritiesID);
    // std::memcpy(RspQryOrder.accountId, pRspField->AccountID, sizeof(TXeleUserIDType));
    // RspQryOrder.requestId = nRequestID;                     // 自增长请求号
    // RspQryOrder.userLocalId = pRspField->UserLocalID;       // 用户自定义编号
    // RspQryOrder.orderSysId = pRspField->OrderSysID;         // 系统订单号
    // RspQryOrder.bsType = pRspField->Direction;              // 买卖方向 TdDirectionType
    // RspQryOrder.ordPrice = pRspField->LimitPrice * 10000;   // 委托价格, 单位精确到元后四位, 即1元 = 10000
    // RspQryOrder.ordQty = pRspField->Volume;                 // 委托数量
    // RspQryOrder.trdQty = pRspField->TradeVolume;            // 成交数量
    // RspQryOrder.trdMoney = pRspField->TradeAmount * 100000; // 成交金额 (单位精确到元后四位, 即: 1元=10000)
    // RspQryOrder.ordStatus = pRspField->OrderStatus;         // 订单当前状态 TdOrdStatus
    // RspQryOrder.errorId = pRspInfo->ErrorID;                // 错误码
    // RspQryOrder.isLast = bIsLast;                           // 是否结束
    //
    // sCacheDataType sCachData;
    // sCachData.FnType = MsgType_RspQryOrder;
    // std::memcpy(sCachData.FnParam1, &RspQryOrder, sizeof(Td_RspQryOrder));
    // MarketMaker::getInstance()->m_NotifyPool->tryPush(sCachData);
};

/// 成交查询应答
void XeleTdSpi::onRspQryTrade(CXeleRspQryTradeField* pRspField, CXeleRspInfo* pRspInfo, int nRequestID, bool bIsLast)
{
    // Td_RspQryTrade RspQryTrade;
    // std::memset(&RspQryTrade, 0, sizeof(Td_RspQryTrade));
    // RspQryTrade.msgType = MsgType_RspQryTrade; // 消息类型
    // RspQryTrade.symbolId = atoi(pRspField->SecuritiesID);
    // std::memcpy(RspQryTrade.accountId, pRspField->AccountID, sizeof(TXeleUserIDType));
    // RspQryTrade.requestId = nRequestID;                    // 自增长请求号
    // RspQryTrade.userLocalId = pRspField->UserLocalID;      // 用户自定义编号
    // RspQryTrade.orderSysId = pRspField->OrderSysID;        // 系统订单号
    // RspQryTrade.tradeId = pRspField->TradeID;              // 成交编号
    // RspQryTrade.bsType = pRspField->Direction;             // 买卖方向 TdDirectionType
    // RspQryTrade.trdPrice = pRspField->TradePrice * 10000;  // 成交价格 (单位精确到元后四位, 即: 1元=10000)
    // RspQryTrade.trdQty = pRspField->TradeVolume;           // 成交数量
    // RspQryTrade.trdMoney = pRspField->TradeAmount * 10000; // 成交金额 (单位精确到元后四位, 即: 1元=10000)
    // RspQryTrade.ordStatus = pRspField->OrderStatus;        // 订单当前状态 TdOrdStatus
    // RspQryTrade.errorId = pRspInfo->ErrorID;               // 错误码
    // RspQryTrade.isLast = bIsLast;                          // 是否结束
    //
    // sCacheDataType sCachData;
    // sCachData.FnType = MsgType_RspQryTrade;
    // std::memcpy(sCachData.FnParam1, &RspQryTrade, sizeof(Td_RspQryTrade));
    // MarketMaker::getInstance()->m_NotifyPool->tryPush(sCachData);
};

///////////////////////////////
/////// 证券相关 //////////////
//////////////////////////////

/// 证券资金查询应答
void XeleTdSpi::onRspQryFund(CXeleRspQryStockClientAccountField* pRspField, CXeleRspInfo* pRspInfo, int nRequestID, bool bIsLast)
{
    LOGf(DEBUG, "onRspQryFund:%.15s,AvailableFund:%lf,TotalFund:%lf,InitTotalFund:%lf\n", pRspField->AccountID, pRspField->AvailableFund, pRspField->TotalFund, pRspField->InitTotalFund);

    // Td_RspQryCashAsset RspQryCashAsset;
    // std::memset(&RspQryCashAsset, 0, sizeof(Td_RspQryCashAsset));
    //
    // RspQryCashAsset.msgType = MsgType_RspQryCashasset; // 消息类型
    // std::memcpy(RspQryCashAsset.accountId, pRspField->AccountID, sizeof(TXeleUserIDType));
    // RspQryCashAsset.requestId = nRequestID;                           // 自增长请求号
    // RspQryCashAsset.frozeCapital = pRspField->FrozeCapital * 10000;   /// 冻结资金
    // RspQryCashAsset.frozenFee = pRspField->FrozenFee * 10000;         /// 冻结手续费
    // RspQryCashAsset.usedFee = pRspField->UsedFee * 10000;             /// 已付手续费
    // RspQryCashAsset.initTotalFund = pRspField->InitTotalFund * 10000; /// 初始上场资金（不变）
    // RspQryCashAsset.totalFund = pRspField->TotalFund * 10000;         /// 上场资金（可变）； 初始上场资金 + 出入金额 ，可能为负（建议客户不使用）
    // RspQryCashAsset.sellFund = pRspField->SellFund * 10000;           /// 总卖出
    // RspQryCashAsset.buyFund = pRspField->BuyFund * 10000;             /// 总买入
    // RspQryCashAsset.availableFund = pRspField->AvailableFund * 10000; /// 可用资金
    // RspQryCashAsset.errorId = pRspInfo->ErrorID;                      // 错误码
    // RspQryCashAsset.isLast = bIsLast;                                 // 是否结束
    //
    // sCacheDataType sCachData;
    // sCachData.FnType = MsgType_RspQryCashasset;
    // std::memcpy(sCachData.FnParam1, &RspQryCashAsset, sizeof(Td_RspQryCashAsset));
    // MarketMaker::getInstance()->m_NotifyPool->tryPush(sCachData);
};

/// 证券持仓查询应答
void XeleTdSpi::onRspQryPosition(CXeleRspQryStockPositionField* pRspField, CXeleRspInfo* pRspInfo, int nRequestID, bool bIsLast)
{
    LOGf(DEBUG, "onRspQryPosition:%s,SecuritiesID:%s,AvailablePosition:%ld, errorId:%d,%s\n", pRspField->AccountID, pRspField->SecuritiesID, pRspField->AvailablePosition, pRspInfo->ErrorID, pRspInfo->ErrorMsg);

    // Td_RspQryPosition RspQryPosition;
    // std::memset(&RspQryPosition, 0, sizeof(Td_RspQryPosition));
    //
    // RspQryPosition.msgType = MsgType_RspQryPosition;         // 消息类型
    // RspQryPosition.symbolId = atoi(pRspField->SecuritiesID); // 证券代码
    // std::memcpy(RspQryPosition.accountId, pRspField->AccountID, sizeof(TXeleUserIDType));
    // RspQryPosition.requestId = nRequestID;                           // 自增长请求号
    // RspQryPosition.tdBuyPosition = pRspField->TdBuyPosition;         /// 今买仓
    // RspQryPosition.tdSellPosition = pRspField->TdSellPosition;       /// 今卖仓
    // RspQryPosition.unTdBuyPosition = pRspField->UnTdBuyPosition;     /// 在途买仓
    // RspQryPosition.unTdSellPosition = pRspField->UnTdSellPosition;   /// 在途卖仓
    // RspQryPosition.ydPosition = pRspField->YdPosition;               /// 昨持仓（不变）
    // RspQryPosition.totalCost = pRspField->TotalCost * 10000;         /// 持仓成本
    // RspQryPosition.remainingPosition = pRspField->RemainingPosition; /// 现有持仓数量（含未卖持仓）=老仓 + 今买仓 (-+)出入仓 - 今卖仓
    // RspQryPosition.availablePosition = pRspField->AvailablePosition; /// 可卖持仓数量
    // RspQryPosition.errorId = pRspInfo->ErrorID;                      // 错误码
    // RspQryPosition.isLast = bIsLast;                                 // 是否结束
    //
    // sCacheDataType sCachData;
    // sCachData.FnType = MsgType_RspQryPosition;
    // std::memcpy(sCachData.FnParam1, &RspQryPosition, sizeof(Td_RspQryPosition));
    // MarketMaker::getInstance()->m_NotifyPool->tryPush(sCachData);
};

}

void OES::start(const char *config)
{
    try {
        nlohmann::json json;
        std::ifstream(config) >> json;
        g_username = json["username"];
        g_password = json["password"];

    } catch (std::exception const &e) {
        SPDLOG_ERROR("config json parse failed: {}", e.what());
        throw;
    }

    // SPDLOG_TRACE("oes creating user spi");
    g_userSpi = new XeleTdSpi;
    // SPDLOG_TRACE("oes creating trade api");
    g_tradeApi = XeleSecuritiesTraderApi::createTraderApi();
    // SPDLOG_TRACE("oes registering user api");
    g_tradeApi->registerSpi(g_userSpi);

    SPDLOG_DEBUG("oes call req login");
    int ret = callReqLogin();
    if (ret != 0) {
        /// ret可以结合XeleSecuritiesTraderApi.h中ApiReturnValue枚举返回值对应错误来判断常见的异常
        SPDLOG_ERROR("oes xele login error: ret={}", ret);
        throw std::runtime_error("oes xele login error");
    }
    // SPDLOG_TRACE("oes call req login ok");
}

bool OES::isStarted()
{
	// 此处是为了得到当前api可以执行的权限而做的等待，实际运用时可以直接在回调函数中进行操作
    return g_userSpi->canOrder && g_userSpi->canQuery && g_userSpi->canFundTransfer;
}

void OES::stop()
{
    g_userSpi = nullptr;
    g_tradeApi->release();
    g_tradeApi = nullptr;
}

HEAT_ZONE_REQORDER void OES::sendReqOrder(ReqOrder &reqOrder)
{
    int32_t requestID = g_requestID.fetch_add(1, std::memory_order_relaxed);
    reqOrder.xeleReqOrderInsert.UserLocalID = g_maxUserLocalID + requestID;
    g_tradeApi->reqInsertOrder(reqOrder.xeleReqOrderInsert, requestID);
    g_orderRefLut.setOrderRef(requestID, reqOrder.userLocalID);
}

HEAT_ZONE_REQORDER void OES::sendReqOrderBatch(ReqOrderBatch &reqOrderBatch)
{
    // assume reqOrderBatch.xeleReqBatchOrderInsert.BatchType == BatchMuilt
    uint32_t nBatch = reqOrderBatch.xeleReqBatchOrderInsert.BatchOrderQty;
    int32_t requestID = g_requestID.fetch_add(nBatch, std::memory_order_relaxed);
    uint32_t userLocalIDBase = g_maxUserLocalID + requestID;
    for (uint32_t i = 0; i < nBatch; ++i) {
        reqOrderBatch.xeleReqBatchOrderInsert.ReqOrderInsertField[i].UserLocalID = userLocalIDBase + i;
    }
    g_tradeApi->reqInsertBatchOrder(reqOrderBatch.xeleReqBatchOrderInsert, requestID);
    g_orderRefLut.setOrderRef(requestID, reqOrderBatch.userLocalID);
}

HEAT_ZONE_REQORDER void OES::sendReqCancel(ReqCancel &reqCancel)
{
    int32_t requestID = g_requestID.fetch_add(1, std::memory_order_relaxed);
    reqCancel.xeleReqOrderAction.UserLocalID = g_maxUserLocalID + requestID;
    g_tradeApi->reqCancelOrder(reqCancel.xeleReqOrderAction, requestID);
}

const char *OES::strErrorId(TXeleErrorIdType errorId)
{
    return strXeleError(errorId);
    // return g_tradeApi->getErrMsg(errorId, MARKET_ID);
}
#endif
