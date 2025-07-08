
/////////////////////////////////////////////////////////////////////////
///@system xele-trade-security
///@company 南京艾科朗克信息科技有限公司
///@file XeleSecuritiesTraderApi.h
///@brief 定义了客户端接口
/////////////////////////////////////////////////////////////////////////

#ifndef _XELE_TRADER_API_H
#define _XELE_TRADER_API_H

#include "XeleSecuritiesUserApiStruct.h"
#include <string>
#include <stdint.h>

/********************************************************************************************************************
 ************系统相关：该标记內的函数接口都是系统相关，比如api初始化、柜台登录等*********************************************
 ************证券、期权公用：该标记內的函数接口是证券与期权都需要使用的公有接口，无论证券用户还是期权用户，都会用到该接口*********
 ************证券相关：该标记內的函数是证券用户单独使用的接口，期权用户可以忽略**********************************************
 ************期权相关：该标记內的函数是期权用户单独使用的接口，证券用户可以忽略**********************************************
 ************其他相关：该标记內的函数接口，用户可以忽略*******************************************************************
 *           说明：用户在使用api时，只需要关注对应的模块函数即可,除了系统函数,证券、期权公用接口之外，      *******************
 *                证券用户只需要关注证券相关标记，期权用户只需要关注期权相关标记。                        *******************
 ********************************************************************************************************************/

/**
* @brief api接口返回值含义对照表
* @details 当调用api接口时，正常情况下，返回0表示接口正常调用
           当出现异常时，接口会返回对应的错误码
*/
enum ApiReturnValue : int {
  interfaceCallOk = 0,
  error1,       /// not registerSpi(调用login之前，未调用registerSpi接口)
  error2,       /// api login in process(当前正在处理登录请求)
  error3,       /// OrderWarm param illegal, check fail(OrderWarm参数填写错误，可填值为1或0)
  error4,       /// coreNum param check fail(绑核参数校验失败，可能绑核参数超出了系统可绑核数,此种情况下默认不绑核)
  error5,       /// socket is block or not, check error(socket是否阻塞参数校验错误，应该在0和1中选择)
  error6,       /// SoftRspRcv param illegal, check fail(SoftRspRcv参数填写错误，可填值为1或0)
  error7,       /// AccountID length check error(资金账号长度校验失败，可能没加结束符('\0')或者长度超过柜台限制)
  error8,       /// Password length check error(账号密码长度校验失败，可能没加结束符('\0')或者长度超过柜台限制)
  error9,       /// url check error(url参数校验失败，可能参数格式错误)
  error10,      /// pthread_create fail, manager query tcp rsp thread(艾科管理中心回报线程创建失败，解决问题后需要重启Api)
  error11,      /// pthread_create fail, manager and counter query tcp rsp thread(艾科柜台查询回报线程创建失败，解决问题后需要重启Api)
  error12,      /// manager tcp connect refuse(艾科管理中心Tcp连接失败,可能是URL参数地址错误)
  error13,      /// api_status check error,please restart api(回报中校验Api状态时发生错误，需要重启Api)
  error14,      /// query tcp connect refuse(艾科柜台查询Tcp连接失败,可能是URL参数地址错误)
  error15,      /// trade ip or trade port empty(交易链路地址获取失败，艾科系统发生异常，需要根据具体情况进行检查)
  error16,      /// trade tcp connect refuse(交易链路连接失败，可以登出后再次调用登录接口)
  error17,      /// init trade failed(建立交易链路失败，当前无法进行报撤单操作)
  error18,      /// no free toe_session(当前节点没有交易权限，检查配置文件节点权限)
  error19,      /// flow rebuild failed(期权流水重构失败，可以通过重启Api或者重新登录再次进行尝试)
  error20,      /// input batch type error(批量报单类型错误，目前支持‘1’(等量拆单),‘2’(递减拆单),‘3’(多证券委托组合))
  error21,      /// ApiMode param illegal, check fail(Api运行模式参数校验失败，检查参数)
  error22,      /// api is logining(Api正在登录中，可能是上次的连接未处理结束，正在连接尝试)
  error23,      /// SuperLog param illegal, check fail(是否详细打印日志参数校验失败，检查参数)
  error24,      /// req create trader tcp fail(发送建立交易链路请求失败，可能网络配置问题，检查相关配置)
  error25,      /// send init trader message fail(发送建立交易链路消息失败，可能由于网络问题，可以重新尝试一次)
  error26,      /// DependentCounterVersion param illegal, check fail(一般是由于配置文件中该字段填写格式错误导致（填写的版本号非Api支持的版本）)
  error27,      /// Manager URL Check Fail(多manager地址格式校验错误)
  error28,      /// no valid URL(没有有效URL地址)
  error29,      /// batch muilt order volumn check error(多证券批量报单时，BatchOrderQty字段填写错误，不是实际报单量)
  error30,      /// BatchOrderQty value too large(多证券批量报单数量太大，目前支持一次100单)
  error31,      /// pthread_create fail,counter trade tcp rsp thread(柜台交易链路线程创建失败)
  error32,      /// Query URL Check Fail(地址格式校验错误)
  error33,      /// Trade URL Check Fail(地址格式校验错误)
  error34,      /// check fail,please restart api(内部校验错,需要重启api)
  error35,      /// manager tcp not connect
  error36,      /// query tcp not connect
  error37,      /// api not login
  error38,      /// CreateRtnOrderByRtnTrade param illegal, check fail(Api是否使用rtnTrade报文构造rtnOrder报文参数校验失败，检查参数)
  error39,      /// AutoReLogin param check fail(AutoReLogin 参数校验失败，可填值为0或1)
  error40,      /// LoopRepeatConnect param check fail(LoopRepeatConnect 参数校验失败，可填值为0或1)
  error41,      /// RecvSendDetach param check fail(RecvSendDetach 参数校验失败，可填值为0或1)
  error42,      /// common func thread create fail(通用接口线程创建失败)
  error43,      /// api version compare failed(api版本号校验失败)
  error44,      /// api already logged(Api已经登录, 不允许重复登录)
  error45,      /// CaptureSignal param illegal, check fail(是否捕获异常参数校验失败，检查参数)
  error46,      /// SuperviseExtraInfo too long, more than 128(字段长度超过128个字节，确认后重新填写)
  error47,      /// AllSuperviseInfo too long, more then 384(字段长度超过384个字节，确认后重新填写)
  error48,      /// FpgaRspRcv param illegal, check fail(FpgaRspRcv参数填写错误，可填值为'1'或'0')
  error49,      /// RecvSendDetach param illegal, check fail(RecvSendDetach参数填写错误，可填值为1或0)
  error50,      /// CaptureSignal param illegal, check fail(CaptureSignal参数填写错误，可填值为1或0)
  error51,      /// AutoReLogin param illegal, check fail(AutoReLogin参数填写错误，可填值为1或0)
  error52,      /// LoopRepeatConnect param illegal, check fail(LoopRepeatConnect参数填写错误，可填值为1或0)
  error53,      /// OpenEncryption param illegal, check fail(OpenEncryption参数填写错误，可填值为1或0)
  error54,      /// Get hard disk serial fail(获取硬盘序列号错误)
  apiReturnValueEnd
};

/**
###########################################
########   Api启动参数结构体   ##############
###########################################
# 以下URL至少需要选择ManagerURL或者QueryURL其中之一进行配置，若两个都不配置则会报错。
# 在满足以上条件之后，其余的配置组合方式以是否填写值进行区分，若不为空则使用配置的值，若为空则使用默认值。
*/

/**
###########################################
########         注意        ##############
###########################################

# 当使用的gcc编译器版本是5.1及以上的版本时，需要在编译代码时增加-D_GLIBCXX_USE_CXX11_ABI=0选项
# 示例：
#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g -D_GLIBCXX_USE_CXX11_ABI=0")
*/

struct ConfigParam{
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
  std::string ManagerURL;
  ///艾科柜台查询链路连接地址，使用统一的格式tcp://地址:端口号
  ///此参数为空，使用管理中心默认分配的地址，此参数为合法地址时，使用配置的值。地址、端口映射同理。配置格式如下示例：
  ///示例1：tcp://192.168.4.67:30000
  ///示例2：tcp://192.168.4.67:30000;tcp://192.168.4.68:30000
  ///支持配置主备柜台地址，使用;来分隔，当主柜台连接失败会轮询连接备柜台
  ///Api自动重连也会轮询连接主备柜台
  ///当同时配置了ManagerURL和QueryURL的情况下，Api连接manager成功之后会以配置项为准来连接柜台查询链路
  std::string QueryURL;
  ///艾科报单链路连接地址，使用统一的格式tcp://地址:端口号
  ///此参数为空，使用管理中心默认分配的地址，此参数为合法地址时，使用配置的值。地址、端口映射同理。配置格式如下示例：
  ///示例：tcp://10.128.123.209:30005
  ///场景：例如原报单链路连接地址为tcp://10.128.123.209:30005，由于组网限制，需要将该地址映射为tcp://192.168.0.100:33333
  ///      此时，将TradeURL参数改为：TradeURL=tcp://192.168.0.100:33333；如果没有此种情况只需要如下不填即可。
  ///场景：例如从管理中心查询到的地址为tcp://192.168.4.67:29999，现需要修改改默认地址，
  ///      此时，将TradeURL参数改为：TradeURL=tcp://192.168.4.216:29999，如果没有这种需求，不填即可。
  std::string TradeURL;
  ///用户账号
  std::string AccountId;
  ///用户密码
  std::string Password;
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
  std::string PcPrefix;
  ///交易终端软件名称及版本
  std::string AppID;
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
  std::string businessCode;
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
  std::string solarfareTradeEthName;
  //是否开启登录的时候进行用户名和密码加密
  //api登录manger和柜台的时候是否发送的账户和密码是密文
  // =0 表示关闭
  // =1 表示打开
  // 默认关闭
  int openEncryption = 0;
  //公网信息
  std::string SuperviseExtraInfo;
  //版本校验, XeleSecuritiesTraderApi.h头文件位置
  //校验当前.so内的版本是否和头文件中的版本号匹配
  //填写值了会去获取指定目录下的相关文件,为空不校验
  //填写绝对路径, 默认值为空
  //例如 ApiIncludePath=/home/xele/api/
  std::string ApiIncludePath;
  //此字段填写后,PcPrefix, APPID, SuperviseExtraInfo配置项均不生效
  //客户登录时填写的完整监管信息,此信息不做校验,直接透传至柜台落库
  //最大支持到384字节,不支持中文格式字符,默认为空
  //柜台默认支持长度255，若超过则需要跟券商确认柜台是否支持
  //仅部分券商支持，使用前请确认
  std::string AllSuperviseInfo;
  ///是否捕获异常，默认捕获
  ///=0表示不捕获异常
  ///=1表示捕获异常
  ///说明，此处的异常是指程序中抛出的信号，Api中捕获SIGSEGV和SIGABRT两个信号，当关闭时，Api不处理任何信号
  int CaptureSignal = 1;
};

class XeleSecuritiesTraderSpi {
 public:
  virtual ~XeleSecuritiesTraderSpi() = default;

  ///////////////////////////////
  /////// 系统相关 //////////////
  //////////////////////////////

  ///api与艾科管理中心tcp建连成功回调
  ///该回调表明当前api已经与管理中心完成了tcp的连接，接下来可以进行登录报文交互（该交互不需要用户关心）
  ///正常情况下无需关心此回调，
  ///当发生异常情况时(例如，登录请求发出后，迟迟收不到相关登录响应，当前环境可能出现异常)，
  ///需要简单排查问题，可以通过该回调大致了解到当前api异常模块，方便自行排查问题
  virtual void onFrontManagerQueryConnect(){};

  ///api与艾科柜台查询tcp建连成功回调
  ///该回调表明当前api已经与艾科柜台完成了查询tcp的连接，接下来可以进行登录报文交互（该交互不需要用户关心）
  ///正常情况下无需关心此回调，
  ///当发生异常情况时(例如，登录请求发出后，迟迟收不到相关登录响应，当前环境可能出现异常)，
  ///需要简单排查问题，可以通过该回调大致了解到当前api异常模块，方便自行排查问题
  virtual void onFrontQueryConnect(){};

  ///api与艾科柜台交易tcp建连成功回调
  ///该回调表明当前api已经与艾科柜台完成了交易tcp的连接，接下来可以进行登录报文交互（该交互不需要用户关心）
  ///正常情况下无需关心此回调，当发生异常情况时(例如，登录请求发出后，迟迟收不到相关登录响应，当前环境可能出现异常)，
  ///需要简单排查问题，可以通过该回调大致了解到当前api异常模块，方便自行排查问题
  virtual void onFrontTradeConnect(){};

  ///艾科管理中心登录应答,当只有登录管理中心的需求时，收到该回报即可进行管理中心相关接口操作
  virtual void onRspLoginManager(CXeleRspUserLoginManagerField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) {};

  ///艾科柜台登录应答,当需要管理中心接口可用，但是只需要艾科柜台查询接口可用时，收到该回报即可进行操作
  virtual void onRspLogin(CXeleRspUserLoginField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) {};

  ///添加交易链路应答,当收到该回报时，标记艾科柜台报、撤单接口可用
  virtual void onRspInitTrader(CXeleRspInitTraderField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) {};

  ///密码更新应答
  virtual void onRspUpdatePwd(CXeleRspUserPasswordUpdateField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) {};

  ///艾科管理中心登出应答
  virtual void onRspLogoutManager(CXeleRspUserLogoutManagerField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) {};

  ///艾科柜台登出应答
  virtual void onRspLogout(CXeleRspUserLogoutField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) {};

  ///当客户端与管理中心服务端查询通信连接断开时，该方法被调用。
  ///该回报触发条件为客户主动发起登出请求，或者链路发生了断连。
  ///出现以上情况时，若不是主动发起登出请求，建议先发起登出请求，再重新登录，即可再次使用相关接口
  virtual void onFrontManagerQueryDisconnected(int nReason) {};

  ///当客户端与柜台服务端查询通信连接断开时，该方法被调用。
  ///该回报触发条件为客户主动发起登出请求，或者链路发生了断连。
  ///出现以上情况时，若不是主动发起登出请求，建议先发起登出请求，再重新登录，即可再次使用相关接口
  virtual void onFrontQueryDisconnected(int nReason) {};

  ///当客户端与柜台服务端交易通信连接断开时，该方法被调用。
  ///当收到该回调，表示当前连接失去了报、撤单功能.
  ///如需恢复，参考OnFrontQueryDisconnected处理方法
  virtual void onFrontTradeDisconnected(int nReason) {};

  ///api内部消息打印回调
  virtual void onApiMsg(int ret, const char *strFormat, ...) {};

  ///////////////////////////////
  /////// 证券、期权共用 /////////
  //////////////////////////////

  ///报单应答
  virtual void onRspInsertOrder(CXeleRspOrderInsertField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) {};

  ///报单错误回报
  virtual void onErrRtnInsertOrder(CXeleRspOrderInsertField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) {};

  ///撤单应答
  virtual void onRspCancelOrder(CXeleRspOrderActionField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) {};

  ///撤单错误回报
  virtual void onErrRtnCancelOrder(CXeleRspOrderActionField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) {};

  ///报单回报
  virtual void onRtnOrder(CXeleRtnOrderField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) {};

  ///成交回报
  virtual void onRtnTrade(CXeleRtnTradeField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) {};

  ///报单查询应答
  virtual void onRspQryOrder(CXeleRspQryOrderField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) {};

  ///成交查询应答
  virtual void onRspQryTrade(CXeleRspQryTradeField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) {};

  ///跨柜台资金调拨应答
  virtual void onRspBTFundTransfer(CXeleRspBTCapTransferManagerField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) {};

  ///跨柜台资金调拨结果返回
  virtual void onRtnBTFundTransfer(CXeleRtnBtCapTransferManagerField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) {};

  ///跨柜台资金调拨记录查询应答
  virtual void onRspQryBTFundTransferRecord(CXeleRspQryBTCapTransferManagerField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) {};

  ///////////////////////////////
  /////// 证券相关 //////////////
  //////////////////////////////

  ///费率(印花税率、过户费率、佣金率、流量费)查询应答
  virtual void onRspQryRate(CXeleRspQryStockFeeField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) {};

  ///集中交易资金查询响应
  virtual void onRspQryCentralTradingFund(CXeleRspQryCentralTradingFundField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) {}

  ///集中交易资金调入艾科柜台应答
  virtual void onRspInFund(CXeleRspCapTransferField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) {};

  ///集中交易资金调出艾科柜台应答
  virtual void onRspOutFund(CXeleRspCapTransferField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) {};

  ///集中交易资金调拨艾科柜台明细查询应答
  virtual void onRspQryInOutFundRecord(CXeleRspQryCapTransferRecordField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) {};

  ///集中交易持仓调入艾科柜台应答(中信柜台专用，其他券商不支持)
  virtual void onRspInPosition(CXeleRspPosTransferField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) {};

  ///集中交易持仓调出艾科柜台应答(中信柜台专用，其他券商不支持)
  virtual void onRspOutPosition(CXeleRspPosTransferField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) {};

  ///集中交易持仓调拨艾科柜台明细查询应答(中信柜台专用，其他券商不支持)
  virtual void onRspQryInOutPositionRecord(CXeleRspQryPositionTransferRecordField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) {};

  ///集中交易持仓调入/调出艾科柜台应答(部分券商使用)
  virtual void onRspInOutPosition(CXeleRspInOutPositionField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) {};

  ///集中交易持仓调拨艾科柜台明细查询应答(部分券商使用)
  virtual void onRspQryInOutPositionDetails(CXeleRspQryInOutPositionDetailsField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) {};

  ///用户可用网关信息查询应答(部分券商使用)
  virtual void onRspQryGateWaysRecord(CXeleRspQryAvailableGateWayRecordField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) {};

  ///沪深柜台资金查询应答
  virtual void onRspQryBtFund(CXeleRspQryClientAccountFundManagerField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) {}

  ///证券资金查询应答
  virtual void onRspQryFund(CXeleRspQryStockClientAccountField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) {};

  ///证券持仓查询应答
  virtual void onRspQryPosition(CXeleRspQryStockPositionField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) {};

  ///证券信息查询应答
  virtual void onRspQrySecurities(CXeleRspQryStockSecuritiesField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) {};

  ///权益查询（目前支持新股、配股额度查询）应答
  virtual void onRspQryRightsAndInterests(CXeleRspQryStockQuotaField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) {};

  ///股东账户信息查询应答
  virtual void onRspQryInvestorInfo(CXeleRspQryInvestorInfoField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) {};

  ///程序化风控信息查询应答
  virtual void onRspQryPrcProgramInfo(CXeleRspQryPrcProgramInfoField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) {};

  ///////////////////////////////
  /////// 期权相关 //////////////
  //////////////////////////////

  ///期权流水重构结束应答
  virtual void onRspRebuildFinish(CXeleRspRebuildFinishField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) {};

  ///期权组合报单应答
  virtual void onRspInsertCombOrder(CXeleRspOptionCombInsertField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) {};

  ///期权组合报单错误回报
  virtual void onErrRtnInsertCombOrder(CXeleRspOptionCombInsertField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) {};

  ///期权组合报单回报
  virtual void onRtnCombOrder(CXeleRtnOptionCombOrderField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) {};

  ///期权组合报单成交回报
  virtual void onRtnCombTrade(CXeleRtnOptionCombTradeField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) {};

  ///单腿行权应答
  virtual void onRspInsertExercise(CXeleRspExerciseInsertField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) {};

  ///单腿行权错误回报
  virtual void onErrRtnInsertExercise(CXeleRspExerciseInsertField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) {};

  ///单腿行权撤单应答
  virtual void onRspCancelExercise(CXeleRspOrderActionField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) {};

  ///单腿行权撤单错误回报
  virtual void onErrRtnCancelExercise(CXeleRspOrderActionField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) {};

  ///单腿行权报单回报
  virtual void onRtnExerciseOrder(CXeleRtnExerciseOrderField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) {};

  ///组合行权报单应答
  virtual void onRspInsertExerciseComb(CXeleRspExerciseCombInsertField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) {};

  ///组合行权错误回报
  virtual void onErrRtnInsertExerciseComb(CXeleRspExerciseCombInsertField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) {};

  ///组合行权撤单应答
  virtual void onRspCancelExerciseComb(CXeleRspOrderActionField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) {};

  ///组合行权撤单错误回报
  virtual void onErrRtnCancelExerciseComb(CXeleRspOrderActionField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) {};

  ///组合行权报单回报
  virtual void onRtnExerciseCombOrder(CXeleRtnExerciseCombOrderField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) {};

  ///证券锁定/解锁响应
  virtual void onRspOTU(CXeleRspSecuritiesLockField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) {}

  ///证券锁定/解锁回报
  virtual void onRtnOTU(CXeleRtnSecuritiesLockField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) {}

  ///证券锁定/解锁错误回报
  virtual void onErrRtnOTU(CXeleRspSecuritiesLockField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) {}

  ///期权持仓查询应答
  virtual void onRspQryOptionPosition(CXeleRspQryOptionPositionField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) {};

  ///期权资金查询应答
  virtual void onRspQryOptionFund(CXeleRspQryOptionClientAccountField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) {};

  ///期权合约查询应答
  virtual void onRspQryOptionSecurities(CXeleRspQryOptionSecuritiesField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) {};

  ///期权佣金费率、保证金率查询应答
  virtual void onRspQryOptionRate(CXeleRspQryOptionMarginFeeField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) {};

  ///期权组合持仓查询应答
  virtual void onRspQryOptionCombPosition(CXeleRspQryOptionCombPositionField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) {};

  ///期权资金(出入金)流水明细回报
  virtual void onRtnCapitalTransferDetails(CXeleRtnCapTransferDetailsField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) {}

  //////////////////////////////
  /////// 当前版本暂不支持      //
  //////////////////////////////
  ///回报流水查询应答 (当前版本暂不支持) 期权使用
  virtual void onRspQryOrderFlow(CXeleRspQryOrderFlowField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) {};

  ///会员申请转处置证券账户响应 (Option Trading Transfer For Execution)(当前版本暂不支持) 期权使用
  virtual void onRspOTT(CXeleRspOptionDisposalField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) {}

  ///会员申请转处置证券账户回报 (Option Trading Transfer For Execution)(当前版本暂不支持) 期权使用
  virtual void onRtnOTT(CXeleRtnOptionDisposalField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) {}

  ///会员申请转处置证券账户错误回报 (Option Trading Transfer For Execution)(当前版本暂不支持) 期权使用
  virtual void onErrRtnOTT(CXeleRspOptionDisposalField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) {}

  ///会员申请转处置证券账户撤单响应 (Option Trading Transfer For Execution)(当前版本暂不支持) 期权使用
  virtual void onRspCancelOTT(CXeleRspOrderActionField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) {}

  ///会员申请转处置证券账户撤单错误回报 (Option Trading Transfer For Execution)(当前版本暂不支持) 期权使用
  virtual void onErrRtnCancelOTT(CXeleRspOrderActionField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) {}

  ///期权双边报价响应 (Option Quote Order Entry)(当前版本暂不支持) 期权使用
  virtual void onRspInsertOQO(CXeleRspBilateralOrderInsertField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) {}

  ///期权双边报价回报 (Option Quote Order Entry)(当前版本暂不支持) 期权使用
  virtual void onRtnInsertOQO(CXeleRtnBilateralOrderField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) {}

  ///期权双边报价错误回报 (Option Quote Order Entry)(当前版本暂不支持) 期权使用
  virtual void onErrRtnInsertOQO(CXeleRspBilateralOrderInsertField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) {}

  ///期权双边报价撤单响应 (Option Quote Order Entry)(当前版本暂不支持) 期权使用
  virtual void onRspCancelOQO(CXeleRspOrderActionField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) {}

  ///期权双边报价撤单错误回报 (Option Quote Order Entry)(当前版本暂不支持) 期权使用
  virtual void onErrRtnCancelOQO(CXeleRspOrderActionField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) {}

  ///保证金查询响应 (Option Margin Requirement)(当前版本暂不支持) 期权使用
  virtual void onRspOMR(CXeleRspOptionMarginField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) {}

  ///保证金查询回报 (Option Margin Requirement)(当前版本暂不支持) 期权使用
  virtual void onRtnOMR(CXeleRtnOptionMarginField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) {}

  ///////////////////////////////
  /////// 其他 //////////////////
  //////////////////////////////

  ///通用接口回报
  virtual void onRspCommonFunc(CXeleRspCommonFuncField *pRspField, CXeleRspInfo *pRspInfo, int nRequestID, bool bIsLast) {}
};

class XeleSecuritiesTraderApi {
 public:

  ///////////////////////////////
  /////// 系统相关 //////////////
  //////////////////////////////


  ///创建TraderApi
  ///@return 创建出的UserApi
  static XeleSecuritiesTraderApi *createTraderApi();

  ///获取系统版本号
  ///@return 系统标识字符串
  static const char *getVersion();

  ///@return 错误信息, 获取错误码对应的错误信息
  ///@param error_code 错误码
  ///@param market 市场
  static const char *getErrMsg(unsigned int error_code, char market);

  ///设置API日志路径
  ///@remark 若不设置或设置失败,使用默认路径../logs;建议创建实例前设置,全局调用一次即可,该设置全局有效
  /// \param logPath 日志路径，相对或绝对路径，调用方保证目录存在
  ///@return true设置成功,false,路径不存在;
  static bool setApiLogPath(const char *logPath);

  ///设置是否开启详细日志打印
  ///@remark 若API实例登陆后，遇到问题，可使用此接口实时开启或关闭详细打印;
  ///该设置优先级大于配置参数ConfigParam中的SuperLog;若设置开启，配置参数中的SuperLog无效,若不设置或设置关闭，配置参数中的SuperLog生效
  ///此接口为静态全局接口，影响所有API实例
  /// \param superLog 0表示关闭详细打印,非0表示开启详细打印
  static void setApiSuperLog(int superLog);

  ///等待接口线程结束运行
  ///@return 线程退出代码
  virtual int join() = 0;

  ///删除接口对象本身
  ///@remark 不再使用本接口对象时,调用该函数删除接口对象
  virtual void release() = 0;

  ///注册回调接口
  ///@param pspi 派生自回调接口类的实例
  virtual void registerSpi(XeleSecuritiesTraderSpi *pspi) = 0;

  ///通用功能接口
  /// \param configPath  配置文件路径
  /// \param command 功能号
  /// \param commandStruct 对应报文
  /// \param nRequstID 请求编号
  /// \return  0:表示接口调用正常，其他值表示接口调用异常
  virtual int commonFunc(const char* configPath, const char* command, const char* commandStruct,  char market='1', int node = 0, int nRequstID=0) = 0;

  ///登录请求
  ///@return 0:表示接口调用正常，其他值表示接口调用异常
  /// \param node 若本地配置文件api_config.txt中有此字段且已配置，则此入参可选填，否则必填，建议必填
  /// \param market 若本地配置文件api_config.txt中有此字段且已配置，则此入参可选填，否则必填，建议必填
  virtual int reqLogin(const char* configPath, const char* accountId, const char* password, int node=-1, char market='\0', int nRequestID=0) = 0;
  
  ///获取交易链路URL
  ///@return {交易链路目标IP，交易链路目标PORT}
  virtual std::pair<char*, uint16_t> getTradeUrl() = 0;

  ///裸协议报单时使用,用于初始化报单链路(配置参数RawPacketMode=1)时调用有效
  ///srcIp 该字段需要以‘\0’结尾
  ///@return 0:表示接口调用正常，-1代表初始化失败，-2代表非裸协议模式，其他值表示接口调用异常
  virtual int InitTrader(int nRequestID,const char* srcIp, uint16_t port, uint16_t * sessionId, uint32_t * token) = 0;

  ///登录请求 (不使用配置文件，直接传参)
  ///@return 0:表示接口调用正常，其他值表示接口调用异常
  virtual int reqLoginEx(const ConfigParam* param, int nRequestID) = 0;

  ///密码修改请求
  ///@return 0:表示接口调用正常，其他值表示接口调用异常
  virtual int reqUpdatePwd(CXeleReqUserPasswordUpdateField &inputField, int nRequestID) = 0;

  ///退出请求
  ///@return 0:表示接口调用正常，其他值表示接口调用异常
  virtual int reqLogout(const char* accountId,int nRequestID) = 0;


  ///////////////////////////////
  /////// 证券、期权公用 /////////
  //////////////////////////////
  ///报单请求
  ///@return 0:表示接口调用正常，其他值表示接口调用异常
  virtual int reqInsertOrder(CXeleReqOrderInsertField &inputField, int nRequestID) = 0;

  ///批量报单请求
  ///@return 0:表示接口调用正常，其他值表示接口调用异常
  virtual int reqInsertBatchOrder(CXeleReqBatchOrderInsertField &inputField ,int nRequestID) = 0;

  ///撤单请求
  ///@return 0:表示接口调用正常，其他值表示接口调用异常
  virtual int reqCancelOrder(CXeleReqOrderActionField &inputField, int nRequestID) = 0;

  ///报单查询请求
  ///@return 0:表示接口调用正常，其他值表示接口调用异常
  virtual int reqQryOrder(CXeleReqQryOrderField &inputField, int nRequestID) = 0;

  ///成交查询请求
  ///@return 0:表示接口调用正常，其他值表示接口调用异常
  virtual int reqQryTrade(CXeleReqQryTradeField &inputField, int nRequestID) = 0;

  ///跨柜台资金调拨请求
  ///@return 0:表示接口调用正常，其他值表示接口调用异常
  virtual int reqBTFundTransfer(CXeleReqBTCapTransferManagerField &inputField, int nRequestID) = 0;

  ///跨柜台资金调拨记录查询请求
  ///@return 0:表示接口调用正常，其他值表示接口调用异常
  virtual int reqQryBTFundTransferRecord(CXeleReqQryBTCapTransferManagerField &inputField, int nRequestID) = 0;



  ///////////////////////////////
  /////// 证券相关 //////////////
  //////////////////////////////
  ///费率(印花税率、过户费率、佣金率、流量费)查询请求
  ///@return 0:表示接口调用正常，其他值表示接口调用异常
  virtual int reqQryRate(CXeleReqQryStockFeeField &inputField, int nRequestID) = 0;

  ///集中交易柜台资金查询请求
  ///@return 0:表示接口调用正常，其他值表示接口调用异常
  virtual int reqQryCentralTradingFund(CXeleReqQryCentralTradingFundField &inputField, int nRequestID) = 0;

  ///集中交易资金调入艾科柜台请求
  ///@return 0:表示接口调用正常，其他值表示接口调用异常
  virtual int reqInFund(CXeleReqCapTransferField &inputField, int nRequestID) = 0;

  ///集中交易资金调出艾科柜台请求
  ///@return 0:表示接口调用正常，其他值表示接口调用异常
  virtual int reqOutFund(CXeleReqCapTransferField &inputField, int nRequestID) = 0;

  ///集中交易资金调拨艾科柜台明细查询请求
  ///@return 0:表示接口调用正常，其他值表示接口调用异常
  virtual int reqQryInOutFundRecord(CXeleReqQryCapTransferRecordField &inputField, int nRequestID) = 0;

  ///集中交易持仓调入艾科柜台请求(中信柜台专用，其他券商不支持)
  ///@return 0:表示接口调用正常，其他值表示接口调用异常
  virtual int reqInPosition(CXeleReqPosTransferField &inputField, int nRequestID) = 0;

  ///集中交易持仓调出艾科柜台请求(中信柜台专用，其他券商不支持)
  ///@return 0:表示接口调用正常，其他值表示接口调用异常
  virtual int reqOutPosition(CXeleReqPosTransferField &inputField, int nRequestID) = 0;

  ///集中交易持仓调拨艾科柜台明细查询请求(中信柜台专用，其他券商不支持)
  ///@return 0:表示接口调用正常，其他值表示接口调用异常
  virtual int reqQryInOutPositionRecord(CXeleReqQryPositionTransferRecordField &inputField, int nRequestID) = 0;

  ///集中交易持仓调入/调出艾科柜台请求(部分券商使用)
  ///@return 0:表示接口调用正常，其他值表示接口调用异常
  virtual int reqInOutPosition(CXeleReqInOutPositionField &inputField, int nRequestID) = 0;

  ///集中交易持仓调拨艾科柜台明细查询请求(部分券商使用)
  ///@return 0:表示接口调用正常，其他值表示接口调用异常
  virtual int reqQryInOutPositionDetails(CXeleReqQryInOutPositionDetailsField &inputField, int nRequestID) = 0;

  ///用户可用网关信息查询(部分券商使用)
  ///@return 0:表示接口调用正常，其他值表示接口调用异常
  virtual int reqQryGateWaysRecord(CXeleReqQryAvailableGateWayRecordField &inputField, int nRequestID) = 0;

  ///沪深柜台资金查询请求
  ///@return 0:表示接口调用正常，其他值表示接口调用异常
  virtual int reqQryBtFund(CXeleReqQryClientAccountFundManagerField &inputField, int nRequestID) = 0;

  ///证券资金查询请求
  ///@return 0:表示接口调用正常，其他值表示接口调用异常
  virtual int reqQryFund(CXeleReqQryClientAccountField &inputField, int nRequestID) = 0;

  ///证券持仓查询请求
  ///@return 0:表示接口调用正常，其他值表示接口调用异常
  virtual int reqQryPosition(CXeleReqQryPositionField &inputField, int nRequestID) = 0;

  ///证券信息查询请求
  ///@return 0:表示接口调用正常，其他值表示接口调用异常
  virtual int reqQrySecurities(CXeleReqQrySecuritiesField &inputField, int nRequestID) = 0;

  ///权益查询（目前支持新股、配股额度查询）请求
  ///@return 0:表示接口调用正常，其他值表示接口调用异常
  virtual int reqQryRightsAndInterests(CXeleReqQryStockQuotaField &inputField, int nRequestID) = 0;

  ///股东账号信息查询请求
  ///@return 0:表示接口调用正常，其他值表示接口调用异常
  virtual int reqQryInvestorInfo(CXeleReqQryInvestorInfoField &inputField, int nRequestID) = 0;

  ///程序化风控信息查询(部分券商使用)
  ///@return 0:表示接口调用正常，其他值表示接口调用异常
  virtual int reqQryPrcProgramInfo(CXeleReqQryPrcProgramInfoField &inputField, int nRequestID) = 0;

  ///////////////////////////////
  /////// 期权相关 //////////////
  //////////////////////////////

  ///期权流水重构状态查询
  ///@return 0:表示接口调用正常，其他值表示接口调用异常
  virtual  int getRebuildFlag() = 0;

  ///期权组合报单请求，组合策略构成若有权利仓，其必须在前
  ///@return 0:表示接口调用正常，其他值表示接口调用异常
  virtual int reqInsertCombOrder(CXeleReqOptionCombInsertField &inputField, int nRequestID) = 0;

  ///单腿行权请求
  ///@return 0:表示接口调用正常，其他值表示接口调用异常
  virtual int reqInsertExercise(CXeleReqExerciseInsertField &inputField, int nRequestID) = 0;

  ///单腿行权撤单请求
  ///@return 0:表示接口调用正常，其他值表示接口调用异常
  virtual int reqCancelExercise(CXeleReqOrderActionField &inputField, int nRequestID) = 0;

  ///组合行权请求
  ///@return 0:表示接口调用正常，其他值表示接口调用异常
  virtual int reqInsertExerciseComb(CXeleReqExerciseCombInsertField &inputField, int nRequestID) = 0;

  ///组合行权撤单请求
  ///@return 0:表示接口调用正常，其他值表示接口调用异常
  virtual int reqCancelExerciseComb(CXeleReqOrderActionField &inputField, int nRequestID) = 0;

  ///证券锁定与解锁请求
  ///@return 0:表示接口调用正常，其他值表示接口调用异常
  virtual int reqOTU(CXeleReqSecuritiesLockField &inputField, int nRequestID) = 0;

  ///期权持仓查询请求
  ///@return 0:表示接口调用正常，其他值表示接口调用异常
  virtual int reqQryOptionPosition(CXeleReqQryPositionField &inputField, int nRequestID) = 0;

  ///期权资金查询请求
  ///@return 0:表示接口调用正常，其他值表示接口调用异常
  virtual int reqQryOptionFund(CXeleReqQryClientAccountField &inputField, int nRequestID) = 0;

  ///期权合约查询请求
  ///@return 0:表示接口调用正常，其他值表示接口调用异常
  virtual int reqQryOptionSecurities(CXeleReqQrySecuritiesField &inputField, int nRequestID) = 0;

  ///期权佣金费率、保证金率查询请求
  ///@return 0:表示接口调用正常，其他值表示接口调用异常
  virtual int reqQryOptionRate(CXeleReqQryOptionMarginFeeField &inputField, int nRequestID) = 0;

  ///期权组合持仓查询请求
  ///@return 0:表示接口调用正常，其他值表示接口调用异常
  virtual int reqQryOptionCombPosition(CXeleReqQryOptionCombPositionField &inputField, int nRequestID) = 0;

  //////////////////////////////
  /////// 当前版本暂不支持////////
  //////////////////////////////
  ///会员申请转处置证券账户请求 (Option Trading Transfer For Execution)(当前版本暂不支持) 期权使用
  ///@return 0:表示接口调用正常，其他值表示接口调用异常
  virtual int reqOTT(CXeleReqOptionDisposalField &inputField, int nRequestID) = 0;

  ///会员申请转处置证券账户撤单请求 (Option Trading Transfer For Execution)(当前版本暂不支持) 期权使用
  ///@return 0:表示接口调用正常，其他值表示接口调用异常
  virtual int reqCancelOTT(CXeleReqOrderActionField &inputField, int nRequestID) = 0;

  ///期权双边报价请求 (Option Quote Order Entry)(当前版本暂不支持) 期权使用
  ///@return 0:表示接口调用正常，其他值表示接口调用异常
  virtual int reqInsertOQO(CXeleReqBilateralOrderInsertField &inputField, int nRequestID) = 0;

  ///期权双边报价撤单请求 (Option Quote Order Entry)(当前版本暂不支持) 期权使用
  ///@return 0:表示接口调用正常，其他值表示接口调用异常
  virtual int reqCancelOQO(CXeleReqOrderActionField &inputField, int nRequestID) = 0;

  ///保证金查询请求 (Option Margin Requirement)(当前版本暂不支持) 期权使用
  ///@return 0:表示接口调用正常，其他值表示接口调用异常
  virtual int reqOMR(CXeleReqOptionMarginField &inputField, int nRequestID) = 0;


  ///////////////////////////////
  /////// 其他 //////////////////
  //////////////////////////////
  ///该部分接口暂时不对用户开放

  ///@return 0:表示接口调用正常，其他值表示接口调用异常
  virtual unsigned int getSupervisionServerInfo() = 0;

  virtual ~XeleSecuritiesTraderApi() = default;

};
#endif

#define API_VERSION "3.2.550.p6-238eb4d"
