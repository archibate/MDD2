#include "config.h"
#if XC
#include "krmds.h"
#include "MDS.h"
#include "mds_api/mds_api.h"
#include "sutil/logger/spk_log.h"
#include "securityId.h"
#include "LOG.h"
#include "MDD.h"
#include <thread>
#include <atomic>
#include <fstream>

namespace
{

std::jthread mdThead;

void  KrRelease(void* pApi)
{
    if (pApi)
    {
        MdsApi_LogoutAll(static_cast<MdsApiClientEnvT*>(pApi), TRUE);
        MdsApi_DestoryAll(static_cast<MdsApiClientEnvT*>(pApi));
    }
}

void* KrInitQryApi(const char* cfgFile)
{
    /* 检查API的头文件与库文件版本是否匹配 */
    if (!__MdsApi_CheckApiVersion())
    {
        LOGf(ERROR, "kr api(in header file) version and libVersion not same,application exiting... apiVersion[%s], libVersion[%s]\n",
            MDS_APPL_VER_ID, MdsApi_GetApiVersion());
        return (void*)0;
    }
    else
    {
        LOGf(INFO, "kr api version: %s\n", MdsApi_GetApiVersion());
    }

    MdsApiClientEnvT* cliEnv = new MdsApiClientEnvT{ NULLOBJ_MDSAPI_CLIENT_ENV };

    int retryCnt = 0;
    do
    {
        /* 1. 初始化客户端环境 (配置文件参见: mds_client_sample.conf)
         *
         * @note 提示:
         * - 可以通过指定地址配置项名称 (xxxAddrKey 参数) 来指定需要对接哪些服务, 为空或配置项
         *   未设置则不连接
         * - 本样例仅对接查询服务 ("qryServer")
         * - 如果只需要对接查询服务的话, 可以使用 InitQryChannel 接口替代 InitAll, 示例如下:
         *   - MdsApi_InitQryChannel(&cliEnv.qryChannel,
         *          THE_CONFIG_FILE_NAME, MDSAPI_CFG_DEFAULT_SECTION, "qryServer")
         *
         * 地址配置项说明:
         * - tcpServer: TCP行情订阅服务的地址
         * - qryServer: 查询服务的地址
         */
        if (!MdsApi_InitAll(cliEnv, cfgFile,
            MDSAPI_CFG_DEFAULT_SECTION_LOGGER, MDSAPI_CFG_DEFAULT_SECTION,
            (char*)NULL, "qryServer",
            (char*)NULL, (char*)NULL, (char*)NULL, (char*)NULL))
        {
            {
                SLOG_ERROR("kr mdapi connection establish failed,reconnect in 3 sec...!");
                SPK_SLEEP_MS(1000);
                SLOG_ERROR("kr mdapi connection establish failed,reconnect in 2 sec...!");
                SPK_SLEEP_MS(1000);
                SLOG_ERROR("kr mdapi connection establish failed,reconnect in 1 sec...!");
                SPK_SLEEP_MS(1000);
            }
        }
        else
        {
            LOGf(INFO, "kr mdapi init success...\n");
            return (void*)cliEnv;
        }
    } while (3 > ++retryCnt);  // 最大重试3次

    LOGf(ERROR, "reconnection reach limitation,exiting to check error...\n");

    /* 关闭客户端环境并释放相关资源 */
    if (cliEnv)
    {
        KrRelease(cliEnv);
    }
    return (void*)0;
}

/**
 * 用于处理证券静态信息查询结果的回调函数 (MdsStockStaticInfoT)
 *
 * @param   pQryChannel         查询通道的会话信息
 * @param   pMsgHead            查询应答的消息头
 * @param   pMsgItem            查询应答的数据条目 (需要根据消息类型转换为对应的数据结构)
 * @param   pQryCursor          指示查询进度的游标
 * @param   pCallbackParams     外部传入的参数
 * @retval  >=0                 大于等于0, 成功
 * @retval  <0                  小于0, 处理失败 (负的错误号)
 *
 * @see     MdsStockStaticInfoT
 * @see     eMdsMsgTypeT
 */
int32
OnKrQryStockStaticInfo(MdsApiSessionInfoT* pQryChannel,
    SMsgHeadT* pMsgHead, void* pMsgItem, MdsQryCursorT* pQryCursor,
    void* pCallbackParams)
{
    MdsStockStaticInfoT* pItem = (MdsStockStaticInfoT*)pMsgItem;

    MDS::Stat stat{};

    stat.stock = securityId(pItem->securityId);
    stat.preClosePrice = pItem->prevClose / 100;
    stat.upperLimitPrice = pItem->upperLimitPrice / 100;
    stat.lowerLimitPrice = pItem->lowerLimitPrice / 100;

    MDD::handleStatic(stat);
    return 0;
}

void OnKrL1DynaShot(const char* data, uint8 msgId)
{
    const  MdsMktDataSnapshotT& snapshot = *(MdsMktDataSnapshotT*)data;
    if (snapshot.head.mdProductType != MDS_MD_PRODUCT_TYPE_STOCK) {
        return;
    }
    if (snapshot.head.exchId != MDS_EXCH_SSE && snapshot.head.exchId != MDS_EXCH_SZSE) {
        return;
    }

    MDS::Snap snap{};

    snap.stock = snapshot.head.instrId;
    snap.timestamp = snapshot.head.updateTime;
    snap.numTrades = snapshot.stock.NumTrades;
    snap.volume = snapshot.stock.TotalVolumeTraded;
    snap.amount = snapshot.stock.TotalValueTraded / 100;
    snap.preClosePrice = snapshot.stock.PrevClosePx / 100;
    snap.openPrice = snapshot.stock.OpenPx / 100;
    snap.highPrice = snapshot.stock.HighPx / 100;
    snap.lowPrice = snapshot.stock.LowPx / 100;
    snap.lastPrice = snapshot.stock.TradePx / 100;

    for (int i = 0; i < 5; i++)
    {
        snap.bidPrice[i] = snapshot.stock.OfferLevels[i].Price / 100;
        snap.bidQuantity[i] = snapshot.stock.OfferLevels[i].OrderQty;
        snap.bidPrice[i] = snapshot.stock.BidLevels[i].Price / 100;
        snap.bidQuantity[i] = snapshot.stock.BidLevels[i].OrderQty;
    }
}

int KrQryByFilter(MdsApiSessionInfoT* pQryChannel,
    const MdsQryStockStaticInfoListFilterT* pQryFilter)
{
    int ret = MdsApi_QueryStockStaticInfoList(pQryChannel,
        (char*)NULL, (char*)NULL, pQryFilter,
        OnKrQryStockStaticInfo, nullptr);
    if (__spk_unlikely(ret < 0))
    {
        LOGf(ERROR, "kr qryapi MdsApi_QueryStockStaticInfoList failed! ret[%d]\n", ret);
        return -1;
    }
    else if (__spk_unlikely(ret == 0))
    {
        LOGf(ERROR, "kr qryapi MdsApi_QueryStockStaticInfoList none item return! ret[%d]\n", ret);
        return 0;
    }
    return ret;
}

int KrQryStaticInfo(void* p)
{
    auto  cliEnv = static_cast<MdsApiClientEnvT*>(p);
    auto  pQryChannel = &(cliEnv->qryChannel);
    if (__spk_unlikely(!pQryChannel))
    {
        LOGf(ERROR, "input parameter invalid! pQryChannel[%p]\n", pQryChannel);
        return 0;
    }

    MdsQryStockStaticInfoListFilterT
        qryFilter = { NULLOBJ_MDS_QRY_STOCK_STATIC_INFO_LIST_FILTER };

    memset(&qryFilter, 0, sizeof(MdsQryStockStaticInfoListFilterT));
    qryFilter.exchId = MDS_EXCH_UNDEFINE;
    qryFilter.subSecurityType = MDS_EXCH_UNDEFINE;
    qryFilter.oesSecurityType = 1;  // 股票
    int ret;
    if (0 <= (ret = KrQryByFilter(pQryChannel, &qryFilter)))
    {
        LOGf(DEBUG, "kr QryStaticInfo recv %d items!\n", ret);
        return ret;
    }
    else
    {
        return 0;
    }
}

void KrInitStatic(const char* mds_config)
{
    void* krHandler = KrInitQryApi(mds_config);
    if (!krHandler)
    {
        LOGf(ERROR, "kr qryapi init error!!!\n");
        exit(-1);
    }

    if (0 >= KrQryStaticInfo(krHandler))
    {
        LOGf(ERROR, "kr query static info failed!!!\n");
        exit(-2);
    }
    KrRelease(krHandler);
}

MdsApiClientEnvT* KrInitMdApi(const char* cfgFile)
{
    MdsApiClientEnvT* cliEnv = new MdsApiClientEnvT{ NULLOBJ_MDSAPI_CLIENT_ENV };
    {
        if (MdsApi_InitAllByConvention(cliEnv, cfgFile))
        {
            LOGf(INFO, "kr mdapi init success...\n");
            return cliEnv;
        }
        else
        {
            LOGf(ERROR, "kr mdapi init failed!!!\n");
        }
    }
    KrRelease(cliEnv);
    return nullptr;
}

int32 OnKrMdsMsg(MdsApiSessionInfoT* pSessionInfo,
    SMsgHeadT* pMsgHead, void* pMsgBody, void* pCallbackParams)
{
    if (pMsgHead->msgId <= __MDS_MSGTYPE_SESSION_MAX)
        return 0;

    MdsMktRspMsgBodyT* pRspMsg = (MdsMktRspMsgBodyT*)pMsgBody;

    switch (pMsgHead->msgId)
    {
    case MDS_MSGTYPE_L2_TRADE:
    case MDS_MSGTYPE_L2_ORDER:
    case MDS_MSGTYPE_L2_SSE_ORDER:
    case MDS_MSGTYPE_L2_MARKET_DATA_SNAPSHOT:   
    case MDS_MSGTYPE_INDEX_SNAPSHOT_FULL_REFRESH:
    case MDS_MSGTYPE_OPTION_SNAPSHOT_FULL_REFRESH:
        break;
    case MDS_MSGTYPE_MARKET_DATA_SNAPSHOT_FULL_REFRESH:
    {
         OnKrL1DynaShot((char*)&pRspMsg->mktDataSnapshot, pMsgHead->msgId);
    }
    break;
    case MDS_MSGTYPE_L2_BEST_ORDERS_SNAPSHOT:

        break;
    case MDS_MSGTYPE_MARKET_DATA_REQUEST:

        break;
    default:
        // MDS_MSGTYPE_TRADING_SESSION_STATUS
        if (pMsgHead->msgId != MDS_MSGTYPE_L2_MARKET_OVERVIEW)
        {
            LOGf(WARN, "kr unknow msgId %u!!!\n", pMsgHead->msgId);
        }
    }
    return 0;
}


void  KrRunMd(std::stop_token stop, MdsApiClientEnvT* cliEnv)
{
    LOGf(DEBUG, "kr mdapi running...\n");
    
    int ret = 0;
    while (likely(!stop.stop_requested()))
    {
        ret = MdsApi_WaitOnMsg(&(cliEnv->tcpChannel), 5000, OnKrMdsMsg, nullptr);
        if (unlikely(ret < 0))
        {
            if (likely(SPK_IS_NEG_ETIMEDOUT(ret)))
            {/* 执行超时检查 (检查会话是否已超时) */
                LOGf(WARN, "krmd api timeout\n");
                continue;
            }
            else if (SPK_IS_NEG_EPIPE(ret))
            {
                LOGf(ERROR, "kr mdapi disconnected!!!\n");
            }
            else
            {
                LOGf(ERROR, "kr mdapi error,[%d][%d]!!!\n", ret, SPK_GET_ERRNO());
		    }
            break;
        }
    }
    LOGf(INFO, "kr mdapi exiting %d...\n", ret);
    KrRelease(cliEnv);
}

const char *KrMakeConfig(std::string const &username, std::string const &password)
{
    const char *config = "md_kr_temp.conf";
    std::ofstream(config) << R"(#
# MDS 客户端配置文件
#

##############################################
# 客户端配置
#
# UDP行情组播频道说明:
# - udpServer.Snap1: 快照-频道1, 上海L1/L2快照
# - udpServer.Snap2: 快照-频道2, 深圳L1/L2快照
# - udpServer.Tick1: 逐笔-频道1, 上海逐笔成交/逐笔委托
# - udpServer.Tick2: 逐笔-频道2, 深圳逐笔成交/逐笔委托
#
# 示例, 如何启用对UDP行情数据的本地行情订阅和过滤功能 (仅对异步API有效):
# - [mds_client.async_api]
#   isUdpFilterable = yes
#
# 示例, 如何为不同组播频道分别设置不同的订阅列表:
# - [mds_client]
#   udpServer.Snap1.sse.stock.instrs = 600000, 600096, ...
#   udpServer.Snap1.sse.index.instrs = ...
#   udpServer.Snap1.sse.option.instrs = ...
#   udpServer.Tick1.sse.stock.instrs = ...
#   udpServer.Snap2.szse.stock.instrs = ...
#   udpServer.Tick2.szse.stock.instrs = ...
##############################################

[mds_client]
#udpServer.Snap1 = udp-mcast://232.200.151.100:5301
#udpServer.Snap2 = udp-mcast://232.200.152.100:5302
#udpServer.Tick1 = udp-mcast://232.200.153.100:5303
#udpServer.Tick2 = udp-mcast://232.200.154.100:5304

# 对于async-tcp命令, 可以通过配置 tcpServer.1=...、tcpServer.2=... 来指定多个通道
# 密码支持明文和MD5两种格式 (如 txt:XXX 或 md5:XXX..., 不带前缀则默认为明文
)"
#if SH
"tcpServer = tcp://10.208.40.93:5101\n"
"qryServer = tcp://10.208.40.93:5201\n"
#endif
#if SZ
"tcpServer = tcp://10.240.55.22:5101\n"
"qryServer = tcp://10.240.55.22:5201\n"
#endif

"username = " << username << "\n" <<
"password = " << password << "\n" <<

R"(
heartBtInt = 30

sse.stock.enable = true
#sse.stock.instrs = 600096, 601933

sse.index.enable = false
sse.index.instrs =

sse.option.enable = false
#sse.option.instrs = 10000001, 11001996

szse.stock.enable = true
szse.stock.instrs =

szse.index.enable = false
szse.index.instrs =

szse.option.enable = false
szse.option.instrs =

# 订阅模式 (0: 设置为订阅列表中的股票, 1: 增加订阅列表中的股票, 2: 删除订阅列表中的股票)
mktData.subMode = 0

# 数据模式 (0: 订阅最新快照并跳过过时数据, 1: 订阅最新快照并立即发送, 2: 订阅所有时点的行情快照)
mktData.tickType = 1

# 逐笔数据的数据重建标识 (标识是否订阅重建到的逐笔数据)
# (0: 不订阅重建到的逐笔数据, 1: 订阅重建到的逐笔数据,
#  2: 只订阅重建到的逐笔数据(@note 需要通过压缩行情端口进行订阅, 非压缩行情和组播行情不支持该选项))
mktData.tickRebuildFlag = 0

# 订阅的数据种类
# (0:所有, 1:L1快照/指数/期权, 2:L2快照, 4:L2委托队列, 8:逐笔成交,
#  0x10:深交所逐笔委托, 0x20:上交所逐笔委托, 0x40:L2市场总览, 0x80:逐笔频道心跳消息,
#  0x100:市场状态, 0x200:证券实时状态, 0x400:指数行情, 0x800:期权行情)
# 要订阅多个数据种类, 可以用逗号或空格分隔, 或者设置为并集值, 如:
# "mktData.dataTypes = 1,2,4" 或等价的 "mktData.dataTypes = 0x07"
mktData.dataTypes = 1

# 请求订阅的行情数据的起始时间 (格式: HHMMSS 或 HHMMSSsss)
# (-1: 从头开始获取, 0: 从最新位置开始获取实时行情, 大于0: 从指定的起始时间开始获取)
mktData.beginTime = 0

# 在推送实时行情数据之前, 是否需要推送已订阅产品的初始的行情快照
mktData.isRequireInitialMktData = 0

# 待订阅的内部频道号 (取值范围{{0,1,2,4,8}}, 0或0xFF订阅所有, 配置方式与 dataTypes 类似)
mktData.channelNos = 0

# 行情服务器集群的集群类型 (1: 基于复制集的高可用集群, 2: 基于对等节点的服务器集群, 0: 默认为基于复制集的高可用集群)
clusterType = 0

# 套接字参数配置 (可选配置)
soRcvbuf = 8192
soSndbuf = 1024
connTimeoutMs = 5000
tcpNodelay = 1
quickAck = 1
keepalive = 1
keepIdle = 60
keepIntvl = 5
keepCnt = 9
#mcastInterfaceIp = 192.168.0.11        # 用于接收组播数据的网络设备接口的IP地址
#localSendingIp = 192.168.0.11          # 本地绑定的网络设备接口的IP地址 (适用于发送端)
#localSendingPort = 7001                # 本地绑定的端口地址 (适用于发送端)


##############################################
# 异步API相关的扩展配置
##############################################

[mds_client.async_api]
# 异步队列的大小 (可缓存的消息数量)
asyncQueueSize = 100000
# 是否优先使用大页内存来创建异步队列 (预期大页页面大小为2MB)
isHugepageAble = no
# 是否启动独立的回调线程来执行回调处理 (否则将直接在通信线程下执行回调处理)
isAsyncCallbackAble = no
# 是否启动独立的连接管理线程来执行连接和OnConnect回调处理 (否则将直接在通信线程下执行回调处理)
isAsyncConnectAble = no
# 是否使用忙等待模式 (TRUE:延迟更低但CPU会被100%占用; FALSE:延迟和CPU使用率相对均衡)
isBusyPollAble = yes
# 是否在启动前预创建并校验所有的连接
isPreconnectAble = no
# 是否需要支持对接压缩后的行情数据 (如果可能会对接压缩行情端口, 就将该参数设置为TRUE, 这样就可以同时兼容压缩和非压缩的行情数据)
isCompressible = yes
# 是否启用对UDP行情数据的本地行情订阅和过滤功能 (如果该参数为TRUE, 则允许对UDP行情设置订阅条件, 并在API端完成对行情数据的过滤)
isUdpFilterable = no
# 是否启用内置的查询通道 (TRUE:启动异步API时自动创建内置的查询通道; FALSE:不创建内置的查询通道)
isBuiltinQueryable = yes

# 异步I/O线程配置 (可选配置)
ioThread.enable = yes                   # I/O线程的使能标志
ioThread.isOutputSimplify = no          # 是否采用精简模式输出数据
ioThread.isAppendMode = no              # 是否采用追加模式输出数据
ioThread.isIoThreadBusyPollAble = no    # I/O线程是否使用忙等待模式 (仅用于延迟测量场景, 否则I/O线程没有必要使用忙等待模式)
ioThread.autoTimeSyncInterval = 0       # 自动执行时间同步的间隔时间 (单位为秒, 必须启用内置的查询通道才能生效. 小于等于0:不自动执行时间同步)
ioThread.clockDriftBeginTime = 0        # 统计时钟漂移情况的起始时间 (格式: HHMMSS 或 HHMMSSsss. 小于等于0:默认从09:10开始统计时钟漂移情况)
ioThread.dataOutputFormat = json        # 数据输出格式 (json, csv, poc, binary, none)
# 数据文件路径 (为空:不输出数据; stdout/stderr:标准输出)
#ioThread.dataOutputPath = ./kr-mkdata.txt
# 统计信息文件路径 (为空:默认输出到日志文件中; stdout/stderr:标准输出)
#ioThread.statsOutputPath = ./kr-stats.txt


##############################################
# 日志配置
##############################################

[log]
log.root_category = INFO, console_log
log.mode=file
log.threshold=TRACE
log.file=./kr-mds.log
log.file.max_file_length=300M
log.file.max_backup_index=3

[console_log]
log.mode=console
log.threshold=ERROR


############################################################
# CPU亲和性设置
#
# 配置说明:
# - CPU编号从1开始, CPU编号列表以逗号或空格分割
# - 使能标志 (cpuset.enable), 若未设置则默认启用亲和性设置
# - 默认值 (cpuset.default), CPU亲和性配置的默认值 (默认的CPU绑定配置, 建议与通信线程/回调线程在一个NUMA节点)
############################################################

[cpuset]
enable = no
default = 0

# 仅适用于同步测试工具的CPU亲和性配置
tcp_captor = 3
tcp_captor[0] = 3
tcp_captor[1] = 5

# 仅适用于同步测试工具CPU亲和性配置
udp_captor = 3
query = 2
#test_req = 2

# 异步API线程的CPU亲和性配置
# - 通信线程 (mdsapi_communication): 关键线程, 需要分配一个独立的CPU核
# - 回调线程 (mdsapi_callback): 关键线程, 需要分配一个独立的CPU核
# - 连接管理线程 (mdsapi_connect): 辅助线程(默认关闭), 使用默认的CPU核就可以 (与通信线程/回调线程在一个NUMA节点)
# - 异步I/O线程 (mdsapi_io_thread): 辅助线程(默认关闭), 分配到一个公共的CPU核, 避免影响关键线程就可以
mdsapi_communication = 3
mdsapi_callback = 5
#mdsapi_connect = 1
mdsapi_io_thread = 4
)";
    return config;
}

}

bool KrRun(std::string const &username, std::string const &password)
{
    const char *mds_config = KrMakeConfig(username, password);
    LOGf(DEBUG, "kr config file: %s\n", mds_config);

    LOGf(DEBUG, "kr ready to get static info...\n");
    KrInitStatic(mds_config);
    LOGf(DEBUG, "kr ready to get dynashot...\n");
    ////////////////////////////////////////////////////////////
    MdsApiClientEnvT* krHandler = KrInitMdApi(mds_config);
    if (!krHandler)
    {
        LOGf(ERROR, "kr mdapi init error!!!\n");
        return false;
    }
    ////////////////////////////////////////////////////////////
    LOGf(INFO, "kr start run to get dynashot...\n");
    ////////////////////////////////////////////////////////////
    mdThead = std::jthread([krHandler] (std::stop_token stop) {
        KrRunMd(stop, krHandler);
    });
    return true;
}

void KrStop()
{
    mdThead.request_stop();
    mdThead.join();
}
#endif
