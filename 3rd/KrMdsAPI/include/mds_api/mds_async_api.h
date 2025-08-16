/*
 * Copyright 2020 the original author or authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @file    mds_async_api.h
 *
 * MDS (行情数据服务系统) 异步接口库
 *
 * @version 0.15.9.1    2019/07/01
 *          - 增加支持异步行情接收的API接口库
 *              - 支持异步行情接收
 *              - 支持自动的连接管理 (自动识别异常并重建连接)
 *          - 样例代码参见:
 *              - samples/01_mds_async_tcp_sample.c
 * @version 0.15.9.4    2019/11/19
 *          - 支持在运行过程中重新指定行情订阅条件
 *          - 调整 OnConnect 回调函数的返回值约定 @see F_MDSAPI_ASYNC_ON_DISCONNECT_T
 * @version 0.15.10     2019/12/20
 *          - 重构异步API, 统一交易和行情的异步接口
 * @version 0.15.11     2020/05/29
 *          - 增加辅助的异步API接口
 *              - MdsAsyncApi_IsAllTerminated, 返回异步API相关的所有线程是否都已经安全退出
 *          - 删除并不适用于行情异步API的密码修改接口
 *              - MdsAsyncApi_SendChangePasswordReq
 * @version 0.15.11.6   2020/07/16
 *          - 增加辅助的异步API接口
 *              - MdsAsyncApi_GetAsyncQueueTotalCount, 返回异步API累计已入队的消息数量
 *              - MdsAsyncApi_GetAsyncQueueRemainingCount, 返回队列中尚未被处理的剩余数据数量
 * @version 0.15.11.16  2021/02/20
 *          - 增加辅助的异步API接口, 以支持对通信线程、回调线程等异步API线程进行初始化处理
 *              - MdsAsyncApi_SetOnCommunicationThreadStart, 设置通信线程的线程初始化回调函数
 *              - MdsAsyncApi_SetOnCallbackThreadStart, 设置回调线程的线程初始化回调函数
 *              - MdsAsyncApi_SetOnIoThreadStart, 设置异步I/O线程的线程初始化回调函数
 *          - 增加辅助的异步API接口, 以方便对接
 *              - MdsAsyncApi_DefaultOnConnect, 连接完成后处理的默认实现 (执行默认的行情订阅处理)
 *          - 增加辅助的行情订阅接口
 *              - MdsAsyncApi_SubscribeByQuery, 查询证券静态信息并根据查询结果订阅行情信息
 *          - 增加内置的查询通道和相关接口, 以整合查询通道管理和查询接口到异步API中
 *              - MdsAsyncApi_SetBuiltinQueryable, 设置是否启用内置的查询通道
 *              - MdsAsyncApi_IsBuiltinQueryable, 返回是否启用内置的查询通道
 *              - MdsAsyncApi_SetBuiltinQueryChannelCfg, 设置内置的查询通道的配置信息
 *              - MdsAsyncApi_LoadBuiltinQueryChannelCfg, 从配置文件中加载内置的查询通道的配置信息
 *              - MdsAsyncApi_GetBuiltinQueryChannelCfg, 返回内置的查询通道的配置信息
 *              - MdsAsyncApi_GetBuiltinQueryChannelRef, 返回内置的查询通道的会话信息
 *          - 增加查询接口的包裹函数 (基于异步API内置的查询通道执行)
 *              - MdsAsyncApi_QueryMktDataSnapshot, 查询证券行情快照
 *              - MdsAsyncApi_QuerySnapshotList, 批量查询行情快照
 *              - MdsAsyncApi_QuerySnapshotList2, 批量查询行情快照
 *              - MdsAsyncApi_QuerySecurityStatus, 查询(深圳)证券实时状态
 *              - MdsAsyncApi_QueryTrdSessionStatus, 查询(上证)市场状态
 *              - MdsAsyncApi_QueryStockStaticInfoList, 批量查询证券(股票/债券/基金)静态信息列表
 *              - MdsAsyncApi_QueryStockStaticInfoList2, 批量查询证券(股票/债券/基金)静态信息列表
 * @version 0.16.1.11   2021/02/22
 *          - 增加查询接口的包裹函数 (基于异步API内置的查询通道执行)
 *              - MdsAsyncApi_QueryOptionStaticInfoList, 批量查询期权合约静态信息列表
 *              - MdsAsyncApi_QueryOptionStaticInfoList2, 批量查询期权合约静态信息列表
 * @version 0.17.6.1    2022/04/13
 *          - 增加辅助的异步API配置项和配置接口, 以支持启动独立的连接管理线程来执行连接处理和OnConnect回调处理
 *              - MdsAsyncApi_SetAsyncConnectAble, 设置是否启动独立的连接管理线程来执行连接处理和OnConnect回调处理
 *              - MdsAsyncApi_IsAsyncConnectAble, 返回是否启动独立的连接管理线程来执行连接处理和OnConnect回调处理
 *              - MdsAsyncApi_LoadCpusetCfg2, 从配置文件中加载CPU亲和性配置 (额外增加对连接管理线程的支持)
 *              - MdsAsyncApi_SetConnectThreadCpusetCfg, 设置连接管理线程的CPU亲和性配置
 *              - MdsAsyncApi_GetConnectThreadCpusetCfg, 返回连接管理线程的CPU亲和性配置信息
 *          - 增加回调函数设置接口, 并增加连接失败时的回调函数接口
 *              - MdsAsyncApi_SetOnConnect, 设置连接或重新连接完成后的回调函数
 *              - MdsAsyncApi_GetOnConnect, 返回连接或重新连接完成后的回调函数
 *              - MdsAsyncApi_SetOnDisconnect, 设置连接断开后的回调函数
 *              - MdsAsyncApi_GetOnDisconnect, 返回连接断开后的回调函数
 *              - MdsAsyncApi_SetOnConnectFailed, 设置连接失败时的回调函数
 *              - MdsAsyncApi_GetOnConnectFailed, 返回连接失败时的回调函数
 * @version 0.17.6.2    2022/05/26
 *          - 增加辅助的异步API接口
 *              - MdsAsyncApi_IsBuiltinQueryChannelConnected, 返回内置的查询通道是否已连接就绪
 * @version 0.17.6.3_u5 2022/11/07
 *          - 增加修改客户端登录密码接口的包裹函数 (基于异步API内置的查询通道执行)
 *              - MdsAsyncApi_SendChangePasswordReq, 发送密码修改请求 (修改客户端登录密码)
 *          - 增加辅助的异步API接口, 以提供登录后不订阅任何行情数据的快捷方法
 *              - MdsAsyncApi_SubscribeNothingOnConnect, 连接完成后处理的默认实现 (不订阅任何行情数据)
 *          - 新增逐笔数据重传接口, 客户端可以通过该接口回补缺失的逐笔数据
 *              - MdsAsyncApi_SendTickResendRequest, 发送逐笔数据重传请求
 *              - MdsAsyncApi_SendTickResendRequest2, 发送逐笔数据重传请求
 *              - MdsAsyncApi_SendTickResendRequestHugely, 发送超大的逐笔数据重传请求 (支持不限制大小的重传请求, @note 请谨慎使用该接口, 避免发起过大或不必要的重传请求)
 * @version 0.17.6.8_u1 2023/03/03
 *          - 添加辅助的连接管理接口, 用于控制是否需要暂停连接重建处理等特殊场景
 *              - __MdsAsyncApi_SetChannelSuspended, 设置通道的连接管理标志为 '暂停重连', 以暂停该通道的连接重建处理
 *              - __MdsAsyncApi_IsChannelSuspended, 返回通道的连接管理标志是否为 '暂停重连'
 *              - __MdsAsyncApi_GetChannelSuspendExpirationTime, 返回通道暂停重连的过期时间
 * @version 0.17.6.10   2023/07/05
 *          - 增加辅助的异步API接口, 返回会话信息对应的异步API连接通道信息
 *              - MdsAsyncApi_GetChannelBySession, 返回会话信息对应的异步API连接通道信息
 * @version 0.17.6.11   2023/10/07
 *          - 重构异步API, 优化委托发送延迟, 并增加对 onload 优化选项的支持
 *              - 增加配置参数 'Onload 加速标志 (onloadFlag)'
 *              - @note 在 Linux 平台下, 编译时需要增加链接参数 -ldl
 *          - 增加辅助的异步API接口
 *              - MdsAsyncApi_GetTotalIoPicked, 返回异步I/O线程累计已提取和处理过的消息数量
 *              - MdsAsyncApi_LoadContextParams, 从配置文件中加载异步API运行参数
 *              - MdsAsyncApi_SetOnloadFlag, 设置 Onload 加速标志
 *              - MdsAsyncApi_GetOnloadFlag, 返回 Onload 加速标志
 *              - MdsAsyncApi_SetIoThreadEnabled, 设置异步I/O线程的使能标志
 *              - MdsAsyncApi_IsIoThreadEnabled, 返回异步I/O线程的使能标志
 *          - 添加辅助的连接管理接口
 *              - __MdsAsyncApi_TriggerChannelBroken, 触发连接断开事件, 主动断开连接
 * @version 0.17.6.14_u1 2024/09/18
 *          - 扩展异步API消息队列支持的最大消息大小, 为兼容日后的消息大小扩展做准备
 *
 * @since   2019/07/01
 */


#ifndef _MDS_ASYNC_API_H
#define _MDS_ASYNC_API_H


#include    <mds_api/mds_api.h>
#include    <sutil/net/spk_general_endpoint_define.h>


#ifdef __cplusplus
extern "C" {
#endif


/* ===================================================================
 * 常量定义
 * =================================================================== */

/** 可以同时连接的远程服务器的最大数量 (256) */
#define MDSAPI_ASYNC_MAX_REMOTE_CNT             SPK_ENDPOINT_MAX_REMOTE_CNT

/** 默认的异步API配置区段名称 */
#define MDSAPI_CFG_DEFAULT_SECTION_ASYNC_API    "async_api"
/** 默认的CPU亲和性配置区段名称 */
#define MDSAPI_CFG_DEFAULT_SECTION_CPUSET       "cpuset"

/** 默认的异步API线程的CPU亲和性配置项名称 (通信线程) */
#define MDSAPI_CFG_DEFAULT_KEY_CPUSET_COMMUNICATION                     \
        "mdsapi_communication"
/** 默认的异步API线程的CPU亲和性配置项名称 (异步回调线程) */
#define MDSAPI_CFG_DEFAULT_KEY_CPUSET_CALLBACK                          \
        "mdsapi_callback"
/** 默认的异步API线程的CPU亲和性配置项名称 (连接管理线程) */
#define MDSAPI_CFG_DEFAULT_KEY_CPUSET_CONNECT                           \
        "mdsapi_connect"
/** 默认的异步API线程的CPU亲和性配置项名称 (I/O线程) */
#define MDSAPI_CFG_DEFAULT_KEY_CPUSET_IO_THREAD                         \
        "mdsapi_io_thread"
/* -------------------------           */


/* ===================================================================
 * 结构体定义
 * =================================================================== */

/**
 * MDS异步API线程的通道配置信息
 */
typedef SEndpointChannelCfgT        MdsAsyncApiChannelCfgT;


/* 结构体的初始化值定义 */
#define NULLOBJ_MDSAPI_ASYNC_CHANNEL_CFG                                \
        NULLOBJ_SPK_ENDPOINT_CHANNEL_CFG
/* -------------------------           */


/**
 * MDS异步API的I/O线程配置信息 (用于异步落地行情数据, 可以通过配置文件开启或禁用)
 */
typedef SEndpointIoThreadCfgT       MdsAsyncApiIoThreadCfgT;


/* 结构体的初始化值定义 */
#define NULLOBJ_MDSAPI_ASYNC_IO_THREAD_CFG                              \
        NULLOBJ_SPK_ENDPOINT_IO_THREAD_CFG
/* -------------------------           */


/**
 * MDS异步API的运行时上下文环境
 * 该结构体下的字段均为API内部使用和管理, 客户端不应直接修改该结构体下的数据
 */
typedef SEndpointContextT           MdsAsyncApiContextT;


/* 结构体的初始化值定义 */
#define NULLOBJ_MDSAPI_ASYNC_CONTEXT                                    \
        NULLOBJ_SPK_ENDPOINT_CONTEXT
/* -------------------------           */


/**
 * MDS异步API的连接通道运行时信息
 */
typedef SEndpointChannelT           MdsAsyncApiChannelT;


/* 结构体的初始化值定义 */
#define NULLOBJ_MDSAPI_ASYNC_CHANNEL                                    \
        NULLOBJ_SPK_ENDPOINT_CHANNEL
/* -------------------------           */


/**
 * MDS异步API的上下文环境的创建参数 (仅做为 CreateContext 接口的参数使用)
 */
typedef struct _MdsAsyncApiContextParams {
    /** 异步队列的大小 */
    int32               asyncQueueSize;

    /** 是否优先使用大页内存来创建异步队列 */
    uint8               isHugepageAble;
    /** 是否启动独立的回调线程来执行回调处理 (否则将直接在通信线程下执行回调处理) */
    uint8               isAsyncCallbackAble;
    /** 是否启动独立的连接管理线程来执行连接处理和OnConnect回调处理 (当通道数量大于1时建议开启, 否则将直接在通信线程下执行) */
    uint8               isAsyncConnectAble;
    /** 是否使用忙等待模式 (TRUE:延迟更低但CPU会被100%占用; FALSE:延迟和CPU使用率相对均衡) */
    uint8               isBusyPollAble;
    /** 是否在启动前预创建并校验所有的连接 */
    uint8               isPreconnectAble;
    /** 是否启用内置的查询通道 (TRUE:启动异步API时自动创建内置的查询通道; FALSE:不创建内置的查询通道) */
    uint8               isBuiltinQueryable;

    /**
     * Onload 加速标志
     * - 0: 未启用 onload 加速
     * - 1: 已启用 onload 加速
     * - 2: 自动管理 onload 栈
     */
    uint8               onloadFlag;
    /** 是否需要支持对接压缩后的行情数据 */
    uint8               isCompressible;
    /** 是否启用对UDP行情数据的本地行情订阅和过滤功能 */
    uint8               isUdpFilterable;
    /** 为保证64位对齐而设 */
    uint8               __filler1[3];

    /** 自动执行时间同步的间隔时间 (单位为秒, 必须启用内置的查询通道才能生效. 小于等于0:不自动执行时间同步) */
    int16               autoTimeSyncInterval;
    /** 为保证64位对齐而设 */
    uint8               __filler2[2];
    /** 统计时钟漂移情况的起始时间 (格式: HHMMSS 或 HHMMSSsss. 小于等于0:默认从09:10开始统计时钟漂移情况) */
    int32               clockDriftBeginTime;
} MdsAsyncApiContextParamsT;


/* 结构体初始化值定义 */
#define NULLOBJ_MDSAPI_ASYNC_CONTEXT_PARAMS                             \
        0, \
        0, 0, 0, 0, 0, 0, \
        0, 0, 0, {0}, \
        0, {0}, 0


/* 结构体的默认值定义 */
#define DEFAULT_MDSAPI_ASYNC_CONTEXT_PARAMS                             \
        -1, \
        TRUE, TRUE, FALSE, TRUE, TRUE, FALSE, \
        2, TRUE, FALSE, {0}, \
        0, {0}, 0
/* -------------------------           */


/* ===================================================================
 * 回调函数的函数原型定义
 * =================================================================== */

/**
 * 对接收到的应答或回报消息进行处理的回调函数的函数原型定义
 *
 * <p> 回调函数说明:
 * - 和 #F_MDSAPI_ONMSG_T 的定义一致, 回调函数可以通用
 * - 对消息体数据(pMsgItem), 需要按照消息类型(pMsgHead->msgId)转换为对应的消息结构进行处理
 * - 具体使用方式可以参考样例代码中的 MdsApiSample_HandleMsg 函数
 * - @note 当使用异步回调模式时, 应尽量避免使用会话信息中的数据
 * </p>
 *
 * <p> 线程说明:
 * - OnMsg/OnConnect/OnDisconnect 回调函数均运行在异步API线程下
 * </p>
 *
 * <p> 逐笔数据消息类型与数据结构的对应关系如下:
 * - MDS_MSGTYPE_L2_TRADE                           => @see MdsL2TradeT
 * - MDS_MSGTYPE_L2_ORDER                           => @see MdsL2OrderT
 * - MDS_MSGTYPE_L2_SSE_ORDER                       => @see MdsL2OrderT
 * - MDS_MSGTYPE_L2_TICK_CHANNEL_HEARTBEAT          => @see MdsTickChannelHeartbeatT
 * </p>
 *
 * <p> 快照行情消息类型与数据结构的对应关系如下:
 * - MDS_MSGTYPE_L2_MARKET_DATA_SNAPSHOT            => @see MdsMktDataSnapshotT, MdsL2StockSnapshotBodyT
 * - MDS_MSGTYPE_L2_BEST_ORDERS_SNAPSHOT            => @see MdsMktDataSnapshotT, MdsL2BestOrdersSnapshotBodyT
 * - MDS_MSGTYPE_L2_MARKET_OVERVIEW                 => @see MdsMktDataSnapshotT, MdsL2MarketOverviewT
 * - MDS_MSGTYPE_MARKET_DATA_SNAPSHOT_FULL_REFRESH  => @see MdsMktDataSnapshotT, MdsStockSnapshotBodyT
 * - MDS_MSGTYPE_INDEX_SNAPSHOT_FULL_REFRESH        => @see MdsMktDataSnapshotT, MdsIndexSnapshotBodyT
 * - MDS_MSGTYPE_OPTION_SNAPSHOT_FULL_REFRESH       => @see MdsMktDataSnapshotT, MdsStockSnapshotBodyT
 * - MDS_MSGTYPE_TRADING_SESSION_STATUS             => @see MdsTradingSessionStatusMsgT
 * - MDS_MSGTYPE_SECURITY_STATUS                    => @see MdsSecurityStatusMsgT
 * </p>
 *
 * @param   pSessionInfo        会话信息
 * @param   pMsgHead            回报消息的消息头
 * @param   pMsgItem            回报消息的数据条目 (需要根据消息类型转换为对应的数据结构)
 * @param   pCallbackParams     外部传入的参数
 * @retval  >=0                 大于等于0, 成功
 * @retval  <0                  小于0, 处理失败, 将尝试断开并重建连接
 *
 * @see     eMdsMsgTypeT
 * @see     MdsMktRspMsgBodyT
 */
typedef F_MDSAPI_ONMSG_T        F_MDSAPI_ASYNC_ON_MSG_T;


/**
 * 异步API线程连接或重新连接完成后的回调函数的函数原型定义
 *
 * <p> 回调函数说明:
 * - 对于TCP通道, 需要通过该回调函数完成行情订阅操作。若函数指针为空, 则会使用通道配置中默认的
 *   订阅参数进行订阅。若函数指针不为空, 但未订阅回报, 90秒以后服务器端会强制断开连接
 * - 若回调函数返回小于0的数, 则异步API将中止运行
 * </p>
 *
 * <p> 线程说明:
 * - OnMsg/OnConnect/OnDisconnect 回调函数均运行在异步API线程下
 * </p>
 *
 * @param   pAsyncChannel       异步API的连接通道信息
 * @param   pCallbackParams     外部传入的参数
 * @retval  =0                  等于0, 成功
 * @retval  >0                  大于0, 处理失败, 将重建连接并继续尝试执行
 * @retval  <0                  小于0, 处理失败, 异步API将中止运行
 */
typedef int32   (*F_MDSAPI_ASYNC_ON_CONNECT_T) (
                MdsAsyncApiChannelT *pAsyncChannel,
                void *pCallbackParams);


/**
 * 异步API线程连接断开后的回调函数的函数原型定义
 *
 * <p> 回调函数说明:
 * - 仅用于通知客户端连接已经断开, 无需做特殊处理, 异步API会自动尝试重建连接
 * - 若函数指针为空, 异步API会自动尝试重建连接并继续执行
 * - 若回调函数返回小于0的数, 则异步API将中止运行
 * </p>
 *
 * <p> 线程说明:
 * - OnMsg/OnConnect/OnDisconnect 回调函数均运行在异步API线程下
 * </p>
 *
 * @param   pAsyncChannel       异步API的连接通道信息
 * @param   pCallbackParams     外部传入的参数
 * @retval  >=0                 大于等于0, 异步API将尝试重建连接并继续执行
 * @retval  <0                  小于0, 异步API将中止运行
 */
typedef int32   (*F_MDSAPI_ASYNC_ON_DISCONNECT_T) (
                MdsAsyncApiChannelT *pAsyncChannel,
                void *pCallbackParams);

/**
 * 异步API线程初始化函数的函数原型定义
 *
 * <p> 回调函数说明:
 * - 若回调函数返回小于0的数, 则异步API将中止运行
 * </p>
 *
 * <p> 线程说明:
 * - 如果为通信线程或回调线程指定了初始化函数, 则线程启动后将回调该初始化函数
 * </p>
 *
 * @param   pAsyncChannel       异步API的连接通道信息
 * @param   pCallbackParams     外部传入的参数
 * @retval  >=0                 大于等于0, 成功
 * @retval  <0                  小于0, 处理失败, 异步API将中止运行
 */
typedef int32   (*F_MDSAPI_ASYNC_ON_THREAD_START_T) (
                MdsAsyncApiContextT *pAsyncContext,
                void *pCallbackParams);
/* -------------------------           */


/* ===================================================================
 * 上下文管理接口
 * =================================================================== */

/**
 * 创建异步API的运行时环境 (通过配置文件和默认的配置区段加载相关配置参数)
 *
 * - 与 MdsAsyncApi_CreateContextSimple 接口的区别为, 会通过配置文件加载如下配置:
 *   - 日志配置
 *   - 异步API相关的扩展配置: 异步队列的大小、是否启动独立的回调线程来执行回调处理、是否使用
 *     忙等待模式、是否需要支持对接压缩后的行情数据等
 *   - 异步I/O线程配置 (@see MdsAsyncApi_LoadIoThreadCfg)
 *   - CPU亲和性配置 (@see MdsAsyncApi_LoadCpusetCfg)
 *
 * @param   pCfgFile            配置文件路径 (可为空, 为空则不加载配置文件)
 * @return  非空, 异步API的运行时环境指针; NULL, 失败
 */
MdsAsyncApiContextT *
        MdsAsyncApi_CreateContext(
                const char *pCfgFile);

/**
 * 创建异步API的运行时环境 (通过配置文件和指定的配置区段加载相关配置参数)
 *
 * - 与 MdsAsyncApi_CreateContextSimple 接口的区别为, 会通过配置文件加载如下配置:
 *   - 日志配置
 *   - 异步API相关的扩展配置: 异步队列的大小、是否启动独立的回调线程来执行回调处理、是否使用
 *     忙等待模式、是否需要支持对接压缩后的行情数据等
 *   - 异步I/O线程配置 (@see MdsAsyncApi_LoadIoThreadCfg)
 *   - CPU亲和性配置 (@see MdsAsyncApi_LoadCpusetCfg)
 *
 * @param   pCfgFile            配置文件路径 (可为空, 为空则不加载配置文件)
 * @param   pLoggerSection      日志记录器的配置区段名称 (e.g. "log")
 *                              - 为空则忽略, 不初始化日志记录器
 * @param   pAsyncApiSection    异步API扩展配置参数的配置区段名称 (e.g. "mds_client.async_api")
 *                              - 为空则忽略, 不加载异步API相关的扩展配置参数
 * @param   pCpusetSection      CPU亲和性配置的配置区段名称 (e.g. "cpuset")
 *                              - 为空则忽略, 不加载CPU亲和性配置
 * @return  非空, 异步API的运行时环境指针; NULL, 失败
 */
MdsAsyncApiContextT *
        MdsAsyncApi_CreateContext2(
                const char *pCfgFile,
                const char *pLoggerSection,
                const char *pAsyncApiSection,
                const char *pCpusetSection);

/**
 * 创建异步API的运行时环境 (仅通过函数参数指定必要的配置参数)
 *
 * @param   pLoggerCfgFile      日志配置文件路径
 *                              - 为空则忽略, 不初始化日志记录器
 * @param   pLoggerSection      日志记录器的配置区段名称 (e.g. "log")
 *                              - 为空则使用默认值
 * @param   asyncQueueSize      用于缓存行情数据的消息队列大小 (最大可缓存的消息数量)
 *                              - 等于0, 不创建缓存队列 (无需异步处理或落地行情数据)
 *                              - 小于0, 使用默认值 500000
 * @return  非空, 异步API的运行时环境指针; NULL, 失败
 */
MdsAsyncApiContextT *
        MdsAsyncApi_CreateContextSimple(
                const char *pLoggerCfgFile,
                const char *pLoggerSection,
                int32 asyncQueueSize);

/**
 * 创建异步API的运行时环境 (仅通过函数参数指定必要的配置参数)
 *
 * @param   pLoggerCfgFile      日志配置文件路径
 *                              - 为空则忽略, 不初始化日志记录器
 * @param   pLoggerSection      日志记录器的配置区段名称 (e.g. "log")
 *                              - 为空则使用默认值
 * @param   pContextParams      上下文环境的创建参数
 *                              - 为空则使用默认值
 * @return  非空, 异步API的运行时环境指针; NULL, 失败
 */
MdsAsyncApiContextT *
        MdsAsyncApi_CreateContextSimple2(
                const char *pLoggerCfgFile,
                const char *pLoggerSection,
                const MdsAsyncApiContextParamsT *pContextParams);

/**
 * 释放异步API的运行时环境
 *
 * @param   pAsyncContext       异步API的运行时环境指针
 */
void    MdsAsyncApi_ReleaseContext(
                MdsAsyncApiContextT *pAsyncContext);

/**
 * 启动异步API线程
 *
 * @param   pAsyncContext       异步API的运行时环境指针
 * @return  TRUE 成功; FALSE 失败
 */
BOOL    MdsAsyncApi_Start(
                MdsAsyncApiContextT *pAsyncContext);

/**
 * 终止异步API线程
 *
 * @param   pAsyncContext       异步API的运行时环境指针
 */
void    MdsAsyncApi_Stop(
                MdsAsyncApiContextT *pAsyncContext);

/**
 * 返回异步API的通信线程是否正在运行过程中
 *
 * @param   pAsyncContext       异步API的运行时环境指针
 * @return  TRUE 正在运行过程中; FALSE 已终止或尚未运行
 */
BOOL    MdsAsyncApi_IsRunning(
                const MdsAsyncApiContextT *pAsyncContext);

/**
 * 返回异步API相关的所有线程是否都已经安全退出 (或尚未运行)
 *
 * @param   pAsyncContext       异步API的运行时环境指针
 * @return  TRUE 所有线程均已退出; FALSE 尚未全部退出
 */
BOOL    MdsAsyncApi_IsAllTerminated(
                const MdsAsyncApiContextT *pAsyncContext);

/**
 * 返回异步API累计已提取和处理过的行情消息数量
 *
 * @param   pAsyncContext       异步API的运行时环境指针
 * @return  累计已提取和处理过的行情消息数量 (包括已处理但处理失败的消息)
 */
int64   MdsAsyncApi_GetTotalPicked(
                const MdsAsyncApiContextT *pAsyncContext);

/**
 * 返回异步I/O线程累计已提取和处理过的消息数量
 *
 * @param   pAsyncContext       异步API的运行时环境指针
 * @return  异步I/O线程累计已提取和处理过的消息数量
 */
int64   MdsAsyncApi_GetTotalIoPicked(
                const MdsAsyncApiContextT *pAsyncContext);

/**
 * 返回异步API累计已入队的消息数量
 *
 * @param   pAsyncContext       异步API的运行时环境指针
 * @return  累计已入队的消息数量
 */
int64   MdsAsyncApi_GetAsyncQueueTotalCount(
                const MdsAsyncApiContextT *pAsyncContext);

/**
 * 返回队列中尚未被处理的剩余数据数量
 *
 * @param   pAsyncContext       异步API的运行时环境指针
 * @return  队列中尚未被处理的剩余数据数量
 * @note    仅适用于已启用回调线程 (isAsyncCallbackAble=yes) 的运行模式
 */
int64   MdsAsyncApi_GetAsyncQueueRemainingCount(
                const MdsAsyncApiContextT *pAsyncContext);
/* -------------------------           */


/* ===================================================================
 * 通道管理接口
 * =================================================================== */

/**
 * 返回通道数量 (通道配置信息数量)
 *
 * @param   pAsyncContext       异步API的运行时环境指针
 * @return  通道数量 (通道配置信息数量)
 */
int32   MdsAsyncApi_GetChannelCount(
                const MdsAsyncApiContextT *pAsyncContext);

/**
 * 返回当前已连接的通道数量
 *
 * @param   pAsyncContext       异步API的运行时环境指针
 * @return  已连接的通道数量
 */
int32   MdsAsyncApi_GetConnectedChannelCount(
                const MdsAsyncApiContextT *pAsyncContext);

/**
 * 添加通道配置信息
 *
 * @note 提示:
 * - 不能将TCP通道和UDP通道添加到同一个异步API实例中
 * - 异步API内置了对查询接口的支持, 不能直接将查询通道添加到异步API实例中
 *
 * @param[out]  pAsyncContext   异步API的运行时环境指针
 * @param       pChannelTag     通道配置信息的自定义标签 (可以为空)
 * @param       pRemoteCfg      待添加的通道配置信息 (不可为空)
 * @param       pSubscribeCfg   默认的行情订阅参数 (可以为空)
 * @param       fnOnMsg         进行消息处理的回调函数 (不可为空)
 * @param       pOnMsgParams    fnOnMsg回调函数的参数
 * @param       fnOnConnect     连接或重新连接完成后的回调函数 (可以为空)
 *                              - 若函数指针为空, 则会使用默认的订阅参数进行订阅
 * @param       pOnConnectParams
 *                              fnOnConnect回调函数的参数
 * @param       fnOnDisconnect  连接断开后的回调函数 (可以为空)
 * @param       pOnDisconnectParams
 *                              fnOnDisconnect回调函数的参数
 * @return      非空, 异步API的连接通道信息; 空, 失败
 *
 * @note        对于组播通道, 为了支持组播身份认证, 可以通过将 pChannelTag
 *              或 remoteCfg.targetCompId 设置为如下取值, 来指定组播频道类型:
 *              - MDSAPI_CFG_DEFAULT_KEY_UDP_ADDR_SNAP1 (udpServer.Snap1)
 *              - MDSAPI_CFG_DEFAULT_KEY_UDP_ADDR_SNAP2 (udpServer.Snap2)
 *              - MDSAPI_CFG_DEFAULT_KEY_UDP_ADDR_TICK1 (udpServer.Tick1)
 *              - MDSAPI_CFG_DEFAULT_KEY_UDP_ADDR_TICK2 (udpServer.Tick2)
 */
MdsAsyncApiChannelT *
        MdsAsyncApi_AddChannel(
                MdsAsyncApiContextT *pAsyncContext,
                const char *pChannelTag,
                const MdsApiRemoteCfgT *pRemoteCfg,
                const MdsApiSubscribeInfoT *pSubscribeCfg,
                F_MDSAPI_ASYNC_ON_MSG_T fnOnMsg,
                void *pOnMsgParams,
                F_MDSAPI_ASYNC_ON_CONNECT_T fnOnConnect,
                void *pOnConnectParams,
                F_MDSAPI_ASYNC_ON_DISCONNECT_T fnOnDisconnect,
                void *pOnDisconnectParams);

/**
 * 从配置文件中加载并添加通道配置信息
 *
 * @note 提示:
 * - 不能将TCP通道和UDP通道添加到同一个异步API实例中
 * - 异步API内置了对查询接口的支持, 不能直接将查询通道添加到异步API实例中
 *
 * @param[out]  pAsyncContext   异步API的运行时环境指针
 * @param       pChannelTag     通道配置信息的自定义标签 (可以为空)
 * @param       pCfgFile        配置文件路径 (不可为空)
 * @param       pCfgSection     配置区段名称 (不可为空, e.g. "mds_client")
 * @param       pAddrKey        服务器地址的配置项关键字 (不可为空)
 * @param       fnOnMsg         进行消息处理的回调函数 (不可为空)
 * @param       pOnMsgParams    fnOnMsg回调函数的参数
 * @param       fnOnConnect     连接或重新连接完成后的回调函数 (可以为空)
 *                              - 若函数指针为空, 则会使用默认的订阅参数进行订阅
 * @param       pOnConnectParams
 *                              fnOnConnect回调函数的参数
 * @param       fnOnDisconnect  连接断开后的回调函数 (可以为空)
 * @param       pOnDisconnectParams
 *                              fnOnDisconnect回调函数的参数
 * @return      非空, 异步API的连接通道信息; 空, 失败
 *
 * @note        对于组播通道, 为了支持组播身份认证, 可以通过将 pChannelTag
 *              或配置项关键字(pAddrKey)设置为如下取值, 来指定组播频道类型:
 *              - MDSAPI_CFG_DEFAULT_KEY_UDP_ADDR_SNAP1 (udpServer.Snap1)
 *              - MDSAPI_CFG_DEFAULT_KEY_UDP_ADDR_SNAP2 (udpServer.Snap2)
 *              - MDSAPI_CFG_DEFAULT_KEY_UDP_ADDR_TICK1 (udpServer.Tick1)
 *              - MDSAPI_CFG_DEFAULT_KEY_UDP_ADDR_TICK2 (udpServer.Tick2)
 */
MdsAsyncApiChannelT *
        MdsAsyncApi_AddChannelFromFile(
                MdsAsyncApiContextT *pAsyncContext,
                const char *pChannelTag,
                const char *pCfgFile,
                const char *pCfgSection,
                const char *pAddrKey,
                F_MDSAPI_ASYNC_ON_MSG_T fnOnMsg,
                void *pOnMsgParams,
                F_MDSAPI_ASYNC_ON_CONNECT_T fnOnConnect,
                void *pOnConnectParams,
                F_MDSAPI_ASYNC_ON_DISCONNECT_T fnOnDisconnect,
                void *pOnDisconnectParams);

/**
 * 返回顺序号对应的连接通道信息
 *
 * @param   pAsyncContext       异步API的运行时环境指针
 * @param   channelIndex        通道顺序号
 *                              - 大于0: 返回与指定顺序号相对应的, 并且与指定通道类型相
 *                                匹配的通道信息 (顺序号与通道配置的添加顺序一致)
 *                              - 小于0: 返回第一个与指定通道类型相匹配的通道信息
 *                              - INT_MAX: 返回最后一个与指定通道类型相匹配的通道信息
 * @return  异步API的连接通道信息
 */
MdsAsyncApiChannelT *
        MdsAsyncApi_GetChannel(
                const MdsAsyncApiContextT *pAsyncContext,
                int32 channelIndex);

/**
 * 返回标签对应的连接通道信息
 *
 * @note 注意事项:
 * - API不强制要求标签必须唯一, 如果标签不唯一, 则将返回第一个匹配到的通道信息
 * - 标签名称不区分大小写
 *
 * @param   pAsyncContext       异步API的运行时环境指针
 * @param   pChannelTag         通道配置信息的自定义标签
 * @return  异步API的连接通道信息
 */
MdsAsyncApiChannelT *
        MdsAsyncApi_GetChannelByTag(
                const MdsAsyncApiContextT *pAsyncContext,
                const char *pChannelTag);

/**
 * 返回会话信息对应的异步API连接通道信息
 *
 * @param   pSessionInfo        会话信息
 * @return  异步API连接通道信息
 */
MdsAsyncApiChannelT *
        MdsAsyncApi_GetChannelBySession(
                const MdsApiSessionInfoT *pSessionInfo);

/**
 * 遍历所有的连接通道信息并执行回调函数
 *
 * @param   pAsyncContext       异步API的运行时环境指针
 * @param   fnCallback          待执行的回调函数 (可以为空)
 *                              - 若返回值小于0, 则将中止遍历并返回该值
 * @param   pParams             回调函数的参数 (可以为空)
 * @return  大于等于0, 成功遍历到的通道数量; 小于0, 参数错误或者回调函数的返回值小于0
 */
int32   MdsAsyncApi_ForeachChannel(
                const MdsAsyncApiContextT *pAsyncContext,
                int32 (*fnCallback)(
                        MdsAsyncApiChannelT *pAsyncChannel, void *pParams),
                void *pParams);

/**
 * 遍历所有的连接通道信息并执行回调函数
 *
 * @param   pAsyncContext       异步API的运行时环境指针
 * @param   fnCallback          待执行的回调函数 (可以为空)
 *                              - 若返回值小于0, 则将中止遍历并返回该值
 * @param   pParam1             回调函数的参数1 (可以为空)
 * @param   pParam2             回调函数的参数2 (可以为空)
 * @return  大于等于0, 成功遍历到的通道数量; 小于0, 参数错误或者回调函数的返回值小于0
 */
int32   MdsAsyncApi_ForeachChannel2(
                const MdsAsyncApiContextT *pAsyncContext,
                int32 (*fnCallback)(
                        MdsAsyncApiChannelT *pAsyncChannel,
                        void *pParam1, void *pParam2),
                void *pParam1,
                void *pParam2);

/**
 * 遍历所有的连接通道信息并执行回调函数
 *
 * @param   pAsyncContext       异步API的运行时环境指针
 * @param   fnCallback          待执行的回调函数 (可以为空)
 *                              - 若返回值小于0, 则将中止遍历并返回该值
 * @param   pParam1             回调函数的参数1 (可以为空)
 * @param   pParam2             回调函数的参数2 (可以为空)
 * @param   pParam3             回调函数的参数3 (可以为空)
 * @return  大于等于0, 成功遍历到的通道数量; 小于0, 参数错误或者回调函数的返回值小于0
 */
int32   MdsAsyncApi_ForeachChannel3(
                const MdsAsyncApiContextT *pAsyncContext,
                int32 (*fnCallback)(MdsAsyncApiChannelT *pAsyncChannel,
                        void *pParam1, void *pParam2, void *pParam3),
                void *pParam1,
                void *pParam2,
                void *pParam3);

/**
 * 返回通道是否已连接就绪
 *
 * @param   pAsyncChannel       异步API的连接通道信息
 * @return  TRUE 已连接就绪; FALSE 尚未连接就绪
 */
BOOL    MdsAsyncApi_IsChannelConnected(
                const MdsAsyncApiChannelT *pAsyncChannel);

/**
 * 返回通道对应的配置信息
 *
 * @param   pAsyncChannel       异步API的连接通道信息
 * @return  通道配置信息
 */
MdsAsyncApiChannelCfgT *
        MdsAsyncApi_GetChannelCfg(
                const MdsAsyncApiChannelT *pAsyncChannel);

/**
 * 返回通道对应的行情订阅配置信息
 *
 * @param   pAsyncChannel       异步API的连接通道信息
 * @return  行情订阅配置信息
 */
MdsApiSubscribeInfoT *
        MdsAsyncApi_GetChannelSubscribeCfg(
                const MdsAsyncApiChannelT *pAsyncChannel);

/**
 * 设置连接或重新连接完成后的回调函数
 *
 * @param   pAsyncChannel       异步API的连接通道信息
 * @param   fnOnConnect         连接或重新连接完成后的回调函数
 * @param   pOnConnectParams    OnConnect回调函数的参数
 * @return  TRUE 成功; FALSE 失败
 */
BOOL    MdsAsyncApi_SetOnConnect(
                MdsAsyncApiChannelT *pAsyncChannel,
                F_MDSAPI_ASYNC_ON_CONNECT_T fnOnConnect,
                void *pOnConnectParams);

/**
 * 返回连接或重新连接完成后的回调函数
 *
 * @param   pAsyncChannel       异步API的连接通道信息
 * @return  连接或重新连接完成后的回调函数指针
 */
F_MDSAPI_ASYNC_ON_CONNECT_T
        MdsAsyncApi_GetOnConnect(
                const MdsAsyncApiChannelT *pAsyncChannel);

/**
 * 设置连接断开后的回调函数
 *
 * @param   pAsyncChannel       异步API的连接通道信息
 * @param   fnOnDisconnect      连接断开后的回调函数
 * @param   pOnDisconnectParams OnDisconnect回调函数的参数
 * @return  TRUE 成功; FALSE 失败
 */
BOOL    MdsAsyncApi_SetOnDisconnect(
                MdsAsyncApiChannelT *pAsyncChannel,
                F_MDSAPI_ASYNC_ON_DISCONNECT_T fnOnDisconnect,
                void *pOnDisconnectParams);

/**
 * 返回连接断开后的回调函数
 *
 * @param   pAsyncChannel       异步API的连接通道信息
 * @return  连接断开后的回调函数指针
 */
F_MDSAPI_ASYNC_ON_DISCONNECT_T
        MdsAsyncApi_GetOnDisconnect(
                const MdsAsyncApiChannelT *pAsyncChannel);

/**
 * 设置连接失败时的回调函数
 *
 * - OnConnectFailed 和 OnDisconnect 回调函数的区别在于:
 *   - 在连接成功以前:
 *     - 当尝试建立或重建连接时, 如果连接失败则回调 OnConnectFailed;
 *     - 如果连接成功, 则回调 OnConnect;
 *   - 在连接成功以后:
 *     - 如果发生连接中断, 则回调 OnDisconnect.
 *
 * @param   pAsyncChannel       异步API的连接通道信息
 * @param   fnOnConnectFailed   连接失败时的回调函数 (可以为空)
 * @param   pOnConnectFailedParams
 *                              OnConnectFailed回调函数的参数
 * @return  TRUE 成功; FALSE 失败
 */
BOOL    MdsAsyncApi_SetOnConnectFailed(
                MdsAsyncApiChannelT *pAsyncChannel,
                F_MDSAPI_ASYNC_ON_DISCONNECT_T fnOnConnectFailed,
                void *pOnConnectFailedParams);

/**
 * 返回连接失败时的回调函数
 *
 * @param   pAsyncChannel       异步API的连接通道信息
 * @return  连接失败时的回调函数指针
 */
F_MDSAPI_ASYNC_ON_DISCONNECT_T
        MdsAsyncApi_GetOnConnectFailed(
                const MdsAsyncApiChannelT *pAsyncChannel);
/* -------------------------           */


/* ===================================================================
 * 会话管理接口
 * =================================================================== */

/**
 * 以异步的方式发送证券行情实时订阅请求, 以重新订阅、追加订阅或删除订阅行情数据
 *
 * @note    频繁订阅会对当前连接的行情获取速度产生不利影响, 建议每次订阅都尽量指定更多的证券
 *          代码, 以减少订阅次数
 *
 * @param   pAsyncChannel       异步API的连接通道信息
 * @param   pMktDataRequestReq  行情订阅请求信息
 * @param   pEntries            产品订阅列表
 * @return  TRUE 成功; FALSE 失败
 */
BOOL    MdsAsyncApi_SubscribeMarketData(
                MdsAsyncApiChannelT *pAsyncChannel,
                const MdsMktDataRequestReqT *pMktDataRequestReq,
                const MdsMktDataRequestEntryT *pEntries);

/**
 * 根据字符串形式的证券代码列表订阅行情信息
 *
 * @note    与 MdsApi_SubscribeByString 接口相同, 无法直接指定 tickType 等订阅参数(均
 *          默认为0), 需要通过 MdsApi_SetThreadSubscribeTickType 等接口来这些订阅参数
 * @note    频繁订阅会对当前连接的行情获取速度产生不利影响, 建议每次订阅都尽量指定更多的证券
 *          代码, 以减少订阅次数
 * @note    订阅的数据类型 (dataTypes) 会以最后一次订阅为准, 所以每次都应该指定为所有待订阅
 *          的数据类型
 *
 * @param   pAsyncChannel       异步API的连接通道信息
 * @param   pSecurityListStr    证券代码列表字符串
 *                              - 证券代码支持以 .SH 或 .SZ 为后缀来指定其所属的交易所
 *                              - 空字符串 "", 表示不订阅任何产品的行情
 *                              - 空指针 NULL, 表示订阅所有产品的行情
 * @param   pDelim              证券代码列表的分隔符 (e.g. ",;| \t")
 *                              - 如果为空, 则使用默认的分隔符:
 *                                ',' 或 ';' 或 '|' 或 ' ' 或 '\t'
 * @param   exchangeId          证券代码所属的交易所代码 (如果证券代码没有 .SH 或 .SZ 后缀的话)
 * @param   mdProductType       行情产品类型 (股票(基金、债券)/指数/期权)
 * @param   subMode             订阅模式 (重新订阅/追加订阅/删除订阅)
 * @param   dataTypes           订阅的数据种类 @see eMdsSubscribeDataTypeT
 *                              (e.g. MDS_SUB_DATA_TYPE_L1_SNAPSHOT
 *                              | MDS_SUB_DATA_TYPE_L2_SNAPSHOT
 *                              | MDS_SUB_DATA_TYPE_L2_TRADE)
 *                              - 当订阅模式为追加订阅时, 如果该参数小于0, 将忽略该参数, 维持上一次订阅时的设置
 *                              - 当订阅模式为删除订阅时, 该参数没有意义, 将会被忽略
 * @return  TRUE 成功; FALSE 失败
 *
 * @see     MdsApi_SetThreadSubscribeTickType
 * @see     MdsApi_SetThreadSubscribeTickRebuildFlag
 * @see     MdsApi_SetThreadSubscribeRequireInitMd
 * @see     MdsApi_SetThreadSubscribeBeginTime
 */
BOOL    MdsAsyncApi_SubscribeByString(
                MdsAsyncApiChannelT *pAsyncChannel,
                const char *pSecurityListStr, const char *pDelim,
                eMdsExchangeIdT exchangeId, eMdsMdProductTypeT mdProductType,
                eMdsSubscribeModeT subMode, int32 dataTypes);

/**
 * 直接根据字符串形式的证券代码列表订阅行情, 并通过证券代码前缀来区分和识别所属市场
 *
 * @note    代码前缀仅对 pSecurityListStr 参数指定的证券代码生效, 只是为了方便区分证券代码
 *          所属的市场, 并不能直接通过代码前缀自动订阅所有匹配的股票
 * @note    与 MdsApi_SubscribeByString 接口相同, 无法直接指定 tickType 等订阅参数(均
 *          默认为0), 需要通过 MdsApi_SetThreadSubscribeTickType 等接口来这些订阅参数
 * @note    频繁订阅会对当前连接的行情获取速度产生不利影响, 建议每次订阅都尽量指定更多的证券
 *          代码, 以减少订阅次数
 * @note    订阅的数据类型 (dataTypes) 会以最后一次订阅为准, 所以每次都应该指定为所有待订阅
 *          的数据类型
 *
 * @param   pAsyncChannel       异步API的连接通道信息
 * @param   pSecurityListStr    证券代码列表字符串
 *                              - 证券代码支持以 .SH 或 .SZ 为后缀来指定其所属的交易所
 *                              - 空字符串 "", 表示不订阅任何产品的行情
 *                              - 空指针 NULL, 表示订阅所有产品的行情
 * @param   pDelim              证券代码列表的分隔符 (e.g. ",;| \t")
 *                              - 如果为空, 则使用默认的分隔符:
 *                                ',' 或 ';' 或 '|' 或 ' ' 或 '\t'
 * @param   pSseCodePrefixes    以逗号或空格分隔的上海证券代码前缀列表, e.g.
 *                              - "6, 300, 301" 将匹配证券代码列表中所有以 '6' 或 '300'
 *                                或 '301' 起始的证券代码
 *                              - 若为NULL或空字符串, 则不会匹配任何证券代码
 *                              - 上海证券代码前缀参考:
 *                                  - "009, 01, 02, "               //国债
 *                                  - "10, 11, 12, 13, 18, 19, "    //债券 (企业债、可转债等)
 *                                  - "20, "                        //债券 (回购)
 *                                  - "5, "                         //基金
 *                                  - "6, "                         //A股
 *                                  - "000"                         //指数
 * @param   pSzseCodePrefixes   以逗号或空格分隔的深圳证券代码前缀列表
 *                              - 若为NULL或空字符串, 则不会匹配任何证券代码
 *                              - 证券代码前缀可以和上海相同, 此时匹配的证券代码会同时对上海
 *                                和深圳两个市场进行订阅
 *                              - 深圳证券代码前缀参考:
 *                                  - "00, "                        //股票
 *                                  - "10, 11, 12, 13, "            //债券
 *                                  - "15, 16, 17, 18, "            //基金
 *                                  - "30, "                        //创业板
 *                                  - "39"                          //指数
 * @param   mdProductType       行情产品类型 (股票(基金、债券)/指数/期权)
 * @param   subMode             订阅模式 (重新订阅/追加订阅/删除订阅)
 * @param   dataTypes           订阅的数据种类 @see eMdsSubscribeDataTypeT
 *                              (e.g. MDS_SUB_DATA_TYPE_L1_SNAPSHOT
 *                              | MDS_SUB_DATA_TYPE_L2_SNAPSHOT
 *                              | MDS_SUB_DATA_TYPE_L2_TRADE)
 *                              - 当订阅模式为追加订阅时, 如果该参数小于0, 将忽略该参数, 维持上一次订阅时的设置
 *                              - 当订阅模式为删除订阅时, 该参数没有意义, 将会被忽略
 * @return  TRUE 成功; FALSE 失败
 *
 * @see     MdsApi_SetThreadSubscribeTickType
 * @see     MdsApi_SetThreadSubscribeTickRebuildFlag
 * @see     MdsApi_SetThreadSubscribeRequireInitMd
 * @see     MdsApi_SetThreadSubscribeBeginTime
 */
BOOL    MdsAsyncApi_SubscribeByStringAndPrefixes(
                MdsAsyncApiChannelT *pAsyncChannel,
                const char *pSecurityListStr,
                const char *pDelim,
                const char *pSseCodePrefixes,
                const char *pSzseCodePrefixes,
                eMdsMdProductTypeT mdProductType,
                eMdsSubscribeModeT subMode,
                int32 dataTypes);

/**
 * 查询证券静态信息, 并根据查询结果订阅行情信息
 *
 * @note    频繁订阅会对当前连接的行情获取速度产生不利影响, 建议每次订阅都尽量指定更多的证券
 *          代码, 以减少订阅次数
 *
 * @param   pAsyncChannel       异步API的连接通道信息
 * @param   subMode             订阅模式 @see eMdsSubscribeModeT
 * @param   dataTypes           订阅的数据种类 @see eMdsSubscribeDataTypeT
 *                              (e.g. MDS_SUB_DATA_TYPE_L1_SNAPSHOT
 *                              | MDS_SUB_DATA_TYPE_L2_SNAPSHOT
 *                              | MDS_SUB_DATA_TYPE_L2_TRADE)
 *                              - 当订阅模式为追加订阅时, 如果该参数小于0, 将忽略该参数, 维持上一次订阅时的设置
 *                              - 当订阅模式为删除订阅时, 该参数没有意义, 将会被忽略
 * @param   pQryChannel         查询通道的会话信息
 *                              - 允许为空, 为空则使用异步API内置的查询通道
 * @param   pQryFilter          查询过滤条件
 *                              - 传空指针或者将过滤条件初始化为0, 代表无需过滤
 * @retval  >=0                 成功查询到的记录数
 * @retval  <0                  失败 (负的错误号)
 */
int32   MdsAsyncApi_SubscribeByQuery(MdsAsyncApiChannelT *pAsyncChannel,
                eMdsSubscribeModeT subMode, int32 dataTypes,
                MdsApiSessionInfoT *pQryChannel,
                const MdsQryStockStaticInfoListFilterT *pQryFilter);

/**
 * 发送心跳消息
 *
 * @param   pAsyncChannel       异步API的连接通道信息
 * @return  TRUE 成功; FALSE 失败
 */
BOOL    MdsAsyncApi_SendHeartbeat(
                MdsAsyncApiChannelT *pAsyncChannel);

/**
 * 发送测试请求消息
 *
 * @param   pAsyncChannel       异步API的连接通道信息
 * @param   pTestReqId          测试请求标识符 (C32, 可以为空)
 * @param   testReqIdSize       测试请求标识符的长度
 * @return  TRUE 成功; FALSE 失败
 */
BOOL    MdsAsyncApi_SendTestReq(
                MdsAsyncApiChannelT *pAsyncChannel,
                const char *pTestReqId,
                int32 testReqIdSize);

/**
 * 连接完成后处理的默认实现 (执行默认的行情订阅处理)
 *
 * <p> 提示:
 * - 可以在 OnConnect 回调函数中调用该接口来完成默认的行情订阅处理
 * - 也可以直接使用该接口作为 OnConnect 回调函数 (与不设置 OnConnect 回调函数的效果相同)
 * </p>
 *
 * @param   pAsyncChannel       异步API的连接通道信息
 * @param   pCallbackParams     外部传入的参数 (不会使用该参数, 传任意值或 NULL 均可)
 * @retval  =0                  等于0, 成功
 * @retval  >0                  大于0, 处理失败, 将重建连接并继续尝试执行
 * @retval  <0                  小于0, 处理失败, 异步API将中止运行
 */
int32   MdsAsyncApi_DefaultOnConnect(
                MdsAsyncApiChannelT *pAsyncChannel,
                void *pCallbackParams);

/**
 * 连接完成后处理的默认实现 (不订阅任何行情数据)
 *
 * <p> 提示:
 * - 可以在 OnConnect 回调函数中调用该接口来实现不订阅任何行情数据的功能
 * - 也可以直接使用该接口作为 OnConnect 回调函数
 * </p>
 *
 * @param   pAsyncChannel       异步API的连接通道信息
 * @param   pCallbackParams     外部传入的参数 (不会使用该参数, 传任意值或 NULL 均可)
 * @retval  =0                  等于0, 成功
 * @retval  >0                  大于0, 处理失败, 将重建连接并继续尝试执行
 * @retval  <0                  小于0, 处理失败, 异步API将中止运行
 */
int32   MdsAsyncApi_SubscribeNothingOnConnect(
                MdsAsyncApiChannelT *pAsyncChannel,
                void *pCallbackParams);
/* -------------------------           */


/* ===================================================================
 * 辅助的配置管理接口
 * =================================================================== */

/**
 * 从配置文件中加载异步API运行参数
 *
 * @param       pCfgFile        配置文件路径
 * @param       pCfgSection     配置区段名称 (e.g. "mds_client.async_api")
 * @param[out]  pOutContextParams
 *                              异步API的运行参数
 * @return      TRUE 成功; FALSE 失败
 */
BOOL    MdsAsyncApi_LoadContextParams(
                const char *pCfgFile,
                const char *pCfgSection,
                MdsAsyncApiContextParamsT *pOutContextParams);

/**
 * 从配置文件中加载CPU亲和性配置
 *
 * @param[out]  pAsyncContext   异步API的运行时环境指针
 * @param       pCfgFile        配置文件路径
 * @param       pCfgSection     配置区段名称 (e.g. "cpuset")
 * @param       pCommunicationCpusetKey
 *                              通信线程的CPU亲和性配置项关键字 (为空则忽略)
 * @param       pCallbackThreadCpusetKey
 *                              异步回调线程的CPU亲和性配置项关键字 (为空则忽略)
 * @param       pIoThreadCpusetKey
 *                              异步I/O线程的CPU亲和性配置项关键字 (为空则忽略)
 * @return      TRUE 成功; FALSE 失败
 */
BOOL    MdsAsyncApi_LoadCpusetCfg(
                MdsAsyncApiContextT *pAsyncContext,
                const char *pCfgFile,
                const char *pCfgSection,
                const char *pCommunicationCpusetKey,
                const char *pCallbackThreadCpusetKey,
                const char *pIoThreadCpusetKey);

/**
 * 从配置文件中加载CPU亲和性配置 (额外增加对连接管理线程的支持)
 *
 * @param[out]  pAsyncContext   异步API的运行时环境指针
 * @param       pCfgFile        配置文件路径
 * @param       pCfgSection     配置区段名称 (e.g. "cpuset")
 * @param       pCommunicationCpusetKey
 *                              通信线程的CPU亲和性配置项关键字 (为空则忽略)
 * @param       pCallbackThreadCpusetKey
 *                              异步回调线程的CPU亲和性配置项关键字 (为空则忽略)
 * @param       pConnectThreadCpusetKey
 *                              连接管理线程的CPU亲和性配置项关键字 (为空则忽略)
 * @param       pIoThreadCpusetKey
 *                              异步I/O线程的CPU亲和性配置项关键字 (为空则忽略)
 * @return      TRUE 成功; FALSE 失败
 */
BOOL    MdsAsyncApi_LoadCpusetCfg2(
                MdsAsyncApiContextT *pAsyncContext,
                const char *pCfgFile,
                const char *pCfgSection,
                const char *pCommunicationCpusetKey,
                const char *pCallbackThreadCpusetKey,
                const char *pConnectThreadCpusetKey,
                const char *pIoThreadCpusetKey);

/**
 * 设置通信线程的CPU亲和性配置
 *
 * @param[out]  pAsyncContext   异步API的运行时环境指针
 * @param       pCommunicationCpuset
 *                              通信线程绑定的CPU列表 (以逗号或空格分割的CPU编号列表, CPU编号从1开始)
 *                              - e.g. "1"      绑定到 0 号核
 *                              - e.g. "1,3,5"  绑定到 0,2,4 号核
 *                              - NULL 或者 "", 不绑定CPU
 * @return      TRUE 成功; FALSE 失败
 */
BOOL    MdsAsyncApi_SetCommunicationCpusetCfg(
                MdsAsyncApiContextT *pAsyncContext,
                const char *pCommunicationCpuset);

/**
 * 返回通信线程的CPU亲和性配置信息
 *
 * @param   pAsyncContext       异步API的运行时环境指针
 * @return  CPU亲和性配置信息
 */
const char *
        MdsAsyncApi_GetCommunicationCpusetCfg(
                const MdsAsyncApiContextT *pAsyncContext);

/**
 * 设置异步回调线程的CPU亲和性配置
 *
 * @param[out]  pAsyncContext   异步API的运行时环境指针
 * @param       pCallbackThreadCpuset
 *                              异步回调线程绑定的CPU列表 (以逗号或空格分割的CPU编号列表, CPU编号从1开始)
 *                              - e.g. "1"      绑定到 0 号核
 *                              - e.g. "1,3,5"  绑定到 0,2,4 号核
 *                              - NULL 或者 "", 不绑定CPU
 * @return      TRUE 成功; FALSE 失败
 */
BOOL    MdsAsyncApi_SetCallbackThreadCpusetCfg(
                MdsAsyncApiContextT *pAsyncContext,
                const char *pCallbackThreadCpuset);

/**
 * 返回异步回调线程的CPU亲和性配置信息
 *
 * @param   pAsyncContext       异步API的运行时环境指针
 * @return  CPU亲和性配置信息
 */
const char *
        MdsAsyncApi_GetCallbackThreadCpusetCfg(
                const MdsAsyncApiContextT *pAsyncContext);

/**
 * 设置连接管理线程的CPU亲和性配置
 *
 * @param[out]  pAsyncContext   异步API的运行时环境指针
 * @param       pConnectThreadCpuset
 *                              连接管理线程绑定的CPU列表 (以逗号或空格分割的CPU编号列表, CPU编号从1开始)
 *                              - e.g. "1"      绑定到 0 号核
 *                              - e.g. "1,3,5"  绑定到 0,2,4 号核
 *                              - NULL 或者 "", 不绑定CPU
 * @return      TRUE 成功; FALSE 失败
 */
BOOL    MdsAsyncApi_SetConnectThreadCpusetCfg(
                MdsAsyncApiContextT *pAsyncContext,
                const char *pConnectThreadCpuset);

/**
 * 返回连接管理线程的CPU亲和性配置信息
 *
 * @param   pAsyncContext       异步API的运行时环境指针
 * @return  CPU亲和性配置信息
 */
const char *
        MdsAsyncApi_GetConnectThreadCpusetCfg(
                const MdsAsyncApiContextT *pAsyncContext);

/**
 * 设置异步I/O线程的CPU亲和性配置
 *
 * @param[out]  pAsyncContext   异步API的运行时环境指针
 * @param       pIoThreadCpuset 异步I/O线程绑定的CPU列表 (以逗号或空格分割的CPU编号列表, CPU编号从1开始)
 *                              - e.g. "1"      绑定到 0 号核
 *                              - e.g. "1,3,5"  绑定到 0,2,4 号核
 *                              - NULL 或者 "", 不绑定CPU
 * @return      TRUE 成功; FALSE 失败
 */
BOOL    MdsAsyncApi_SetIoThreadCpusetCfg(
                MdsAsyncApiContextT *pAsyncContext,
                const char *pIoThreadCpuset);

/**
 * 返回异步I/O线程的CPU亲和性配置信息
 *
 * @param   pAsyncContext       异步API的运行时环境指针
 * @return  CPU亲和性配置信息
 */
const char *
        MdsAsyncApi_GetIoThreadCpusetCfg(
                const MdsAsyncApiContextT *pAsyncContext);

/**
 * 设置是否在启动前预创建并校验所有的连接
 *
 * @param   pAsyncContext       异步API的运行时环境指针
 * @param   isPreconnectAble    是否在启动前预创建并校验所有的连接
 *                              TRUE: 启动前预创建并校验所有的连接, 如果连接失败则中止启动
 *                              FALSE: 启动前不预先创建和校验连接 (默认行为)
 * @return  TRUE 成功; FALSE 失败
 */
BOOL    MdsAsyncApi_SetPreconnectAble(
                MdsAsyncApiContextT *pAsyncContext,
                BOOL isPreconnectAble);

/**
 * 返回是否在启动前预创建并校验所有的连接
 *
 * @param   pAsyncContext       异步API的运行时环境指针
 * @return  是否在启动前预创建并校验所有的连接
 */
BOOL    MdsAsyncApi_IsPreconnectAble(
                const MdsAsyncApiContextT *pAsyncContext);

/**
 * 设置是否需要支持对接压缩后的行情数据
 *
 * @param[out]  pAsyncContext   异步API的运行时环境指针
 * @param       isCompressible  是否需要支持对接压缩后的行情数据
 *                              - 如果可能会对接压缩行情端口, 可以将该参数设置为TRUE, 这
 *                                样就可以同时兼容压缩和非压缩的行情数据
 *                              - 如果确定不会对接压缩行情端口的话, 则可以将该参数设置为
 *                                FALSE, 这样可以避免额外的(微小的)性能消耗
 * @return      TRUE 成功; FALSE 失败
 */
BOOL    MdsAsyncApi_SetCompressible(
                MdsAsyncApiContextT *pAsyncContext,
                BOOL isCompressible);

/**
 * 返回是否支持对接压缩后的行情数据
 *
 * @param   pAsyncContext       异步API的运行时环境指针
 * @return  是否支持对接压缩后的行情数据
 */
BOOL    MdsAsyncApi_IsCompressible(
                const MdsAsyncApiContextT *pAsyncContext);

/**
 * 设置是否启用对UDP行情数据的本地行情订阅和过滤功能
 *
 * @param[out]  pAsyncContext   异步API的运行时环境指针
 * @param       isUdpFilterable 是否启用对UDP行情数据的本地订阅和过滤功能
 *                              - 如果将该参数设置为TRUE, 则允许通过
 *                                MdsAsyncApi_SubscribeByString 等接口设置 UDP 行
 *                                情的订阅条件, 并在API端完成对行情数据的过滤
 *                              - 如果不需要通过API进行行情数据过滤的话, 可以将该参数设
 *                                置为FALSE, 这样可以避免额外的(微小的)性能消耗
 * @return      TRUE 成功; FALSE 失败
 */
BOOL    MdsAsyncApi_SetUdpFilterable(
                MdsAsyncApiContextT *pAsyncContext,
                BOOL isUdpFilterable);

/**
 * 返回是否启用对UDP行情数据的本地行情订阅和过滤功能
 *
 * @param   pAsyncContext       异步API的运行时环境指针
 * @return  是否启用对UDP行情数据的本地行情订阅和过滤功能
 */
BOOL    MdsAsyncApi_IsUdpFilterable(
                const MdsAsyncApiContextT *pAsyncContext);

/**
 * 设置是否接管启动线程
 *
 * @param   pAsyncContext       异步API的运行时环境指针
 * @param   isTakeoverStartThread
 *                              设置是否接管启动线程
 *                              - 0: 启动时将创建独立的线程执行通信处理 (默认行为)
 *                              - 1: 无需创建独立的通信线程, 启动时将接管当前线程并直接
 *                                在当前线程下执行通信处理
 *                              - SCHAR_MAX: 不创建通信线程, 也不执行除OnConnect回调以
 *                                外的任何消息接收等异步处理, 完全由当前线程控制后续的执
 *                                行 (仅适用于测试场景, 正常情况下不会使用)
 * @return  TRUE 成功; FALSE 失败
 */
BOOL    MdsAsyncApi_SetTakeoverStartThreadFlag(
                MdsAsyncApiContextT *pAsyncContext,
                int32 isTakeoverStartThread);

/**
 * 返回是否接管启动线程
 *
 * @param   pAsyncContext       异步API的运行时环境指针
 * @return  是否接管启动线程
 */
int32   MdsAsyncApi_GetTakeoverStartThreadFlag(
                const MdsAsyncApiContextT *pAsyncContext);

/**
 * 设置是否启动独立的回调线程来执行回调处理
 *
 * @param   pAsyncContext       异步API的运行时环境指针
 * @param   isAsyncCallbackAble 是否启动独立的回调线程来执行回调处理
 *                              - TRUE: 创建单独的回调线程 (默认行为)
 *                              - FALSE: 不启动单独的回调线程, 直接在通信线程下执行回调处理
 * @return  TRUE 成功; FALSE 失败
 */
BOOL    MdsAsyncApi_SetAsyncCallbackAble(
                MdsAsyncApiContextT *pAsyncContext,
                BOOL isAsyncCallbackAble);

/**
 * 返回是否启动独立的回调线程来执行回调处理
 *
 * @param   pAsyncContext       异步API的运行时环境指针
 * @return  是否启动独立的回调线程来执行回调处理
 */
BOOL    MdsAsyncApi_IsAsyncCallbackAble(
                const MdsAsyncApiContextT *pAsyncContext);

/**
 * 设置是否启动独立的连接管理线程来执行连接处理和OnConnect回调处理
 *
 * @param   pAsyncContext       异步API的运行时环境指针
 * @param   isAsyncConnectAble  是否启动独立的连接管理线程来执行连接处理和OnConnect回调处理
 *                              - TRUE: 创建单独的连接管理线程
 *                              - FALSE: 不启动单独的连接管理线程, 直接在通信线程下执行回调处理
 * @return  TRUE 成功; FALSE 失败
 */
BOOL    MdsAsyncApi_SetAsyncConnectAble(
                MdsAsyncApiContextT *pAsyncContext,
                BOOL isAsyncConnectAble);

/**
 * 返回是否启动独立的连接管理线程来执行连接处理和OnConnect回调处理
 *
 * @param   pAsyncContext       异步API的运行时环境指针
 * @return  是否在启动前预创建并校验所有的连接
 */
BOOL    MdsAsyncApi_IsAsyncConnectAble(
                const MdsAsyncApiContextT *pAsyncContext);

/**
 * 设置 Onload 加速标志
 *
 * @param   pAsyncContext       异步API的运行时环境指针
 * @param   onloadFlag          Onload 加速标志
 *                              - 0: 未启用 onload 加速
 *                              - 1: 已启用 onload 加速
 *                              - 2: 自动管理 onload 栈
 * @return  TRUE 成功; FALSE 失败
 */
BOOL    MdsAsyncApi_SetOnloadFlag(
                MdsAsyncApiContextT *pAsyncContext,
                uint8 onloadFlag);

/**
 * 返回 Onload 加速标志
 *
 * @param   pAsyncContext       异步API的运行时环境指针
 * @return  Onload 加速标志
 */
uint8   MdsAsyncApi_GetOnloadFlag(
                const MdsAsyncApiContextT *pAsyncContext);

/**
 * 设置异步回调线程的忙等待模式
 *
 * @param[out]  pAsyncContext   异步API的运行时环境指针
 * @param       isBusyPollAble  是否使用忙等待模式
 *                              TRUE: 使用忙等待模式, 延迟更低, 但CPU会被100%占用
 *                              FALSE: 不使用忙等待模式, 延迟和CPU使用率相对均衡
 * @return      TRUE 成功; FALSE 失败
 */
BOOL    MdsAsyncApi_SetAsyncCallbackBusyPollAble(
                MdsAsyncApiContextT *pAsyncContext,
                BOOL isBusyPollAble);

/**
 * 返回异步回调线程的忙等待模式
 *
 * @param   pAsyncContext       异步API的运行时环境指针
 * @return  是否使用忙等待模式
 */
BOOL    MdsAsyncApi_IsAsyncCallbackBusyPollAble(
                const MdsAsyncApiContextT *pAsyncContext);

/**
 * 返回异步通信队列的长度 (可缓存的最大消息数量)
 *
 * @param   pAsyncContext       异步API的运行时环境指针
 * @return  异步通信队列的长度 (可缓存的最大消息数量)
 */
int64   MdsAsyncApi_GetAsyncQueueLength(
                const MdsAsyncApiContextT *pAsyncContext);

/**
 * 返回异步通信队列的数据空间大小
 *
 * @param   pAsyncContext       异步API的运行时环境指针
 * @return  异步通信队列的数据空间大小 (字节数)
 */
int64   MdsAsyncApi_GetAsyncQueueDataAreaSize(
                const MdsAsyncApiContextT *pAsyncContext);

/**
 * 设置是否启用内置的查询通道
 *
 * @param[out]  pAsyncContext   异步API的运行时环境指针
 * @param       isBuiltinQueryable
 *                              是否启用内置的查询通道
 *                              - 如果将该参数设置为TRUE, 则启动异步API时将自动创建一个
 *                                与行情查询服务的连接
 *                              - 如果不需要通过异步API查询行情数据的话, 可以将该参数设
 *                                置为FALSE, 这样可以避免额外占用一个查询通道的连接数量
 *                              - 不指定的话, 默认为FALSE
 * @return      TRUE 成功; FALSE 失败
 */
BOOL    MdsAsyncApi_SetBuiltinQueryable(
                MdsAsyncApiContextT *pAsyncContext,
                BOOL isBuiltinQueryable);

/**
 * 返回是否启用内置的查询通道
 *
 * @param   pAsyncContext       异步API的运行时环境指针
 * @return  是否启用内置的查询通道
 */
BOOL    MdsAsyncApi_IsBuiltinQueryable(
                const MdsAsyncApiContextT *pAsyncContext);

/**
 * 返回内置的查询通道是否已连接就绪
 *
 * @param   pAsyncContext       异步API的运行时环境指针
 * @return  TRUE 已连接就绪; FALSE 尚未连接就绪
 */
BOOL    MdsAsyncApi_IsBuiltinQueryChannelConnected(
                const MdsAsyncApiContextT *pAsyncContext);

/**
 * 设置内置的查询通道的配置信息
 *
 * @param[out]  pAsyncContext   异步API的运行时环境指针
 * @param       pRemoteCfg      查询通道的远程主机配置信息
 *                              - 若参数不为空, 则自动启用内置的查询通道
 *                              - 否则若参数为空, 则自动禁用内置的查询通道
 * @return      TRUE 成功; FALSE 失败
 */
BOOL    MdsAsyncApi_SetBuiltinQueryChannelCfg(
                MdsAsyncApiContextT *pAsyncContext,
                const MdsApiRemoteCfgT *pRemoteCfg);

/**
 * 从配置文件中加载内置的查询通道的配置信息
 *
 * @param[out]  pAsyncContext   异步API的运行时环境指针
 * @param       pCfgFile        配置文件路径
 * @param       pCfgSection     查询服务的配置区段名称(为空则使用默认值)
 * @param       pAddrKey        查询服务的配置项关键字(为空则使用默认值)
 * @return      TRUE 成功; FALSE 失败
 */
BOOL    MdsAsyncApi_LoadBuiltinQueryChannelCfg(
                MdsAsyncApiContextT *pAsyncContext,
                const char *pCfgFile,
                const char *pCfgSection,
                const char *pAddrKey);

/**
 * 返回内置的查询通道的配置信息
 *
 * @param   pAsyncContext       异步API的运行时环境指针
 * @return  查询通道的远程主机配置信息
 */
MdsApiRemoteCfgT *
        MdsAsyncApi_GetBuiltinQueryChannelCfg(
                const MdsAsyncApiContextT *pAsyncContext);

/**
 * 返回内置的查询通道的会话信息
 *
 * @param   pAsyncContext       异步API的运行时环境指针
 * @return  内置的查询通道的会话信息 (若尚未启用则返回空)
 */
MdsApiSessionInfoT *
        MdsAsyncApi_GetBuiltinQueryChannelRef(
                const MdsAsyncApiContextT *pAsyncContext);

/**
 * 设置异步I/O线程配置
 *
 * @param[out]  pAsyncContext   异步API的运行时环境指针
 * @param       isEnabled       使能标志
 * @param       isOutputSimplify
 *                              是否采用精简模式输出数据
 * @param       dataOutputFormat
 *                              数据输出格式
 * @param       pDataOutputFilePath
 *                              数据文件输出路径 (为空:不输出数据; STDOUT/STDERR:标准输出)
 * @param       pStatsOutputFilePath
 *                              统计信息文件输出路径 (为空:不输出数据; STDOUT/STDERR:标准输出)
 * @return      TRUE 成功; FALSE 失败
 */
BOOL    MdsAsyncApi_SetIoThreadCfg(
                MdsAsyncApiContextT *pAsyncContext,
                BOOL isEnabled,
                BOOL isOutputSimplify,
                int32 dataOutputFormat,
                const char *pDataOutputFilePath,
                const char *pStatsOutputFilePath);

/**
 * 从配置文件中加载异步I/O线程配置
 *
 * @param[out]  pAsyncContext   异步API的运行时环境指针
 * @param       pCfgFile        配置文件路径
 * @param       pCfgSection     配置区段名称 (e.g. "mds_client.async_api")
 * @return      TRUE 成功; FALSE 失败
 */
BOOL    MdsAsyncApi_LoadIoThreadCfg(
                MdsAsyncApiContextT *pAsyncContext,
                const char *pCfgFile,
                const char *pCfgSection);

/**
 * 返回异步I/O线程配置
 *
 * @param   pAsyncContext       异步API的运行时环境指针
 * @return  异步I/O线程配置信息
 */
MdsAsyncApiIoThreadCfgT *
        MdsAsyncApi_GetIoThreadCfg(
                const MdsAsyncApiContextT *pAsyncContext);

/**
 * 设置异步I/O线程的使能标志
 *
 * @param   pAsyncContext       异步API的运行时环境指针
 * @param   isEnabled           使能标志
 * @return  TRUE 成功; FALSE 失败
 */
BOOL    MdsAsyncApi_SetIoThreadEnabled(
                MdsAsyncApiContextT *pAsyncContext,
                BOOL isEnabled);

/**
 * 返回异步I/O线程的使能标志
 *
 * @param   pAsyncContext       异步API的运行时环境指针
 * @return  异步I/O线程的使能标志
 */
BOOL    MdsAsyncApi_IsIoThreadEnabled(
                const MdsAsyncApiContextT *pAsyncContext);

/**
 * 设置通信线程的线程初始化回调函数
 *
 * @param[out]  pAsyncContext   异步API的运行时环境指针
 * @param       fnOnThreadStart 线程初始化函数 (为空则不回调)
 * @param       pCallbackParams 传递给线程初始化回调函数的参数
 * @return      TRUE 成功; FALSE 失败
 */
BOOL    MdsAsyncApi_SetOnCommunicationThreadStart(
                MdsAsyncApiContextT *pAsyncContext,
                F_MDSAPI_ASYNC_ON_THREAD_START_T fnOnThreadStart,
                void *pCallbackParams);

/**
 * 设置回调线程的线程初始化回调函数 (如果已启用了独立的回调线程的话)
 *
 * @param[out]  pAsyncContext   异步API的运行时环境指针
 * @param       fnOnThreadStart 线程初始化函数 (为空则不回调)
 * @param       pCallbackParams 传递给线程初始化回调函数的参数
 * @return      TRUE 成功; FALSE 失败
 *
 * @see         MdsAsyncApi_SetAsyncCallbackAble
 */
BOOL    MdsAsyncApi_SetOnCallbackThreadStart(
                MdsAsyncApiContextT *pAsyncContext,
                F_MDSAPI_ASYNC_ON_THREAD_START_T fnOnThreadStart,
                void *pCallbackParams);

/**
 * 设置异步I/O线程的线程初始化回调函数 (如果已启用了异步I/O线程的话)
 *
 * @param[out]  pAsyncContext   异步API的运行时环境指针
 * @param       fnOnThreadStart 线程初始化函数 (为空则不回调)
 * @param       pCallbackParams 传递给线程初始化回调函数的参数
 * @return      TRUE 成功; FALSE 失败
 *
 * @see         MdsAsyncApi_SetIoThreadCfg
 */
BOOL    MdsAsyncApi_SetOnIoThreadStart(
                MdsAsyncApiContextT *pAsyncContext,
                F_MDSAPI_ASYNC_ON_THREAD_START_T fnOnThreadStart,
                void *pCallbackParams);
/* -------------------------           */


/* ===================================================================
 * 查询接口 (同步API查询接口的包裹函数, 并基于内置的查询通道执行)
 * =================================================================== */

/**
 * 获取API的发行版本号
 *
 * @note    与同步API接口的异同:
 *          - 与同步API接口相同, 没有区别
 *
 * @return  API的发行版本号 (如: "0.15.3")
 * @see     MdsApi_GetApiVersion
 */
const char *
        MdsAsyncApi_GetApiVersion();

/**
 * 查询证券行情快照 (基于异步API内置的查询通道执行)
 *
 * @note        与同步API接口的不同处:
 *              - 支持自动重建连接
 *              - 线程安全 (内置了加锁处理)
 * @note        与同步API接口的相同处:
 *              - 均为请求/应答模式的同步调用
 *
 * @param       pAsyncContext   异步API的运行时环境指针
 * @param       exchangeId      交易所代码
 * @param       mdProductType   行情产品类型
 * @param       instrId         证券代码 (转换为整数类型的证券代码)
 * @param[out]  pRspBuf         用于输出查询结果的应答数据缓存
 *                              - 消息体的数据类型为L1快照 <code>MdsL1SnapshotT</code>
 * @retval      =0              查询成功
 * @retval      <0              查询失败 (负的错误号)
 * @exception   ENOENT          未检索到待查询的数据
 *
 * @see         MdsApi_QueryMktDataSnapshot
 * @see         MdsL1SnapshotT
 */
int32   MdsAsyncApi_QueryMktDataSnapshot(
                MdsAsyncApiContextT *pAsyncContext,
                eMdsExchangeIdT exchangeId,
                eMdsMdProductTypeT mdProductType,
                int32 instrId,
                MdsMktDataSnapshotT *pRspBuf);

/**
 * 批量查询行情快照 (基于异步API内置的查询通道执行)
 *
 * @note    与同步API接口的不同处:
 *          - 支持自动重建连接
 *          - 线程安全 (内置了加锁处理)
 * @note    与同步API接口的相同处:
 *          - 均为请求/应答模式的同步调用 (回调函数也是在当前线程下同步调用的)
 *
 * @param   pAsyncContext       异步API的运行时环境指针
 * @param   pSecurityListStr    证券代码列表字符串
 *                              - 证券代码支持以 .SH 或 .SZ 为后缀来指定其所属的交易所
 *                              - 空字符串 "" 或 NULL, 表示查询所有产品的行情 (不包括指数和期权)
 * @param   pDelim              证券代码列表的分隔符 (e.g. ",;| \t")
 *                              - 如果为空, 则使用默认的分隔符:
 *                                ',' 或 ';' 或 '|' 或 ' ' 或 '\t'
 * @param   pQryFilter          查询过滤条件
 *                              - 传空指针或者将过滤条件初始化为0, 代表无需过滤
 * @param   fnQryMsgCallback    进行消息处理的回调函数
 *                              - 消息体的数据类型为L1快照 <code>MdsL1SnapshotT</code>
 * @param   pCallbackParams     回调函数的参数
 * @retval  >=0                 成功查询到的记录数
 * @retval  <0                  失败 (负的错误号)
 *
 * @see     MdsApi_QuerySnapshotList
 * @see     MdsL1SnapshotT
 */
int32   MdsAsyncApi_QuerySnapshotList(
                MdsAsyncApiContextT *pAsyncContext,
                const char *pSecurityListStr,
                const char *pDelim,
                const MdsQrySnapshotListFilterT *pQryFilter,
                F_MDSAPI_ON_QRY_MSG_T fnQryMsgCallback,
                void *pCallbackParams);

/**
 * 批量查询行情快照 (使用字符串指针数组形式的证券代码列表)
 *
 * @note    与同步API接口的不同处:
 *          - 支持自动重建连接
 *          - 线程安全 (内置了加锁处理)
 * @note    与同步API接口的相同处:
 *          - 均为请求/应答模式的同步调用 (回调函数也是在当前线程下同步调用的)
 *
 * @param   pAsyncContext       异步API的运行时环境指针
 * @param   ppSecurityArray     证券代码列表的指针数组
 *                              - 证券代码支持以 .SH 或 .SZ 为后缀来指定其所属的交易所
 *                              - 空指针NULL或代码数量为0, 表示查询所有产品的行情 (不包括指数和期权)
 * @param   securityCount       证券代码数量
 * @param   pQryFilter          查询过滤条件
 *                              - 传空指针或者将过滤条件初始化为0, 代表无需过滤
 * @param   fnQryMsgCallback    进行消息处理的回调函数
 *                              - 消息体的数据类型为L1快照 <code>MdsL1SnapshotT</code>
 * @param   pCallbackParams     回调函数的参数
 * @retval  >=0                 成功查询到的记录数
 * @retval  <0                  失败 (负的错误号)
 *
 * @see     MdsApi_QuerySnapshotList2
 * @see     MdsL1SnapshotT
 */
int32   MdsAsyncApi_QuerySnapshotList2(
                MdsAsyncApiContextT *pAsyncContext,
                const char *ppSecurityArray[],
                int32 securityCount,
                const MdsQrySnapshotListFilterT *pQryFilter,
                F_MDSAPI_ON_QRY_MSG_T fnQryMsgCallback,
                void *pCallbackParams);

/**
 * 查询(深圳)证券实时状态 (基于异步API内置的查询通道执行)
 *
 * @note        与同步API接口的不同处:
 *              - 支持自动重建连接
 *              - 线程安全 (内置了加锁处理)
 * @note        与同步API接口的相同处:
 *              - 均为请求/应答模式的同步调用
 *
 * @param       pAsyncContext   异步API的运行时环境指针
 * @param       exchangeId      交易所代码
 * @param       mdProductType   行情产品类型
 * @param       instrId         证券代码 (转换为整数类型的证券代码)
 * @param[out]  pRspBuf         用于输出查询结果的应答数据缓存
 * @retval      =0              查询成功
 * @retval      <0              查询失败 (负的错误号)
 * @exception   ENOENT          未检索到待查询的数据
 *
 * @see         MdsApi_QuerySecurityStatus
 * @see         MdsSecurityStatusMsgT
 */
int32   MdsAsyncApi_QuerySecurityStatus(
                MdsAsyncApiContextT *pAsyncContext,
                eMdsExchangeIdT exchangeId,
                eMdsMdProductTypeT mdProductType,
                int32 instrId,
                MdsSecurityStatusMsgT *pRspBuf);

/**
 * 查询(上证)市场状态 (基于异步API内置的查询通道执行)
 *
 * @note        与同步API接口的不同处:
 *              - 支持自动重建连接
 *              - 线程安全 (内置了加锁处理)
 * @note        与同步API接口的相同处:
 *              - 均为请求/应答模式的同步调用
 *
 * @param       pAsyncContext   异步API的运行时环境指针
 * @param       exchangeId      交易所代码
 * @param       mdProductType   行情产品类型
 * @param[out]  pRspBuf         用于输出查询结果的应答数据缓存
 * @retval      =0              查询成功
 * @retval      <0              查询失败 (负的错误号)
 * @exception   ENOENT          未检索到待查询的数据
 *
 * @see         MdsApi_QueryTrdSessionStatus
 * @see         MdsTradingSessionStatusMsgT
 */
int32   MdsAsyncApi_QueryTrdSessionStatus(
                MdsAsyncApiContextT *pAsyncContext,
                eMdsExchangeIdT exchangeId,
                eMdsMdProductTypeT mdProductType,
                MdsTradingSessionStatusMsgT *pRspBuf);

/**
 * 批量查询证券(股票/债券/基金)静态信息列表 (基于异步API内置的查询通道执行)
 *
 * @note    与同步API接口的不同处:
 *          - 支持自动重建连接
 *          - 线程安全 (内置了加锁处理)
 * @note    与同步API接口的相同处:
 *          - 均为请求/应答模式的同步调用 (回调函数也是在当前线程下同步调用的)
 *
 * @param   pAsyncContext       异步API的运行时环境指针
 * @param   pSecurityListStr    证券代码列表字符串
 *                              - 证券代码支持以 .SH 或 .SZ 为后缀来指定其所属的交易所
 *                              - 空字符串 "" 或 NULL, 表示查询所有证券 (不包括指数和期权)
 * @param   pDelim              证券代码列表的分隔符 (e.g. ",;| \t")
 *                              - 如果为空, 则使用默认的分隔符:
 *                                ',' 或 ';' 或 '|' 或 ' ' 或 '\t'
 * @param   pQryFilter          查询过滤条件
 *                              - 传空指针或者将过滤条件初始化为0, 代表无需过滤
 * @param   fnQryMsgCallback    进行消息处理的回调函数
 *                              - 消息体的数据类型为证券静态信息 <code>MdsStockStaticInfoT</code>
 * @param   pCallbackParams     回调函数的参数
 * @retval  >=0                 成功查询到的记录数
 * @retval  <0                  失败 (负的错误号)
 *
 * @see     MdsApi_QueryStockStaticInfoList
 * @see     MdsStockStaticInfoT
 */
int32   MdsAsyncApi_QueryStockStaticInfoList(
                MdsAsyncApiContextT *pAsyncContext,
                const char *pSecurityListStr,
                const char *pDelim,
                const MdsQryStockStaticInfoListFilterT *pQryFilter,
                F_MDSAPI_ON_QRY_MSG_T fnQryMsgCallback,
                void *pCallbackParams);

/**
 * 批量查询证券(股票/债券/基金)静态信息列表 (字符串指针数组形式的证券代码列表)
 *
 * @note    与同步API接口的不同处:
 *          - 支持自动重建连接
 *          - 线程安全 (内置了加锁处理)
 * @note    与同步API接口的相同处:
 *          - 均为请求/应答模式的同步调用 (回调函数也是在当前线程下同步调用的)
 *
 * @param   pAsyncContext       异步API的运行时环境指针
 * @param   ppSecurityArray     证券代码列表的指针数组
 *                              - 证券代码支持以 .SH 或 .SZ 为后缀来指定其所属的交易所
 *                              - 空指针NULL或代码数量为0, 表示查询所有证券 (不包括指数和期权)
 * @param   securityCount       证券代码数量
 * @param   pQryFilter          查询过滤条件
 *                              - 传空指针或者将过滤条件初始化为0, 代表无需过滤
 * @param   fnQryMsgCallback    进行消息处理的回调函数
 *                              - 消息体的数据类型为证券静态信息 <code>MdsStockStaticInfoT</code>
 * @param   pCallbackParams     回调函数的参数
 * @retval  >=0                 成功查询到的记录数
 * @retval  <0                  失败 (负的错误号)
 *
 * @see     MdsApi_QueryStockStaticInfoList2
 * @see     MdsStockStaticInfoT
 */
int32   MdsAsyncApi_QueryStockStaticInfoList2(
                MdsAsyncApiContextT *pAsyncContext,
                const char *ppSecurityArray[],
                int32 securityCount,
                const MdsQryStockStaticInfoListFilterT *pQryFilter,
                F_MDSAPI_ON_QRY_MSG_T fnQryMsgCallback,
                void *pCallbackParams);

/**
 * 批量查询期权合约静态信息列表 (基于异步API内置的查询通道执行)
 *
 * @note    与同步API接口的不同处:
 *          - 支持自动重建连接
 *          - 线程安全 (内置了加锁处理)
 * @note    与同步API接口的相同处:
 *          - 均为请求/应答模式的同步调用 (回调函数也是在当前线程下同步调用的)
 *
 * @param   pAsyncContext       异步API的运行时环境指针
 * @param   pSecurityListStr    期权代码列表字符串
 *                              - 期权代码支持以 .SH 或 .SZ 为后缀来指定其所属的交易所
 *                              - 空字符串 "" 或 NULL, 表示查询所有期权
 * @param   pDelim              期权代码列表的分隔符 (e.g. ",;| \t")
 *                              - 如果为空, 则使用默认的分隔符:
 *                                ',' 或 ';' 或 '|' 或 ' ' 或 '\t'
 * @param   pQryFilter          查询过滤条件
 *                              - 传空指针或者将过滤条件初始化为0, 代表无需过滤
 * @param   fnQryMsgCallback    进行消息处理的回调函数
 *                              - 消息体的数据类型为期权静态信息 <code>MdsOptionStaticInfoT</code>
 * @param   pCallbackParams     回调函数的参数
 * @retval  >=0                 成功查询到的记录数
 * @retval  <0                  失败 (负的错误号)
 *
 * @see     MdsApi_QueryOptionStaticInfoList
 * @see     MdsOptionStaticInfoT
 */
int32   MdsAsyncApi_QueryOptionStaticInfoList(
                MdsAsyncApiContextT *pAsyncContext,
                const char *pSecurityListStr,
                const char *pDelim,
                const MdsQryOptionStaticInfoListFilterT *pQryFilter,
                F_MDSAPI_ON_QRY_MSG_T fnQryMsgCallback,
                void *pCallbackParams);

/**
 * 批量查询期权合约静态信息列表 (字符串指针数组形式的证券代码列表)
 *
 * @note    与同步API接口的不同处:
 *          - 支持自动重建连接
 *          - 线程安全 (内置了加锁处理)
 * @note    与同步API接口的相同处:
 *          - 均为请求/应答模式的同步调用 (回调函数也是在当前线程下同步调用的)
 *
 * @param   pAsyncContext       异步API的运行时环境指针
 * @param   ppSecurityArray     期权代码列表的指针数组
 *                              - 期权代码支持以 .SH 或 .SZ 为后缀来指定其所属的交易所
 *                              - 空指针NULL或代码数量为0, 表示查询所有期权
 * @param   securityCount       期权代码数量
 * @param   pQryFilter          查询过滤条件
 *                              - 传空指针或者将过滤条件初始化为0, 代表无需过滤
 * @param   fnQryMsgCallback    进行消息处理的回调函数
 *                              - 消息体的数据类型为期权静态信息 <code>MdsOptionStaticInfoT</code>
 * @param   pCallbackParams     回调函数的参数
 * @retval  >=0                 成功查询到的记录数
 * @retval  <0                  失败 (负的错误号)
 *
 * @see     MdsApi_QueryOptionStaticInfoList2
 * @see     MdsOptionStaticInfoT
 */
int32   MdsAsyncApi_QueryOptionStaticInfoList2(
                MdsAsyncApiContextT *pAsyncContext,
                const char *ppSecurityArray[],
                int32 securityCount,
                const MdsQryOptionStaticInfoListFilterT *pQryFilter,
                F_MDSAPI_ON_QRY_MSG_T fnQryMsgCallback,
                void *pCallbackParams);
/* -------------------------           */


/* ===================================================================
 * 逐笔数据重传请求接口函数声明
 * =================================================================== */

/**
 * 发送逐笔数据重传请求
 * 逐笔数据重传请求通过查询通道发送到MDS服务器, 并采用请求/应答的方式返回处理结果并同步执行回调函数
 *
 * @note        因为上交所逐笔合并数据即将上线, 为了简化接口, 未提供是否上交所老版竞价逐笔委托
 *              频道 (isSseOldTickOrder) 参数。在逐笔合并数据上线前如需重传上交所老版竞价
 *              逐笔委托数据, 需要使用 MdsAsyncApi_SendTickResendRequest2 接口
 *
 * @note        建议每次请求重传的数据不要大于 1000 (MDS_MAX_TICK_RESEND_ITEM_COUNT) 条,
 *              如果请求重传的数据范围超过 1000 条, 服务器端只会返回前 1000 条数据
 * @note        提示: 重传请求的应答数据类型和回调函数的执行次序如下:
 *              - [0~1000] 条重传的逐笔数据, msgType 为: MDS_MSGTYPE_L2_TRADE / MDS_MSGTYPE_L2_ORDER / MDS_MSGTYPE_L2_SSE_ORDER
 *              - 1 条逐笔数据重传请求的应答消息, msgType 为: MDS_MSGTYPE_TICK_RESEND_REQUEST
 *
 * @param       pAsyncContext   异步API的运行时环境指针
 * @param       exchangeId      交易所代码(沪/深) @see eMdsExchangeIdT
 * @param       channelNo       频道代码 (取值范围[1..9999])
 * @param       beginApplSeqNum 待重传的逐笔数据起始序号
 * @param       endApplSeqNum   待重传的逐笔数据结束序号
 * @param       fnQryMsgCallback
 *                              进行消息处理的回调函数
 *                              - 消息体的数据类型包括:
 *                                - MDS_MSGTYPE_L2_TRADE            => @see MdsL2TradeT
 *                                - MDS_MSGTYPE_L2_ORDER            => @see MdsL2OrderT
 *                                - MDS_MSGTYPE_L2_SSE_ORDER        => @see MdsL2OrderT
 *                                - MDS_MSGTYPE_TICK_RESEND_REQUEST => @see MdsTickResendRequestRspT
 * @param       pCallbackParams 回调函数的参数
 * @retval      >=0             成功重建到的逐笔成交/逐笔委托记录数
 * @retval      <0              失败 (负的错误号)
 *
 * @see         MdsApi_SendTickResendRequest
 */
int32   MdsAsyncApi_SendTickResendRequest(
                MdsAsyncApiContextT *pAsyncContext,
                uint8 exchangeId,
                uint16 channelNo,
                uint32 beginApplSeqNum,
                uint32 endApplSeqNum,
                F_MDSAPI_ON_QRY_MSG_T fnQryMsgCallback,
                void *pCallbackParams);

/**
 * 发送逐笔数据重传请求
 * 逐笔数据重传请求通过查询通道发送到MDS服务器, 并采用请求/应答的方式返回处理结果并同步执行回调函数
 *
 * @note        建议每次请求重传的数据不要大于 1000 (MDS_MAX_TICK_RESEND_ITEM_COUNT) 条,
 *              如果请求重传的数据范围超过 1000 条, 服务器端只会返回前 1000 条数据
 * @note        提示: 重传请求的应答数据类型和回调函数的执行次序如下:
 *              - [0~1000] 条重传的逐笔数据, msgType 为: MDS_MSGTYPE_L2_TRADE / MDS_MSGTYPE_L2_ORDER / MDS_MSGTYPE_L2_SSE_ORDER
 *              - 1 条逐笔数据重传请求的应答消息, msgType 为: MDS_MSGTYPE_TICK_RESEND_REQUEST
 *
 * @param       pAsyncContext   异步API的运行时环境指针
 * @param       pResendReq      待发送的逐笔数据重传请求
 * @param       fnQryMsgCallback
 *                              进行消息处理的回调函数
 *                              - 消息体的数据类型包括:
 *                                - MDS_MSGTYPE_L2_TRADE            => @see MdsL2TradeT
 *                                - MDS_MSGTYPE_L2_ORDER            => @see MdsL2OrderT
 *                                - MDS_MSGTYPE_L2_SSE_ORDER        => @see MdsL2OrderT
 *                                - MDS_MSGTYPE_TICK_RESEND_REQUEST => @see MdsTickResendRequestRspT
 * @param       pCallbackParams 回调函数的参数
 * @param[out]  pOutResendRsp   输出逐笔数据重传请求的应答消息 (可选参数, 可以为空)
 * @retval      >=0             成功重建到的逐笔成交/逐笔委托记录数
 * @retval      <0              失败 (负的错误号)
 *
 * @see         MdsApi_SendTickResendRequest2
 */
int32   MdsAsyncApi_SendTickResendRequest2(
                MdsAsyncApiContextT *pAsyncContext,
                const MdsTickResendRequestReqT *pResendReq,
                F_MDSAPI_ON_QRY_MSG_T fnQryMsgCallback,
                void *pCallbackParams,
                MdsTickResendRequestRspT *pOutResendRsp);

/**
 * 发送超大的逐笔数据重传请求
 * 支持不限制大小的重传请求 (将自动拆分为多条小的逐笔数据重传请求发送到MDS服务器, 并采用请求/应答的方式返回处理结果并同步执行回调函数)
 *
 * @note        注意: 请谨慎使用该接口, 避免发起过大或不必要的重传请求, 以免挤占带宽资源
 * @note        提示: 重传请求的应答数据类型和回调函数的执行次序如下:
 *              - n 条重传的逐笔数据, msgType 为: MDS_MSGTYPE_L2_TRADE / MDS_MSGTYPE_L2_ORDER / MDS_MSGTYPE_L2_SSE_ORDER
 *              - 1 条逐笔数据重传请求的应答消息, msgType 为: MDS_MSGTYPE_TICK_RESEND_REQUEST
 * @note        当长时间请求不到任何数据时而超时返回时, 函数返回值仍然为大于等于0的值.
 *              具体的重传状态可以通过重传请求的应答消息(回调处理)判断, 或直接通过函数返回值
 *              (成功重建到的逐笔数量)进行判断
 *
 * @param       pAsyncContext   异步API的运行时环境指针
 * @param       pResendReq      待发送的逐笔数据重传请求
 * @param       fnQryMsgCallback
 *                              进行消息处理的回调函数
 *                              - 消息体的数据类型包括:
 *                                - MDS_MSGTYPE_L2_TRADE            => @see MdsL2TradeT
 *                                - MDS_MSGTYPE_L2_ORDER            => @see MdsL2OrderT
 *                                - MDS_MSGTYPE_L2_SSE_ORDER        => @see MdsL2OrderT
 *                                - MDS_MSGTYPE_TICK_RESEND_REQUEST => @see MdsTickResendRequestRspT
 * @param       pCallbackParams 回调函数的参数
 * @param[out]  pOutResendRsp   输出逐笔数据重传请求的应答消息 (可选参数, 可以为空)
 * @param       timeoutMs       长时间请求不到任何数据时的超时时间 (单位:毫秒)
 *                              - 大于0, 最大超时时间 (毫秒)
 *                              - 等于0, 立即返回
 *                              - 小于0, 使用默认值 (30秒)
 * @retval      >=0             成功重建到的逐笔成交/逐笔委托记录数
 * @retval      <0              失败 (负的错误号)
 *
 * @see         MdsApi_SendTickResendRequestHugely
 */
int32   MdsAsyncApi_SendTickResendRequestHugely(
                MdsAsyncApiContextT *pAsyncContext,
                const MdsTickResendRequestReqT *pResendReq,
                F_MDSAPI_ON_QRY_MSG_T fnQryMsgCallback,
                void *pCallbackParams,
                MdsTickResendRequestRspT *pOutResendRsp,
                int32 timeoutMs);
/* -------------------------           */


/* ===================================================================
 * 密码修改接口函数声明
 * =================================================================== */

/**
 * 发送密码修改请求 (修改客户端登录密码)
 * 密码修改请求通过查询通道发送到MDS服务器, 并采用请求/应答的方式直接返回处理结果
 *
 * @note        与同步API接口的不同处:
 *              - 支持自动重建连接
 *              - 线程安全 (内置了加锁处理)
 * @note        与同步API接口的相同处:
 *              - 均为请求/应答模式的同步调用
 *
 * @param       pAsyncContext   异步API的运行时环境指针
 * @param       pChangePasswordReq
 *                              待发送的密码修改请求
 * @param[out]  pOutChangePasswordRsp
 *                              用于输出密码修改请求应答的缓存区
 * @retval      0               成功
 * @retval      <0              API调用失败 (负的错误号)
 * @retval      >0              服务端业务处理失败 (MDS错误号)
 *
 * @exception   EINVAL          传入参数非法
 * @exception   EPIPE           连接已破裂
 * @exception   Others          由send()系统调用返回的错误
 */
int32   MdsAsyncApi_SendChangePasswordReq(
                MdsAsyncApiContextT *pAsyncContext,
                const MdsChangePasswordReqT *pChangePasswordReq,
                MdsChangePasswordRspT *pOutChangePasswordRsp);
/* -------------------------           */


/* ===================================================================
 * 辅助的连接管理接口 (用于控制是否需要暂停连接重建处理等特殊场景)
 * =================================================================== */

/**
 * 触发连接断开事件, 主动断开连接
 *
 * @param   pAsyncChannel       异步API的连接通道信息
 * @retval  >=0                 大于等于0, 成功
 * @retval  <0                  小于0, 处理失败, 异步API将中止运行
 */
int32   __MdsAsyncApi_TriggerChannelBroken(
                MdsAsyncApiChannelT *pAsyncChannel);

/**
 * 设置通道的连接管理标志为 '暂停重连', 以暂停该通道的连接重建处理
 *
 * @param   pAsyncChannel       异步API的连接通道信息
 * @param   isSuspended         是否暂停重连 (暂停该通道的连接重建处理)
 * @param   suspendAbsoluteExpirationTime
 *                              暂停重连的过期时间 (UTC时间, 即相对于1970年的秒数)
 *                              - 小于等于0表示没有过期时间 (即一直保持暂停重连状态)
 *                              - @note 该值需设置为绝对时间 (自1970年至今的总秒数)
 */
void    __MdsAsyncApi_SetChannelSuspended(
                MdsAsyncApiChannelT *pAsyncChannel,
                BOOL isSuspended,
                int64 suspendAbsoluteExpirationTime);

/**
 * 返回通道的连接管理标志是否为 '暂停重连'
 *
 * @param   pAsyncChannel       异步API的连接通道信息
 * @return  通道的连接管理标志是否为 '暂停重连'
 */
BOOL    __MdsAsyncApi_IsChannelSuspended(
                const MdsAsyncApiChannelT *pAsyncChannel);

/**
 * 返回通道暂停重连的过期时间
 *
 * @param   pAsyncChannel       异步API的连接通道信息
 * @return  暂停重连的过期时间 (UTC时间, 即相对于1970年的秒数)
 *          - 小于等于0表示没有过期时间 (即一直保持暂停重连状态)
 */
int64   __MdsAsyncApi_GetChannelSuspendExpirationTime(
                const MdsAsyncApiChannelT *pAsyncChannel);
/* -------------------------           */


/* ===================================================================
 * 辅助的 Inline 函数
 * =================================================================== */

/**
 * 返回异步API线程是否正在运行过程中
 *
 * MdsAsyncApi_IsRunning 接口的 inline 简化版本
 *
 * @param   pAsyncContext       异步API的运行时环境指针 (@note 参数不能为空)
 * @return  TRUE 正在运行过程中; FALSE 已终止或尚未运行
 */
static __inline BOOL
__MdsAsyncApi_IsRunning(const MdsAsyncApiContextT *pAsyncContext) {
    return pAsyncContext->pInternalRefs != NULL
            && *((volatile uint8 *) &pAsyncContext->terminateFlag) == 0;
}


/**
 * 返回通道是否已连接就绪
 *
 * MdsAsyncApi_IsChannelConnected 接口的 inline 版本
 *
 * @param   pAsyncChannel       异步API的连接通道信息 (@note 参数不能为空)
 * @return  TRUE 已连接就绪; FALSE 尚未连接就绪
 */
static __inline BOOL
__MdsAsyncApi_IsChannelConnected(const MdsAsyncApiChannelT *pAsyncChannel) {
    return pAsyncChannel->pSessionInfo
            && (*((volatile uint8 *) &pAsyncChannel->isConnected)
                    >= SPK_ENDPOINT_CHANNEL_STATUS_READY);
}
/* -------------------------           */


#ifdef __cplusplus
}
#endif

#endif  /* _MDS_ASYNC_API_H */
