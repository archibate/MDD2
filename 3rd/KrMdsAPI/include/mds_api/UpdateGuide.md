# MDS-API Update Guide    {#update_guide}

MdsApi 版本更新说明
-----------------------------------

##### *完整变更说明请参考文档:*
  - *`include/mds_api/CHANGELOG.md`*
  - *`include/mds_api/UpdateGuide.md`*

MDS_0.17.6.17 / 2024-09-18
-----------------------------------

### 1. API更新概要

  1. 服务端兼容 v0.15.5.1 版本API, 客户可以选择不升级 (建议升级至当前版本)
  2. 删除 v0.17.6.13 版本引入的股、债分离的Lv1/Lv2快照行情结构体定义, 以方便后续版本对快照行情结构进行重构和统一

### 2. 服务端更新概要

  * 无

### 3. 注意事项

  1. 删除了 v0.17.6.13 引入的股、债分离的Lv1/Lv2快照行情结构体定义:
     - 废弃结构体定义 MdsL1PureStockSnapshotBodyT, 请直接使用 MdsStockSnapshotBodyT
     - 废弃结构体定义 MdsL1PureBondSnapshotBodyT, 请直接使用 MdsStockSnapshotBodyT
     - 废弃结构体定义 MdsL2PureStockSnapshotBodyT, 请直接使用 MdsL2StockSnapshotBodyT
     - 废弃结构体定义 MdsL2PureBondSnapshotBodyT, 请直接使用 MdsL2StockSnapshotBodyT

---

MDS_0.17.6.16 / 2024-07-19
-----------------------------------

### 1. API更新概要

  1. 服务端兼容 v0.15.5.1 版本API, 客户可以选择不升级 (建议升级至当前版本)
  2. 调整API的默认编译参数, 由 -Ofast 调整为 -O3, 以避免在 Ubuntu 下报找不到 __exp_finite 函数的链接错误

### 2. 服务端更新概要

  1. 细节完善和优化

### 3. 注意事项

  * 无

---

MDS_0.17.6.14 / 2024-05-20
-----------------------------------

### 1. API更新概要

  * 无

### 2. 服务端更新概要

  1. 细节完善和优化

### 3. 注意事项

  * 无

---

MDS_0.17.6.13_u1 / 2024-04-17
-----------------------------------

### 1. API更新概要

  1. 服务端兼容 v0.15.5.1 版本API, 客户可以选择不升级 (建议升级至当前版本)
  2. fix: 修复与lz4库一起编译时会报函数 (read_long_length_no_check) 多重定义的编译错误
  3. 同时生成和发布非PIC(位置无关代码)和PIC版本的静态库
    - 普通静态库: linux64/liboes_api.a, linux64/libmds_api.a
    - 位置无关代码(PIC)版本的静态库: linux64/pic/liboes_api.pic.a, linux64/pic/libmds_api.pic.a

### 2. 服务端更新概要

  * 无

### 3. 注意事项

  * 无

---

MDS_0.17.6.13 / 2024-03-11
-----------------------------------

### 1. API更新概要

  1. 服务端兼容 v0.15.5.1 版本API, 客户可以选择不升级 (建议升级至当前版本)
  2. 调整行情数据来源枚举类型的取值, 以区分多路择优场景下来源于不同线路的行情数据 (@note 协议保持兼容, 但 `__origMdSource` 字段的取值发生变化 `@see eMdsMsgSourceT`)
  3. 废弃快照头中的内部字段 `__directSourceId`, 取值固定为0
  4. 额外增加更加扁平化的股、债分离的Lv1/Lv2快照行情结构体定义 (@note 已废弃, 请忽略该调整)

### 2. 服务端更新概要

  1. 对接深交所新增的基金实时参考净值(IOPV)行情数据
  2. 修复没有转发上交所逐笔合并数据的撤单委托价格的问题
  3. 完善组播身份认证, 增加对 Level-2 数据授权的检查, 要求具备 Level-2 数据权限才能对接组播行情
  4. 其它细节优化和完善

### 3. 注意事项

  1. 行情数据来源 (__origMdSource) 字段的取值发生变化 `@see eMdsMsgSourceT`
  2. 逐笔数据中的 SseBizIndex 字段已标识为即将废弃, 待全面切换到逐笔合并数据以后将废弃该字段

---

MDS_0.17.6.12 / 2023-11-10
-----------------------------------

### 1. API更新概要

  1. 服务端兼容 v0.15.5.1 版本API, 客户可以选择不升级 (建议升级至当前版本)
  2. fix: 修复在MinGW环境下错误码识别不准确的问题
  3. fix: 完善异步API对 `SetThreadUsername` 等线程局部变量的支持, 避免初始化时设置的线程局部变量无法应用到连接管理线程的问题
  4. 完善对内置查询通道的支持, 当未配置查询通道的用户名称和密码时, 尝试通过TCP通道自动填充内置查询通道的用户名称和密码

### 2. 服务端更新概要

  1. 其他细节完善和优化

### 3. 注意事项

  * 无

---

MDS_0.17.6.11 / 2023-10-07
-----------------------------------

### 1. API更新概要

  1. 服务端兼容 v0.15.5.1 版本API, 客户可以选择不升级 (建议升级至当前版本)
  2. 修复当启用异步回调线程时, OnMsg 回调函数返回负数会导致异步API实例中止运行和退出的问题。当 OnMsg 回调函数返回负数时, 将尝试断开并重建对应的连接
  3. 优化异步API, 增加对 onload 优化选项的支持
  4. 增加辅助的异步API接口
    | API                                            | 描述
    | ---------------------------------------------- | ---------------
    | MdsAsyncApi_GetTotalIoPicked                   | 返回异步I/O线程累计已提取和处理过的消息数量
    | MdsAsyncApi_LoadContextParams                  | 从配置文件中加载异步API运行参数
    | MdsAsyncApi_SetOnloadFlag                      | 设置 Onload 加速标志
    | MdsAsyncApi_GetOnloadFlag                      | 返回 Onload 加速标志
    | MdsAsyncApi_SetIoThreadEnabled                 | 设置异步I/O线程的使能标志
    | MdsAsyncApi_IsIoThreadEnabled                  | 返回异步I/O线程的使能标志
  5. 添加辅助的连接管理接口
    | API                                            | 描述
    | ---------------------------------------------- | ---------------
    | __MdsAsyncApi_TriggerChannelBroken             | 触发连接断开事件, 主动断开连接
  6. 新增如下错误码
    | 错误码 | 描述
    | ---- | ---------------
    | 1044 | 数据无变化
    | 1045 | 无效的登录用户名称或密码
    | 1046 | 密码已被锁定

### 2. 服务端更新概要

  1. 其他细节完善和优化

### 3. 注意事项

  1. 在 Linux 平台下, 编译API时需要增加链接参数 -ldl

---

MDS_0.17.6.10 / 2023-08-06
-----------------------------------

### 1. API更新概要

  1. 服务端兼容 v0.15.5.1 版本API, 客户可以选择不升级 (建议升级至当前版本)
  2. 为组播行情增加身份认证处理 (API接口以及现有配置和行为均保持兼容)
  3. 新增 MdsApi_InitUdpChannel3 接口, 以方便指定组播频道类型
       | API                                            | 描述
       | ---------------------------------------------- | ---------------
       | MdsApi_InitUdpChannel3                         | 初始化基于UDP协议的行情订阅通道 (包括完整的连接建立过程)
  4. 将上交所Level-2快照行情的增量更新消息以及相关的常量定义和结构体定义标识为已废弃 (上交所行情快照发送机制调整后, 不再推送增量更新消息)
  5. 调整逐笔数据重建标识(tickRebuildFlag)的含义, 补充和重复的逐笔数据相关的内容:
     - 该字段已过时, 建议固定设置为0, 并通过逐笔数据重传接口来重传缺失的逐笔数据
     - 当设置 tickRebuildFlag=1 时, 除了会接收到重建到的逐笔数据, 还可能会接收到重复的逐笔数据 (当服务器端同时对接多路行情时)
     - 调整后的取值范围说明:
       - 0: 不订阅重建到的逐笔数据或重复的逐笔数据
       - 1: 订阅重建到的逐笔数据和重复的逐笔数据 (实时行情+重建数据+重复的逐笔数据)
       - 2: 只订阅重建到的逐笔数据 (仅重建数据 @note 需要通过压缩行情端口进行订阅, 非压缩行情和组播行情不支持该选项)

### 2. 服务端更新概要

  1. 支持组播行情身份认证
  2. 增加行情多路择优功能, 支持同时对接多路行情
  3. 其他细节完善和优化

### 3. 注意事项

  1. 关于组播行情的身份认证处理:
     - API接口以及现有行为保持兼容
       - 可以保持现有配置不变 (直接配置为实际的组播地址), 配置和程序均无需做任何改变
     - 如需启用组播身份认证处理, 则可以将各组播频道的组播地址均配置为查询服务地址, 这样就能够通过查询服务来实现组播行情的身份认证和组播频道的初始化处理
       - 对于 MdsApi_InitUdpChannel 和 MdsAsyncApi_AddChannelFromFile 接口, 无需进行调整, 可以自动根据配置项关键字名称来识别组播频道类型
       - 对于 MdsApi_InitUdpChannel2 和 MdsAsyncApi_AddChannel 接口, 可以通过 remoteCfg.targetCompId 来指定组播频道类型.
         如果是通过 MdsApi_ParseConfigFromFile 等接口解析的配置信息, 则会自动填充 targetCompId 字段, 无需额外设置
       - 另外, 新增 MdsApi_InitUdpChannel3 接口, 可以直接通过参数指定组播频道类型
  2. 关于逐笔数据重建标识(tickRebuildFlag):
     - 当设置 tickRebuildFlag=1 时, 除了会接收到重建到的逐笔数据, 还可能会接收到重复的逐笔数据 (当服务器端同时对接多路行情时)
     - 该字段已过时, 建议固定设置为0, 并通过逐笔数据重传接口来重传缺失的逐笔数据

---

MDS_0.17.6.8_u1 / 2023-03-03
-----------------------------------

### 1. API更新概要

  1. 服务端兼容 v0.15.5.1 版本API, 客户可以选择不升级 (建议升级至当前版本)
  2. 证券状态枚举(`eOesSecurityStatusT`) 中:
    - 新增 退市整理期首日(OES_SECURITY_STATUS_FIRST_DELIST) 定义
  3. 为异步API添加辅助的连接管理接口 (用于控制是否需要暂停连接重建处理等特殊场景)
       | API                                            | 描述
       | ---------------------------------------------- | ---------------
       | __MdsAsyncApi_SetChannelSuspended              | 设置通道的连接管理标志为 '暂停重连', 以暂停该通道的连接重建处理
       | __MdsAsyncApi_IsChannelSuspended               | 返回通道的连接管理标志是否为 '暂停重连'
       | __MdsAsyncApi_GetChannelSuspendExpirationTime  | 返回通道暂停重连的过期时间

### 2. 服务端更新概要

  1. 完善对全面注册制改革的支持
  2. 其他细节完善和优化

### 3. 注意事项

  * 无


---

MDS_0.17.6.8 / 2023-02-09
-----------------------------------

### 1. API更新概要

  1. 服务端兼容 v0.15.5.1 版本API, 客户可以选择不升级 (建议升级至当前版本)
  2. 为适配全面注册制改革, 调整相关注释及描述信息
    - 调整 上证委托类型(eOesOrdTypeShT) 中部分委托类型的描述
    - 调整 现货产品基础信息(OesStockBaseInfoT) 中部分字段的描述
    - 调整 证券发行基础信息(OesIssueBaseInfoT) 中部分字段的描述

### 2. 服务端更新概要

  1. 支持全面注册制改革相关改造
  2. 完善对上交所增量快照的更新处理

### 3. 注意事项

  * 无


---

MDS_0.17.6.7 / 2022-11-23
-----------------------------------

### 1. API更新概要

  1. 服务端兼容 v0.15.5.1 版本API, 客户可以选择不升级 (建议升级至当前版本)
  2. 为支持将登录密码配置为加密后的密文密码, 而添加如下接口和工具
     - 增加如下辅助接口
       | API                                   | 描述
       | ------------------------------------- | ---------------
       | MdsApi_SetLocalSecretKey              | 设置客户端本地安全密钥 (对当前进程生效, 用于解密密文密码)
       | MdsApi_ResetLocalSecretKey            | 重置(清空)客户端本地安全密钥 (对当前进程生效)
       | MdsApi_GetThreadPasswordEncryptType   | 返回当前线程登录MDS时使用的登录密码的加密类型
       | MdsHelper_GenerateCiphertextPassword  | 生成加密后的密文密码字符串

     - 增加生成密文密码的工具: mds_tester 工具中增加生成密文密码的指令 generate-ciphertext-password (pwd) 指令, 以方便客户使用
     - 增加生成密文密码的样例代码: samples/mds_sample/10_mds_generate_ciphertext_password_sample.c

### 2. 服务端更新概要

  1. 调整快照行情的去重和过滤处理，弱化对快照行情时间等数据的依赖，以提高容错性
  2. 废弃逐笔数据过期时间 (tickExpireType) 行情订阅参数 (API保持兼容, 但将忽略该参数, 不再生效)
  3. 其他细节完善和优化

### 3. 注意事项

  1. 废弃逐笔数据过期时间 (tickExpireType) 行情订阅参数 (API保持兼容, 但将忽略该参数, 不再生效)
  2. 为支持将登录密码配置为加密后的密文密码, 而扩展配置结构 (SGeneralClientAddrInfoT, SGeneralClientRemoteCfgT) 中 password 字段的长度:
     - password 字段长度由 GENERAL_CLI_MAX_PWD_LEN (40字节) 扩展为 GENERAL_CLI_MAX_PWD_BASE64_LEN (128字节)
     - @note C/C++ API接口保持兼容, 使用其他编程语言(如Python)封装API接口的客户须留意此变动的影响


---

MDS_0.17.6.6 / 2022-11-07
-----------------------------------

### 1. API更新概要

  1. 服务端兼容 v0.15.5.1 版本API, 客户可以选择不升级 (建议升级至当前版本)
  2. fix: 修正发送测试请求消息接口不支持查询通道的问题
  3. fix: 消除API接口中隐藏的风险及隐患
     - 修复 '测试文件路径是否可写函数' 存在BUG，导致初始化日志时可能误判日志路径不可写进而导致API初始化失败的问题
     - 修复当优先使用大页内存时异步API释放大页内存失败的问题
  4. 新增逐笔数据重传的API接口, 客户端可以通过该接口回补缺失的逐笔数据
    | API                                       | 描述
    | ----------------------------------------- | ---------------
    | MdsAsyncApi_SendTickResendRequest         | 发送逐笔数据重传请求 (异步接口)
    | MdsAsyncApi_SendTickResendRequest2        | 发送逐笔数据重传请求 (异步接口)
    | MdsAsyncApi_SendTickResendRequestHugely   | 发送超大的逐笔数据重传请求 (异步接口)
    | MdsApi_SendTickResendRequest              | 发送逐笔数据重传请求 (同步接口)
    | MdsApi_SendTickResendRequest2             | 发送逐笔数据重传请求 (同步接口)
    | MdsApi_SendTickResendRequestHugely        | 发送超大的逐笔数据重传请求 (同步接口)

    @note `MdsAsyncApi_SendTickResendRequestHugely` 和 `MdsApi_SendTickResendRequestHugely` 支持不限制大小的重传请求, 请谨慎使用该接口, 避免发起过大或不必要的重传请求
  5. 支持上交所逐笔合并数据, 调整和增加如下枚举类型定义
     - 调整 '消息来源(`eMdsMsgSourceT`)' 枚举类型定义, 增加如下消息来源, 以方便区分逐笔合并数据:
        - SSE-VDE-逐笔合并数据(`MDS_MSGSRC_VDE_MERGED_TICK`)
        - SSE-VDE-逐笔合并数据-逐笔重建(`MDS_MSGSRC_VDE_MERGED_TICK_REBUILD`)
     - 增加 '上交所产品状态订单的逐笔委托买卖方向字段取值(`eMdsL2SseStatusOrderSideT`)' 枚举类型定义
  6. 新增 '逐笔频道心跳消息', 以方便下游系统检查和识别逐笔行情消息是否发生丢失
     - 新增对应的结构体定义: 'Level2 逐笔频道心跳消息(`MdsTickChannelHeartbeatT`)'
     - 新增行情消息类型定义: 'Level2 逐笔频道心跳消息(`MDS_MSGTYPE_L2_TICK_CHANNEL_HEARTBEAT`)'
     - 新增可订阅的数据种类: '逐笔频道心跳消息(`MDS_SUB_DATA_TYPE_L2_TICK_CHANNEL_HEARTBEAT`)'
  7. 异步API中增加修改客户端登录密码接口
    | API                                       | 描述
    | ----------------------------------------- | ---------------
    | MdsAsyncApi_SendChangePasswordReq         | 发送密码修改请求 (修改客户端登录密码)
  8. 增加辅助的异步API接口
    | API                                       | 描述
    | ----------------------------------------- | ---------------
    | MdsAsyncApi_SubscribeNothingOnConnect     | 提供登录后不订阅任何行情数据的快捷方法
  9. 调整逐笔委托/逐笔成交下的 ApplSeqNum 字段类型, 由 int32 调整为 uint32, 以更好的兼容上交所逐笔合并数据

### 2. 服务端更新概要

  1. 新增逐笔数据重传功能
  2. 支持上交所逐笔合并数据
  3. 支持上交所Lv2行情的IOPV高精度字段改造
  4. 其他细节完善和优化

### 3. 注意事项

  1. 新增逐笔频道心跳消息, 使用默认订阅数据种类(dataTypes=0)时会收到此消息, 客户端可酌情使用或者忽略此消息 (历史版本API不会收到此消息)
  2. 逐笔委托/逐笔成交下的 ApplSeqNum 字段类型由 int32 调整为 uint32, 此调整可能会引起编译警告


---

MDS_0.17.6.3_u5 / 2022-11-07
-----------------------------------

### 1. API更新概要

  1. 服务端兼容 v0.15.5.1 版本API, 客户可以选择不升级 (建议升级至当前版本)
  2. fix: 消除API接口中隐藏的风险及隐患
     - 修复 '测试文件路径是否可写函数' 存在BUG，导致初始化日志时可能误判日志路径不可写进而导致API初始化失败的问题
     - 修复当优先使用大页内存时异步API释放大页内存失败的问题
  3. 异步API中增加修改客户端登录密码接口
    | API                                       | 描述
    | ----------------------------------------- | ---------------
    | MdsAsyncApi_SendChangePasswordReq         | 发送密码修改请求 (修改客户端登录密码)
  4. 增加辅助的异步API接口
    | API                                       | 描述
    | ----------------------------------------- | ---------------
    | MdsAsyncApi_SubscribeNothingOnConnect     | 提供登录后不订阅任何行情数据的快捷方法

### 2. 服务端更新概要

  1. 细节完善和优化

### 3. 注意事项

  * 无


---

MDS_0.17.6.3_u1 / 2022-06-28
-----------------------------------

### 1. API更新概要

  1. 服务端兼容 v0.15.5.1 版本API, 客户可以选择不升级 (建议升级至当前版本)
  2. 支持期权扩位简称, 查询期权合约静态信息应答消息 (`MdsOptionStaticInfoT`) 中
    - '期权合约名称(`securityName`)' 最大长度由C40调整至C56

### 2. 服务端更新概要

  1. 细节完善和优化

### 3. 注意事项

  1. 因交易所期权扩位简称变更, 查询期权合约静态信息接口的应答消息(`MdsOptionStaticInfoT`) 结构体定义有变动。C/C++ API接口保持兼容, 使用其他编程语言(如Python)封装API接口的客户须留意此变动的影响


---

MDS_0.17.6.2 / 2022-05-26
-----------------------------------

### 1. API更新概要

  1. 服务端兼容 v0.15.5.1 版本API, 客户可以选择不升级 (建议升级至当前版本)
  2. 新增不依赖配置文件的日志初始化接口
    | API                                   | 描述
    | ------------------------------------- | ---------------
    | MdsApi_InitLoggerDirect               | 直接通过指定的参数初始化日志记录器

  3. 新增辅助的异步API配置项和配置接口
    | API                                        | 描述
    | ------------------------------------------ | ---------------
    | MdsAsyncApi_SetAsyncConnectAble            | 设置是否启动独立的连接管理线程来执行连接处理和OnConnect回调处理
    | MdsAsyncApi_IsAsyncConnectAble             | 返回是否启动独立的连接管理线程来执行连接处理和OnConnect回调处理
    | MdsAsyncApi_LoadCpusetCfg2                 | 从配置文件中加载CPU亲和性配置 (额外增加对连接管理线程的支持)
    | MdsAsyncApi_SetConnectThreadCpusetCfg      | 设置连接管理线程的CPU亲和性配置
    | MdsAsyncApi_GetConnectThreadCpusetCfg      | 返回连接管理线程的CPU亲和性配置信息
    | MdsAsyncApi_IsBuiltinQueryChannelConnected | 返回内置的查询通道是否已连接就绪

  4. 新增异步API连接回调接口
    | API                                   | 描述
    | ------------------------------------- | ---------------
    | MdsAsyncApi_SetOnConnect              | 设置连接或重新连接完成后的回调函数
    | MdsAsyncApi_GetOnConnect              | 返回连接或重新连接完成后的回调函数
    | MdsAsyncApi_SetOnDisconnect           | 设置连接断开后的回调函数
    | MdsAsyncApi_GetOnDisconnect           | 返回连接断开后的回调函数
    | MdsAsyncApi_SetOnConnectFailed        | 设置连接失败时的回调函数
    | MdsAsyncApi_GetOnConnectFailed        | 返回连接失败时的回调函数

  5. 调整 '上交所逐笔委托的订单类型 (`eMdsL2SseOrderTypeT`)' 枚举类型定义, 增加订单类型:
    - 产品状态订单 (`MDS_L2_SSE_ORDER_TYPE_STATUS`)

### 2. 服务端更新概要

  1. 支持在盘前修改客户端交易密码 (依赖于券商的配置)
  2. 其它细节完善和优化

### 3. 注意事项

  * 无


---

MDS_0.17.5.8 / 2022-05-13
-----------------------------------

### 1. API更新概要

  1. 服务端兼容 v0.15.5.1 版本API, 客户可以选择不升级 (建议升级至当前版本)
  2. 调整 'Level-1快照行情 (`MdsStockSnapshotBodyT`)' 结构体定义, 增加如下深交所债券行情专用字段:
     - 深交所债券加权平均价 (`BondWeightedAvgPx`), 仅适用于深交所质押式回购及债券现券交易产品, 表示质押式回购成交量加权平均利率及债券现券交易成交量加权平均价
     - 深交所债券匹配成交的最近成交价 (`BondAuctionTradePx`), 仅适用于深交所债券现券交易产品
     - 深交所债券匹配成交的成交总量 (`BondAuctionVolumeTraded`), 仅适用于深交所债券现券交易产品
  3. 调整 '行情数据类别 (`eMdsSubStreamTypeT`)' 枚举类型定义, 增加行情数据类别:
     - 深交所债券现券交易匹配成交大额逐笔行情 (`MDS_SUB_STREAM_TYPE_BOND_BLOCK_TRADE`, 仅适用于深交所逐笔行情)

### 2. 服务端更新概要

  1. 完善对深交所债券现券交易业务的支持
  2. 其他细节完善和优化

### 3. 注意事项

  * 无


---

MDS_0.17.5.7 / 2022-04-02
-----------------------------------

### 1. API更新概要

  * 无

### 2. 服务端更新概要

  1. 细节完善和优化

### 3. 注意事项

  * 无


---

MDS_0.17.5.6 / 2022-01-14
-----------------------------------

### 1. API更新概要

  1. 服务端兼容 v0.15.5.1 版本API, 客户可以选择不升级 (建议升级至当前版本)
  2. 逐笔委托数据中增加上交所新债券'产品状态订单'行情数据

### 2. 服务端更新概要

  1. 增加对上交所新债券逐笔行情中的'产品状态订单'的转发处理
  2. 其他细节完善和优化

### 3. 注意事项

  1. 逐笔委托数据中将会新增上交所新债券'产品状态订单'行情数据
    - 对应的 MsgType=MDS_MSGTYPE_L2_SSE_ORDER (Level2 上交所逐笔委托行情)
    - 对应的 OrderType='S', 且Side 字段取值含义如下:
        - 'A': ADD – 产品未上市
        - 'S': START – 启动
        - 'O': OCALL – 开市集合竞价
        - 'T': TRADE – 连续自动撮合
        - 'P': SUSP – 停牌
        - 'C': CLOSE – 闭市
        - 'E': ENDTR – 交易结束


---

MDS_0.17.5.4 / 2021-11-23
-----------------------------------

### 1. API更新概要

  * 无

### 2. 服务端更新概要

  1. 细节完善和优化

### 3. 注意事项

  * 无


---

MDS_0.17.5.3 / 2021-11-05
-----------------------------------

### 1. API更新概要

  1. 服务端兼容 v0.15.5.1 版本API, 客户可以选择不升级 (建议升级至当前版本)
  2. fix: 修复当同时订阅的证券代码数量超过1000时，没有对订阅模式做正确拆分处理的问题

### 2. 服务端更新概要

  1. 细节完善和优化

### 3. 注意事项

  * 无


---

MDS_0.17.4.8 / 2022-05-13
-----------------------------------

### 1. API更新概要

  1. 服务端兼容 v0.15.5.1 版本API, 客户可以选择不升级 (建议升级至当前版本)
  2. 调整 'Level-1快照行情 (`MdsStockSnapshotBodyT`)' 结构体定义, 增加如下深交所债券行情专用字段:
     - 深交所债券加权平均价 (`BondWeightedAvgPx`), 仅适用于深交所质押式回购及债券现券交易产品, 表示质押式回购成交量加权平均利率及债券现券交易成交量加权平均价
     - 深交所债券匹配成交的最近成交价 (`BondAuctionTradePx`), 仅适用于深交所债券现券交易产品
     - 深交所债券匹配成交的成交总量 (`BondAuctionVolumeTraded`), 仅适用于深交所债券现券交易产品
  3. 调整 '行情数据类别 (`eMdsSubStreamTypeT`)' 枚举类型定义, 增加行情数据类别:
     - 深交所债券现券交易匹配成交大额逐笔行情 (`MDS_SUB_STREAM_TYPE_BOND_BLOCK_TRADE`, 仅适用于深交所逐笔行情)

### 2. 服务端更新概要

  1. 完善对深交所债券现券交易业务的支持
  2. 其他细节完善和优化

### 3. 注意事项

  * 无


---

MDS_0.17.4.7 / 2022-04-02
-----------------------------------

### 1. API更新概要

  * 无

### 2. 服务端更新概要

  1. 细节完善和优化

### 3. 注意事项

  * 无


---

MDS_0.17.4.6 / 2022-01-14
-----------------------------------

### 1. API更新概要

  1. 服务端兼容 v0.15.5.1 版本API, 客户可以选择不升级 (建议升级至当前版本)
  2. 逐笔委托数据中增加上交所新债券'产品状态订单'行情数据

### 2. 服务端更新概要

  1. 增加对上交所新债券逐笔行情中的'产品状态订单'的转发处理
  2. 其他细节完善和优化

### 3. 注意事项

  1. 逐笔委托数据中将会新增上交所新债券'产品状态订单'行情数据
    - 对应的 MsgType=MDS_MSGTYPE_L2_SSE_ORDER (Level2 上交所逐笔委托行情)
    - 对应的 OrderType='S', 且Side 字段取值含义如下:
        - 'A': ADD – 产品未上市
        - 'S': START – 启动
        - 'O': OCALL – 开市集合竞价
        - 'T': TRADE – 连续自动撮合
        - 'P': SUSP – 停牌
        - 'C': CLOSE – 闭市
        - 'E': ENDTR – 交易结束


---

MDS_0.17.4.3 / 2021-11-08
-----------------------------------

### 1. API更新概要

  1. 服务端兼容 v0.15.5.1 版本API, 客户可以选择不升级 (建议升级至当前版本)
  2. fix: 修复当同时订阅的证券代码数量超过1000时，没有对订阅模式做正确拆分处理的问题

### 2. 服务端更新概要

  1. 细节完善和优化

### 3. 注意事项

  * 无


---

MDS_0.17.4.2 / 2021-10-18
-----------------------------------

### 1. API更新概要

  * 无

### 2. 服务端更新概要

  1. 细节完善和优化

### 3. 注意事项

  * 无


---

MDS_0.17.4.1 / 2021-09-24
-----------------------------------

### 1. API更新概要

  1. 服务端兼容 v0.15.5.1 版本API, 客户可以选择不升级
  2. fix: 尝试修复Windows平台下偶然发生一直检测不到网络连接异常的问题

### 2. 服务端更新概要

  1. 细节完善和优化

### 3. 注意事项

  * 无


---

MDS_0.17.3.1 / 2021-08-26
-----------------------------------

### 1. API更新概要

  1. 服务端兼容 v0.15.5.1 版本API, 客户可以选择不升级
     - 内部字段有调整, 如果使用了内部字段 '内部频道号 `__channelNo`' 和 '行情数据的更新版本号 `__dataVersion`', 则需要升级到最新版本
     - 消息来源 `eMdsMsgSourceT` 枚举类型的取值有调整, 如果使用到该枚举类型, 也需要升级到最新版本
  2. 完成上交所和深交所债券行情改造, 并梳理和重构相关行情消息定义 (涉及债券行情新增字段、行情数据类别、行情来源和频道号等)
     - 增加枚举类型定义:
        - 新增 '行情数据类别 `eMdsSubStreamTypeT`' 枚举类型定义, 用于标识具体行情数据类别 (如: 股票、债券、基金、指数、期权等)
     - 删除枚举类型定义:
        - 废弃原 '行情数据类型 `eMdsMdStreamTypeT`' 枚举类型定义
            - 判断消息类型, 请直接使用快照行情相关的消息类型定义 `@see eMdsMsgTypeT`
            - 判断行情类别, 请使用行情数据类别 `subStreamType` 字段 `@see eMdsSubStreamTypeT`
     - 调整枚举类型定义
        - 调整 '消息来源 `eMdsMsgSourceT`' 的枚举类型取值 (@note 取值发生变化)
     - 调整快照行情结构体定义
        - 调整 '快照行情消息头 `MdsMktDataSnapshotHeadT`' 结构体定义
            - 重命名 '行情数据类型 `mdStreamType`' 字段 => '快照数据对应的消息类型 `bodyType`'
            - 添加字段 '行情数据类别 `subStreamType`', 用于标识具体行情数据类别 (如: 股票、债券、基金、指数、期权等)
            - 调整内部字段 '内部频道号 `__channelNo:uint8`' 的定义顺序, 并重新定义为 => '频道代码 `__channelNo:uint16`'
            - 调整内部字段 '行情数据的更新版本号 `__dataVersion:uint32`' 的定义顺序, 并调整数据类型 => `uint16`
        - 调整 'Lv2快照行情消息体 `MdsL2StockSnapshotBodyT`' 结构体定义, 增加如下债券行情专用字段:
            - 债券加权平均价 `BondWeightedAvgPx`, 适用于质押式回购及债券现券交易产品, 表示质押式回购成交量加权平均利率及债券现券交易成交量加权平均价
            - 深交所债券匹配成交的最近成交价 `BondAuctionTradePx`, 仅适用于深交所债券现券交易产品
            - 深交所债券匹配成交的成交总量 `BondAuctionVolumeTraded`, 仅适用于深交所债券现券交易产品
            - 深交所债券匹配成交的成交总金额 `BondAuctionValueTraded`, 仅适用于深交所债券现券交易产品
     - 调整 '逐笔成交 `MdsL2TradeT`' 和 '逐笔委托 `MdsL2OrderT`' 结构体定义
        - 调整字段 '频道代码 `ChannelNo:int32`' 的数据类型 => `uint16`
        - 添加字段 '行情数据类别 `subStreamType`', 用于标识具体行情数据类别 (如: 股票、债券、基金等)
        - 删除内部字段 '内部频道号 `__channelNo`'
     - 调整 '市场状态 `MdsTradingSessionStatusMsgT`' 和 '证券实时状态 `MdsSecurityStatusMsgT`' 结构体中的内部字段定义
        - 调整内部字段 '行情数据的更新版本号 `__dataVersion:uint32`' 的数据类型 => `uint16`
        - 删除内部字段 '内部频道号 `__channelNo`'
  3. 证券静态信息 `MdsStockStaticInfoT` 中添加字段:
     - 添加字段 '计价方式 `pricingMethod`', 目前仅适用于债券现券交易产品

### 2. 服务端更新概要

  1. 支持上交所(新债券交易系统)和深交所(债券现券交易业务)的债券行情改造
  2. 其它细节完善和优化

### 3. 注意事项

  1. 以下枚举类型的取值或定义发生调整, 如果使用到, 需要升级到最新版本
     - 消息来源 `eMdsMsgSourceT` 枚举类型的取值有调整
     - 行情数据类型 `eMdsMdStreamTypeT` 枚举类型已经废弃
        - 判断消息类型, 请直接使用快照行情相关的消息类型定义 `@see eMdsMsgTypeT`
        - 判断行情类别, 请使用行情数据类别 `subStreamType` 字段 `@see eMdsSubStreamTypeT`
  2. 以下内部字段发生调整, 如果使用到, 需要升级到最新版本
     - 调整内部字段 '行情数据的更新版本号 `__dataVersion:uint32`' 的数据类型 => `uint16`
     - 删除内部字段 '内部频道号 `__channelNo:uint8`' (快照行情将该字段重新定义为 => '频道代码 `__channelNo:uint16`')
  3. Lv2快照行情 `MdsL2StockSnapshotBodyT` 中新增的债券行情字段通过 `union` 的方式复用了仅适用于基金、期权或上交所的行情字段, 使用时需要按照行情数据类别或交易所代码进行区分
     - 债券加权平均价 `BondWeightedAvgPx` 字段可以根据 '行情数据类别 `subStreamType`' 进行区分, 例如:

     ~~~{.c}
        - IOPV = pSnapshot->head.subStreamType != MDS_SUB_STREAM_TYPE_BOND ? pSnapshot->l2Stock.IOPV : 0;
        - BondWeightedAvgPx = pSnapshot->head.subStreamType != MDS_SUB_STREAM_TYPE_BOND ? 0 : pSnapshot->l2Stock.BondWeightedAvgPx;
     ~~~

     - 深交所债券匹配成交的最近成交价 `BondAuctionTradePx`、深交所债券匹配成交的成交总量 `BondAuctionVolumeTraded`、深交所债券匹配成交的成交总金额 `BondAuctionValueTraded` 这三个字段可以根据交易所代码进行区分, 例如:

     ~~~{.c}
        - BondAuctionTradePx = pSnapshot->head.exchId == MDS_EXCH_SZSE ? pSnapshot->l2Stock.BondAuctionTradePx : 0;
        - BondAuctionVolumeTraded = pSnapshot->head.exchId == MDS_EXCH_SZSE ? pSnapshot->l2Stock.BondAuctionVolumeTraded : 0LL;
        - BondAuctionValueTraded = pSnapshot->head.exchId == MDS_EXCH_SZSE ? pSnapshot->l2Stock.BondAuctionValueTraded : 0LL;
     ~~~


---

MDS_0.17.2.1 / 2021-08-09
-----------------------------------

### 1. API更新概要

  1. 服务端兼容 v0.15.5.1 版本API, 客户可以选择不升级

### 2. 服务端更新概要

  1. 细节完善和优化

### 3. 注意事项

  * 无


---

MDS_0.17.1.3 / 2021-07-16
-----------------------------------

### 1. API更新概要

  1. 服务端兼容 v0.15.5.1 版本API, 客户可以选择不升级

### 2. 服务端更新概要

  1. 细节完善和优化

### 3. 注意事项

  * 无


---

MDS_0.16.2.1 / 2021-04-20
-----------------------------------

### 1. API更新概要

  1. 服务端兼容 v0.15.5.1 版本API, 客户可以选择不升级
  2. fix: 在上海增量快照中增加最高价/最低价字段，以修复可能会丢失增量快照中的最高价/最低价变化的问题
  3. fix: 增加对Windows下的CPU绑定操作的支持, 并完善Windows下的进程号、线程号处理
  4. 增加对上交所逐笔委托行情的支持:
     - 增加行情消息类型: Level2 上交所逐笔委托行情 (MDS_MSGTYPE_L2_SSE_ORDER, 仅适用于上交所)
     - 增加可订阅的数据种类: 上交所逐笔委托 (MDS_SUB_DATA_TYPE_L2_SSE_ORDER, 仅适用于上交所)
  5. 为异步API增加内置的查询通道，并整合查询通道管理和查询接口到异步API中
     - 增加查询通道相关的配置接口
       | API                                   | 描述 |
       | --------------------------------------| -------------------- |
       | MdsAsyncApi_SetBuiltinQueryable       | 设置是否启用内置的查询通道 (即启动异步API时自动创建一个内置的查询通道) |
       | MdsAsyncApi_IsBuiltinQueryable        | 返回是否启用内置的查询通道 |
       | MdsAsyncApi_SetBuiltinQueryChannelCfg | 设置内置的查询通道的配置信息 |
       | MdsAsyncApi_LoadBuiltinQueryChannelCfg| 从配置文件中加载内置的查询通道的配置信息 |
       | MdsAsyncApi_GetBuiltinQueryChannelCfg | 返回内置的查询通道的配置信息 |
       | MdsAsyncApi_GetBuiltinQueryChannelRef | 返回内置的查询通道的会话信息 |

     - 增加异步API查询接口
       | API                                   | 描述 |
       | --------------------------------------| -------------------- |
       | MdsAsyncApi_QueryMktDataSnapshot      | 查询证券行情快照 |
       | MdsAsyncApi_QuerySnapshotList         | 批量查询行情快照 |
       | MdsAsyncApi_QuerySnapshotList2        | 批量查询行情快照 |
       | MdsAsyncApi_QuerySecurityStatus       | 查询(深圳)证券实时状态 |
       | MdsAsyncApi_QueryTrdSessionStatus     | 查询(上证)市场状态 |
       | MdsAsyncApi_QueryStockStaticInfoList  | 批量查询证券(股票/债券/基金)静态信息列表 |
       | MdsAsyncApi_QueryStockStaticInfoList2 | 批量查询证券(股票/债券/基金)静态信息列表 |

     - 样例代码参见: samples/mds_sample/01_mds_async_tcp_sample.c

  6. 增加辅助的异步API接口, 以支持对通信线程、回调线程等异步API线程进行初始化处理
       | API                                       | 描述 |
       | ------------------------------------------| -------------------- |
       | MdsAsyncApi_SetOnCommunicationThreadStart | 设置通信线程的线程初始化回调函数 |
       | MdsAsyncApi_SetOnCallbackThreadStart      | 设置回调线程的线程初始化回调函数 |
       | MdsAsyncApi_SetOnIoThreadStart            | 设置异步I/O线程的线程初始化回调函数 |

  7. 增加辅助的行情订阅和会话管理接口
       | API                                       | 描述 |
       | ------------------------------------------| -------------------- |
       | MdsAsyncApi_DefaultOnConnect              | 连接完成后处理的默认实现 (执行默认的行情订阅处理) |
       | MdsAsyncApi_SubscribeByQuery              | 查询证券静态信息并根据查询结果订阅行情信息 (异步API) |
       | MdsApi_SubscribeByQuery                   | 查询证券静态信息并根据查询结果订阅行情信息 (同步API) |
       | MdsApi_GetClientId                        | 返回通道对应的客户端编号 |
       | MdsHelper_AddSubscribeRequestEntry2       | 添加待订阅产品到订阅信息中 (没有数量限制) |
  8. 调整UDP行情组播的心跳间隔时间为: 10秒 (如果超过3倍心跳时间没有收到任何组播消息, 就可以认为组播链路有故障)
  9. 增加对组播地址连接的高可用处理, 为异步API增加组播地址的自动切换处理 (连接异常时自动尝试下一个组播地址), 为同步API增加相应的辅助接口
       | API                                   | 描述 |
       | --------------------------------------| -------------------- |
       | MdsApi_SetUdpReconnectFromNextAddrAble| 设置重建连接组播通道时是否从下一个地址开始尝试 |
       | MdsApi_IsUdpReconnectFromNextAddrAble | 返回重建连接组播通道时是否从下一个地址开始尝试 |

### 2. 服务端更新概要

  1. 修复可能会丢失上海增量快照中的最高价/最低价变化的问题
  2. 修复个别场景下当委托数量没有变化时会遗漏上证L2快照中的‘价位总委托笔数’变化的问题
  3. 增加对上交所逐笔委托行情的支持
  4. 其它细节完善和优化

### 3. 注意事项

  1. 如果使用的是旧版本的API, 那么服务器端将不再推送上交所Level2快照的增量更新消息, 将只推送全量快照。如果需要使用增量更新消息, 就需要升级到最新版本。
  2. 逐笔成交和逐笔委托结构体中有两个内部使用的字段发生变化:
     - 调整内部字段 '内部频道号 (__channelNo)' 的定义顺序, 如果使用了该字段就需要升级到最新版本；
     - 删除内部字段 '对应的原始行情的序列号 (__origTickSeq)', 该字段没有业务含义通常不会被使用到, 如果使用了该字段, 需要修改或删除相关代码。


---

MDS_0.16.1.9 / 2020-11-20
-----------------------------------

### 1. API更新概要

  1. 服务端兼容 v0.15.5.1 版本API, 客户可以选择不升级
    - 如果使用到了时间戳字段下的微秒时间, 则至少需要升级到v0.16.1.2版本。具体请参见 v0.16.1.2 版本的更新说明
  2. 证券静态信息 (MdsStockStaticInfoT) 中新增证券子类型:
    - 新增 基础设施基金(OES_SUB_SECURITY_TYPE_FUND_REITS)
  3. 增加辅助接口
    | API                           | 描述 |
    | ------------------------------| -------------------- |
    | MdsApi_GetClientType          | 返回通道对应的客户端类型 |
    | MdsApi_GetClientStatus        | 返回通道对应的客户端状态 |
    | MdsApi_SetRetainExtDataAble   | 设置是否保留(不清空)由应用层自定义使用的扩展存储空间数据 (__extData) |
    | MdsApi_IsRetainExtDataAble    | 返回是否保留(不清空)由应用层自定义使用的扩展存储空间数据 |

### 2. 服务端更新概要

  1. 无


---

MDS_0.16.1.7 / 2020-09-30
-----------------------------------

### 1. API更新概要

  1. 服务端兼容 v0.15.5.1 版本API, 客户可以选择不升级 (建议升级)
    - 如果使用到了时间戳字段下的微秒时间, 则至少需要升级到v0.16.1.2版本。具体请参见 v0.16.1.2 版本的更新说明
  2. 新增 证券属性定义(eOesSecurityAttributeT) 枚举类型
  3. '证券静态信息(MdsStockStaticInfoT)' 中启用 证券属性 (securityAttribute) 字段

### 2. 服务端更新概要

  1. 无


---

MDS_0.16.1.4 / 2020-08-28
-----------------------------------

### 1. API更新概要

  1. 服务端兼容 v0.15.5.1 版本API, 客户可以选择不升级 (建议升级)
     - 如果使用到了时间戳字段下的微秒时间, 则需要升级最新的API, 因为时间戳字段已全部升级为纳秒级时间戳。具体请参见 v0.16.1.2 版本的更新说明
  2. fix: 完备Windows平台下的WSACleanup资源释放处理，避免额外调用WSACleanup导致外部系统的网络操作异常
  3. fix: 修复Win64下不能正确获取纳秒级时间戳的问题
  4. fix: 修复MinGW下 struct timespec 结构体未按64位对齐的问题
  5. fix: 将行情API中的默认集群类型调整为基于复制集的高可用集群, 避免误连接到行情备机
  6. 为异步API增加用于返回尚未被处理的剩余数据数量的辅助接口
    | API                                     | 描述 |
    | ----------------------------------------| -------------------- |
    | MdsAsyncApi_GetAsyncQueueTotalCount     | 返回异步API累计已入队的消息数量 |
    | MdsAsyncApi_GetAsyncQueueRemainingCount | 返回队列中尚未被处理的剩余数据数量 |

### 2. 服务端更新概要

  1. 完善系统在异常场景下24小时持续运行的可靠性
  2. fix: 修复会丢弃部分上海时间戳相同的全量快照 (导致丢失一段时间的最高价/最低价变化), 以及部分时间戳相同但开盘价等有变化的增量快照 (导致丢失一段时间的开盘价信息) 的问题
  3. fix: 进一步重构和完善对上海快照的去重和合并处理, 修复会丢失部分上海最优委托队列和指数快照的问题
  4. fix: 忽略上海L1行情中的综合业务行情数据
  5. 其它细节完善和优化


---

MDS_0.16.1.2 / 2020-07-07
-----------------------------------

### 1. API更新概要

  1. 服务端兼容 v0.15.5.1 版本API (建议升级, 以支持期权业务)
     - 如果使用到了时间戳字段下的微秒时间, 则需要升级到最新版本, 因为时间戳字段已全部升级为纳秒级时间戳。具体请参见注意事项说明
  2. fix: 修复在Win32下因为对齐问题导致指针位置不正确的BUG (当启用异步API的异步回调处理时会触发)
  3. 异步API新增如下接口
    | API                                | 描述 |
    | -----------------------------------| -------------------- |
    | MdsAsyncApi_IsAllTerminated        | 检查所有线程是否均已安全退出 |
  4. 同步API新增如下接口
    | API                                | 描述 |
    | -----------------------------------| -------------------- |
    | MdsApi_QueryOptionStaticInfoList   | 批量查询期权合约静态信息列表 |
    | MdsApi_QueryOptionStaticInfoList2  | 批量查询期权合约静态信息列表 (字符串指针数组形式的期权代码列表) |
    | MdsApi_QueryStockStaticInfoList    | 批量查询证券(股票/债券/基金)静态信息列表 |
    | MdsApi_QueryStockStaticInfoList2   | 批量查询证券(股票/债券/基金)静态信息列表 (字符串指针数组形式的证券代码列表) |
    | MdsApi_GetChannelGroupLastRecvTime | 返回通道组最近接收消息时间 |
    | MdsApi_GetChannelGroupLastSendTime | 返回通道组最近发送消息时间 |
    | MdsApi_HasStockStatus              | 返回现货产品是否具有指定状态|
    | __MdsApi_CheckApiVersion           | 检查API版本是否匹配 (检查API头文件和库文件的版本是否匹配) |
  5. 为支持创业板注册制改革, 证券静态信息 (MdsStockStaticInfoT) 中新增如下字段:
    | 字段                        | 描述 |
    | -------------------------- | -------------------- |
    | isRegistration             | 是否注册制 |
    | currType                   | 币种 |
    | qualificationClass         | 投资者适当性管理分类 |
    | securityStatus             | 证券状态 |
    | securityAttribute          | 证券属性 (保留字段) |
    | suspFlag                   | 连续停牌标识 |
    | isDayTrading               | 是否支持当日回转交易 |
    | isCrdMarginTradeUnderlying | 是否为融资标的 |
    | isCrdShortSellUnderlying   | 是否为融券标的 |
    | isCrdCollateral            | 是否为融资融券担保品 |
    | isNoProfit                 | 是否尚未盈利 |
    | isWeightedVotingRights     | 是否存在投票权差异 |
    | isVie                      | 是否具有协议控制框架 |
    | lmtBuyQtyUnit              | 限价买入单位 |
    | lmtSellQtyUnit             | 限价卖出单位 |
    | mktBuyQtyUnit              | 市价买入单位 |
    | mktSellQtyUnit             | 市价卖出单位 |
    | parValue                   | 面值 (重命名 parPrice) |
    | auctionLimitType           | 连续交易时段的有效竞价范围限制类型 |
    | auctionReferPriceType      | 连续交易时段的有效竞价范围基准价类型 |
    | auctionUpDownRange         | 连续交易时段的有效竞价范围涨跌幅度 |
    | listDate                   | 上市日期 |
    | maturityDate               | 到期日期 (仅适用于债券等有发行期限的产品) |
    | underlyingSecurityId       | 基础证券代码 |
    | securityLongName           | 证券长名称 (UTF-8 编码) |
    | securityEnglishName        | 证券英文名称 (UTF-8 编码) |
    | securityIsinCode           | ISIN代码 |
  6. 证券静态信息 (MdsStockStaticInfoT) 中新增证券子类型(同OES):
     - 创业板存托凭证 (OES_SUB_SECURITY_TYPE_STOCK_GEMCDR)
     - 可交换债券 (OES_SUB_SECURITY_TYPE_BOND_EXG)
     - 商品期货ETF (OES_SUB_SECURITY_TYPE_ETF_COMMODITY_FUTURES)
  7. 优化异步API
     - 为异步API增加是否优先使用大页内存来创建异步队列的配置项
     - 为异步API的I/O线程增加追加模式输出的配置项
     - 为异步API的I/O线程增加支持忙等待的配置选项，以使异步队列的延迟统计结果更接近实际情况
  8. 优化时间戳精度, 将系统下的时间戳全部升级为纳秒级时间戳, 以提高时延统计的精度
     - 时间戳字段的数据类型从 STimevalT/STimeval32T 变更为 STimespecT/STimespec32T
     - 协议保持兼容, 但如果使用到了时间戳字段下的微秒时间(tv_usec 字段), 则需要修改为纳秒时间(tv_nsec 字段),
       否则会因为时间单位的调整而导致时延计算错误
  9. API中添加vs2015工程样例

### 2. 服务端更新概要

  1. fix: 更新上海增量快照的去重处理, 以处理存在时间相同但数据不同的增量快照问题
  2. fix: 完善上海增量快照的更新处理, 防止因为数据丢失导致出现买卖盘价格倒挂
  3. fix: 完善由上海增量快照合成的快照消息中，没有更新和携带实际的原始行情的序列号（__origTickSeq）的问题
  4. fix: 修复委托队列增量消息的大小未全部按64位对齐的问题
  5. 支持创业板注册制改革
  6. 其它细节完善和性能优化

### 3. 注意事项说明

  1. 时间戳精度的修改没法完全兼容之前的API, 如果使用到了时间戳字段下的微秒时间(tv_usec 字段), 则需要升级API到最新版本
     - 通常只有在统计延迟时才会使用到微秒时间(tv_usec)字段, 如果使用到了该字段, 则需要改为使用纳秒时间(tv_nsec)字段,
       否则会因为时间单位的调整而导致时延计算错误
     - 升级API以后, 可以通过检查有无编译错误的方式, 来检查是否使用到了 tv_usec 字段。如果没有编译错误就无需再做额外的调整
  2. 上海市场存在更新时间相同但数据不同的Level-2快照, 与通常的预期不太相符。(频率不高, 但会存在这样的数据)


---

MDS_0.16.0.5 / 2020-04-17
-----------------------------------

### 1. API更新概要

  1. 服务端兼容 v0.15.5.1 版本API, 客户可以选择不升级 (建议升级, 以支持期权业务)
  2. 统一沪深期权行情的昨收盘价字段, 不再通过该字段推送昨结算价（深圳期权快照中没有结算价字段）
  3. 调整组播行情的行为, 不再强制过滤掉重复的快照行情和重建到的逐笔行情, 并允许通过订阅条件进行控制
  4. 为异步API增加对UDP行情数据的本地订阅/过滤功能
      - 订阅方式与TCP行情订阅方式相同, 相关订阅接口如下:
         - MdsAsyncApi_SubscribeMarketData
         - MdsAsyncApi_SubscribeByString
         - MdsAsyncApi_SubscribeByStringAndPrefixes
      - 对UDP行情数据的订阅和过滤功能默认不开启, 可通过接口或配置予以控制
      - 样例代码参见:
         - samples/mds_sample/04_mds_async_udp_sample.c
  5. 调整示例代码的目录位置

### 2. 服务端更新概要

  1. 无


---

MDS_0.16.0.4 / 2020-02-28
-----------------------------------

### 1. API更新概要

  1. 服务端兼容 v0.15.5.1 版本API, 客户可以选择不升级 (建议升级, 以支持期权业务)
  2. fix: 为API增加对SIGPIPE信号的屏蔽处理，以避免连接断开以后重复send导致程序异常退出
  3. 重构异步API接口, 统一交易和行情的异步API定义 (不兼容之前版本的异步API)
     - 支持异步接收行情数据
     - 支持自动的连接管理 (自动识别异常并重建连接)
     - 支持异步数据落地和延迟统计 (可以通过配置文件进行控制, 默认为禁用)
     - 主要接口:
        - MdsAsyncApi_CreateContext
        - MdsAsyncApi_AddChannel
        - MdsAsyncApi_Start
        - MdsAsyncApi_Stop
        - MdsAsyncApi_ReleaseContext
    - 样例代码参见: mds_api/samples/02_mds_async_api_sample.c

### 2. 服务端更新概要

  1. fix: 修复因为指令乱序而导致快照行情发生数据重复和数据丢失的风险
  2. 其它细节完善

### 3. 注意事项说明

  1. 异步接口发生了变化, 无法兼容之前版本的异步API, 请参考样例代码进行升级处理。


---

MDS_0.16.0.1 / 2019-12-12
-----------------------------------

### 1. API更新概要

  1. 完善异步API的实现机制及其异常处理

### 2. 服务端更新概要

  1. 完善对无效期权行情的处理
  2. 其它细节完善


---

MDS_0.16 / 2019-11-20
-----------------------------------

### 1. API更新概要

  1. 服务端兼容 v0.15.5.1 版本API, 客户可以选择不升级 (建议升级, 以支持期权业务)
  2. API新增如下接口
    | API                          | 描述
    | ---------------------------- | ---------------
    | MdsApi_QueryOptionStaticInfo | 查询期权静态信息


---

MDS_0.15.12.3 / 2021-08-05
-----------------------------------

### 1. API更新概要

  1. 服务端兼容 v0.15.5.1 版本API, 客户可以选择不升级
  2. fix: 修正查询证券静态信息和快照时，如果指定的证券代码数量较多，会因为自动拆分为多次调用而导致存在多笔数据的 `isEnd` 标志为 `TRUE` 的现象
     (@note 修改后存在如果最后一轮查询结果为空的话, 会导致 `isEnd` 一直都无法被设置上的问题，这个问题需要客户端在查询接口执行完成以后自行判断和处理一下)

### 2. 服务端更新概要

  * 无


---

MDS_0.15.12.2 / 2021-05-28
-----------------------------------

### 1. API更新概要

  1. 服务端兼容 v0.15.5.1 版本API, 客户可以选择不升级
  2. 增加时间同步处理, 计算客户端时钟与服务器端时钟的时间偏差以修正延迟统计信息
  3. 调整API配置文件中的 `tickType` 默认取值为1
  4. 调整UDP行情组播的心跳间隔时间为: 10秒 (如果超过3倍心跳时间没有收到任何组播消息, 就可以认为组播链路有故障)
  5. 增加对组播地址连接的高可用处理, 为异步API增加组播地址的自动切换处理 (连接异常时自动尝试下一个组播地址), 为同步API增加相应的辅助接口

   | API                                   | 描述 |
   | --------------------------------------| -------------------- |
   | MdsApi_SetUdpReconnectFromNextAddrAble| 设置重建连接组播通道时是否从下一个地址开始尝试 |
   | MdsApi_IsUdpReconnectFromNextAddrAble | 返回重建连接组播通道时是否从下一个地址开始尝试 |

### 2. 服务端更新概要

  1. 修复个别场景下当委托数量没有变化时会遗漏上证L2快照中的‘价位总委托笔数’变化的问题
  2. 支持在连接上交所VDE重建服务时也执行登录操作（可配置，用于兼容第三方行情转发程序）
  3. 增加时间同步处理, 计算客户端时钟与服务器端时钟的时间偏差以修正延迟统计信息
  4. 其它细节完善和优化


---

MDS_0.15.12 / 2021-03-23
-----------------------------------

### 1. API更新概要

  1. 服务端兼容 v0.15.5.1 版本API, 客户可以选择不升级
  2. fix: 在上海增量快照中增加最高价/最低价字段，以修复可能会丢失增量快照中的最高价/最低价变化的问题
  3. fix: 增加对Windows下的CPU绑定操作的支持, 并完善Windows下的进程号、线程号处理
  4. 增加对上交所逐笔委托行情的支持:
     - 增加行情消息类型: Level2 上交所逐笔委托行情 (`MDS_MSGTYPE_L2_SSE_ORDER`, 仅适用于上交所)
     - 增加可订阅的数据种类: 上交所逐笔委托 (`MDS_SUB_DATA_TYPE_L2_SSE_ORDER`, 仅适用于上交所)
  5. 为异步API增加内置的查询通道，并整合查询通道管理和查询接口到异步API中
     - 增加查询通道相关的配置接口

       | API                                   | 描述 |
       | --------------------------------------| -------------------- |
       | MdsAsyncApi_SetBuiltinQueryable       | 设置是否启用内置的查询通道 |
       | MdsAsyncApi_IsBuiltinQueryable        | 返回是否启用内置的查询通道 |
       | MdsAsyncApi_SetBuiltinQueryChannelCfg | 设置内置的查询通道的配置信息 |
       | MdsAsyncApi_LoadBuiltinQueryChannelCfg| 从配置文件中加载内置的查询通道的配置信息 |
       | MdsAsyncApi_GetBuiltinQueryChannelCfg | 返回内置的查询通道的配置信息 |
       | MdsAsyncApi_GetBuiltinQueryChannelRef | 返回内置的查询通道的会话信息 |

     - 增加异步API查询接口

       | API                                   | 描述 |
       | --------------------------------------| -------------------- |
       | MdsAsyncApi_QueryMktDataSnapshot      | 查询证券行情快照 |
       | MdsAsyncApi_QuerySnapshotList         | 批量查询行情快照 |
       | MdsAsyncApi_QuerySnapshotList2        | 批量查询行情快照 |
       | MdsAsyncApi_QuerySecurityStatus       | 查询(深圳)证券实时状态 |
       | MdsAsyncApi_QueryTrdSessionStatus     | 查询(上证)市场状态 |
       | MdsAsyncApi_QueryStockStaticInfoList  | 批量查询证券(股票/债券/基金)静态信息列表 |
       | MdsAsyncApi_QueryStockStaticInfoList2 | 批量查询证券(股票/债券/基金)静态信息列表 |

     - 样例代码参见: *`samples/mds_sample/01_mds_async_tcp_sample.c`*

  6. 增加辅助的异步API接口, 以支持对通信线程、回调线程等异步API线程进行初始化处理

    | API                                       | 描述 |
    | ------------------------------------------| -------------------- |
    | MdsAsyncApi_SetOnCommunicationThreadStart | 设置通信线程的线程初始化回调函数 |
    | MdsAsyncApi_SetOnCallbackThreadStart      | 设置回调线程的线程初始化回调函数 |
    | MdsAsyncApi_SetOnIoThreadStart            | 设置异步I/O线程的线程初始化回调函数 |

  7. 增加辅助的行情订阅和会话管理接口

    | API                                       | 描述 |
    | ------------------------------------------| -------------------- |
    | MdsAsyncApi_DefaultOnConnect              | 连接完成后处理的默认实现 (执行默认的行情订阅处理) |
    | MdsAsyncApi_SubscribeByQuery              | 查询证券静态信息并根据查询结果订阅行情信息 (异步API) |
    | MdsApi_SubscribeByQuery                   | 查询证券静态信息并根据查询结果订阅行情信息 (同步API) |
    | MdsApi_GetClientId                        | 返回通道对应的客户端编号 |
    | MdsHelper_AddSubscribeRequestEntry2       | 添加待订阅产品到订阅信息中 (没有数量限制) |

### 2. 服务端更新概要

  1. 修复可能会丢失上海增量快照中的最高价/最低价变化的问题
  2. 增加对上交所逐笔委托行情的支持
  3. 其它细节完善和优化

### 3. 注意事项

  1. 如果使用的是旧版本的API, 那么服务器端将不再推送上交所Level2快照的增量更新消息, 将只推送全量快照。如果需要使用增量更新消息, 就需要升级到最新版本。
  2. 逐笔成交和逐笔委托结构体中有两个内部使用的字段发生变化:
     - 调整内部字段 '内部频道号 `__channelNo`' 的定义顺序, 如果使用了该字段就需要升级到最新版本；
     - 删除内部字段 '对应的原始行情的序列号 `__origTickSeq`', 该字段没有业务含义通常不会被使用到, 如果使用了该字段, 需要修改或删除相关代码。


---

MDS_0.15.11.15 / 2020-11-20
-----------------------------------

### 1. API更新概要

  1. 服务端兼容 v0.15.5.1 版本API, 客户可以选择不升级
  2. 证券静态信息 `MdsStockStaticInfoT` 中新增证券子类型:
    - 新增 基础设施基金 `OES_SUB_SECURITY_TYPE_FUND_REITS`
  3. 增加辅助接口

    | API                           | 描述 |
    | ------------------------------| -------------------- |
    | MdsApi_GetClientType          | 返回通道对应的客户端类型 |
    | MdsApi_GetClientStatus        | 返回通道对应的客户端状态 |
    | MdsApi_SetRetainExtDataAble   | 设置是否保留(不清空)由应用层自定义使用的扩展存储空间数据 (__extData) |
    | MdsApi_IsRetainExtDataAble    | 返回是否保留(不清空)由应用层自定义使用的扩展存储空间数据 |

### 2. 服务端更新概要

  * 无


---

MDS_0.15.11.12 / 2020-09-30
-----------------------------------

### 1. API更新概要

  1. 服务端兼容 v0.15.5.1 版本API, 客户可以选择不升级
  2. 新增 证券属性定义 `eOesSecurityAttributeT` 枚举类型
  3. '证券静态信息 `MdsStockStaticInfoT`' 中启用 证券属性 `securityAttribute` 字段

### 2. 服务端更新概要

  1. fix: 修复会丢弃部分上海时间戳相同的全量快照 (导致丢失一段时间的最高价/最低价变化), 以及部分时间戳相同但开盘价等有变化的增量快照 (导致丢失一段时间的开盘价信息) 的问题
  2. fix: 进一步重构和完善对上海快照的去重和合并处理, 修复会丢失部分上海最优委托队列和指数快照的问题
  3. fix: 忽略上海L1行情中的综合业务行情数据
  4. 其它细节完善和优化


---

MDS_0.15.11.6 / 2020-07-23
-----------------------------------

### 1. API更新概要

  1. 该版本是 v015.11.4 的BUG修复版本, 建议升级到该版本
  2. 服务端兼容 v0.15.5.1 版本API, 客户可以选择不升级 (建议升级)
     - 如果使用到了时间戳字段下的微秒时间, 则需要升级最新的API, 因为时间戳字段已全部升级为纳秒级时间戳。具体请参见 v0.15.11.4 版本的更新说明
  3. fix: 完备Windows平台下的 `WSACleanup` 资源释放处理，避免额外调用 `WSACleanup` 导致外部系统的网络操作异常
  4. fix: 修复Win64下不能正确获取纳秒级时间戳的问题
  5. fix: 修复MinGW下 `struct timespec` 结构体未按64位对齐的问题
  6. fix: 将行情API中的默认集群类型调整为基于复制集的高可用集群, 避免误连接到行情备机
  7. 为异步API增加用于返回尚未被处理的剩余数据数量的辅助接口

    | API                                     | 描述 |
    | ----------------------------------------| -------------------- |
    | MdsAsyncApi_GetAsyncQueueTotalCount     | 返回异步API累计已入队的消息数量 |
    | MdsAsyncApi_GetAsyncQueueRemainingCount | 返回队列中尚未被处理的剩余数据数量 |

### 2. 服务端更新概要

  1. 完善系统在异常场景下24小时持续运行的可靠性
  2. 其它细节完善和优化


---

MDS_0.15.11.4 / 2020-07-07
-----------------------------------

### 1. API更新概要

  1. 服务端兼容 v0.15.5.1 版本API, 客户可以选择不升级 (建议升级以支持创业板注册制改革的相关内容)
     - 如果使用到了时间戳字段下的微秒时间, 则需要升级最新的API, 因为时间戳字段已全部升级为纳秒级时间戳。具体请参见注意事项说明
  2. fix: 修复在Win32下因为对齐问题导致指针位置不正确的BUG (当启用异步API的异步回调处理时会触发)
  3. 异步API新增如下接口

    | API                                | 描述 |
    | -----------------------------------| -------------------- |
    | MdsAsyncApi_IsAllTerminated        | 检查所有线程是否均已安全退出 |
  4. 同步API新增如下接口

    | API                                | 描述 |
    | -----------------------------------| -------------------- |
    | MdsApi_QueryStockStaticInfoList    | 批量查询证券(股票/债券/基金)静态信息列表 |
    | MdsApi_QueryStockStaticInfoList2   | 批量查询证券(股票/债券/基金)静态信息列表 (字符串指针数组形式的证券代码列表) |
    | MdsApi_GetChannelGroupLastRecvTime | 返回通道组最近接收消息时间 |
    | MdsApi_GetChannelGroupLastSendTime | 返回通道组最近发送消息时间 |
    | MdsApi_HasStockStatus              | 返回现货产品是否具有指定状态|
    | __MdsApi_CheckApiVersion           | 检查API版本是否匹配 (检查API头文件和库文件的版本是否匹配) |
  5. 为支持创业板注册制改革, 证券静态信息 `MdsStockStaticInfoT` 中新增如下字段:

    | 字段                        | 描述 |
    | -------------------------- | -------------------- |
    | isRegistration             | 是否注册制 |
    | currType                   | 币种 |
    | qualificationClass         | 投资者适当性管理分类 |
    | securityStatus             | 证券状态 |
    | securityAttribute          | 证券属性 (保留字段) |
    | suspFlag                   | 连续停牌标识 |
    | isDayTrading               | 是否支持当日回转交易 |
    | isCrdMarginTradeUnderlying | 是否为融资标的 |
    | isCrdShortSellUnderlying   | 是否为融券标的 |
    | isCrdCollateral            | 是否为融资融券担保品 |
    | isNoProfit                 | 是否尚未盈利 |
    | isWeightedVotingRights     | 是否存在投票权差异 |
    | isVie                      | 是否具有协议控制框架 |
    | lmtBuyQtyUnit              | 限价买入单位 |
    | lmtSellQtyUnit             | 限价卖出单位 |
    | mktBuyQtyUnit              | 市价买入单位 |
    | mktSellQtyUnit             | 市价卖出单位 |
    | parValue                   | 面值 (重命名 parPrice) |
    | auctionLimitType           | 连续交易时段的有效竞价范围限制类型 |
    | auctionReferPriceType      | 连续交易时段的有效竞价范围基准价类型 |
    | auctionUpDownRange         | 连续交易时段的有效竞价范围涨跌幅度 |
    | listDate                   | 上市日期 |
    | maturityDate               | 到期日期 (仅适用于债券等有发行期限的产品) |
    | underlyingSecurityId       | 基础证券代码 |
    | securityLongName           | 证券长名称 (UTF-8 编码) |
    | securityEnglishName        | 证券英文名称 (UTF-8 编码) |
    | securityIsinCode           | ISIN代码 |
  6. 证券静态信息 `MdsStockStaticInfoT` 中新增证券子类型(同OES):
     - 创业板存托凭证 `OES_SUB_SECURITY_TYPE_STOCK_GEMCDR`
     - 可交换债券 `OES_SUB_SECURITY_TYPE_BOND_EXG`
     - 商品期货ETF `OES_SUB_SECURITY_TYPE_ETF_COMMODITY_FUTURES`
  7. 优化异步API
     - 为异步API增加是否优先使用大页内存来创建异步队列的配置项
     - 为异步API的I/O线程增加追加模式输出的配置项
     - 为异步API的I/O线程增加支持忙等待的配置选项，以使异步队列的延迟统计结果更接近实际情况
  8. 优化时间戳精度, 将系统下的时间戳全部升级为纳秒级时间戳, 以提高时延统计的精度
     - 时间戳字段的数据类型从 `STimevalT` / `STimeval32T` 变更为 `STimespecT` / `STimespec32T`
     - 协议保持兼容, 但如果使用到了时间戳字段下的微秒时间 `tv_usec` 字段, 则需要修改为纳秒时间 `tv_nsec` 字段,
       否则会因为时间单位的调整而导致时延计算错误
  9. API中添加vs2015工程样例

### 2. 服务端更新概要

  1. fix: 更新上海增量快照的去重处理, 以处理存在时间相同但数据不同的增量快照问题
  2. fix: 完善上海增量快照的更新处理, 防止因为数据丢失导致出现买卖盘价格倒挂
  3. fix: 完善由上海增量快照合成的快照消息中，没有更新和携带实际的原始行情的序列号 `__origTickSeq` 的问题
  4. fix: 修复委托队列增量消息的大小未全部按64位对齐的问题
  5. 支持创业板注册制改革
  6. 其它细节完善和性能优化

### 3. 注意事项说明

  1. 时间戳精度的修改没法完全兼容之前的API, 如果使用到了时间戳字段下的微秒时间 `tv_usec` 字段, 则需要升级API到最新版本
     - 通常只有在统计延迟时才会使用到微秒时间 `tv_usec` 字段, 如果使用到了该字段, 则需要改为使用纳秒时间 `tv_nsec` 字段,
       否则会因为时间单位的调整而导致时延计算错误
     - 升级API以后, 可以通过检查有无编译错误的方式, 来检查是否使用到了 `tv_usec` 字段。如果没有编译错误就无需再做额外的调整
  2. 上海市场存在更新时间相同但数据不同的Level-2快照, 与通常的预期不太相符。(频率不高, 但会存在这样的数据)


---

MDS_0.15.10.5 / 2020-04-07
-----------------------------------

### 1. API更新概要

  1. 服务端兼容 v0.15.5.1 版本API, 客户可以选择不升级
  2. fix: 为API增加对 `SIGPIPE` 信号的屏蔽处理，以避免连接断开以后重复 `send` 导致程序异常退出
  3. 统一沪深期权行情的昨收盘价字段，不再通过该字段推送昨结算价（深圳期权快照中没有结算价字段）
  4. 调整组播行情的行为, 不再强制过滤掉重复的快照行情和重建到的逐笔行情，并允许通过订阅条件进行控制
  5. 重构异步API接口, 统一交易和行情的异步API定义 (不兼容之前版本的异步API)
     - 支持异步接收行情数据
     - 支持自动的连接管理 (自动识别异常并重建连接)
     - 支持异步数据落地和延迟统计 (可以通过配置文件进行控制, 默认为禁用)
     - 主要接口:
        - `MdsAsyncApi_CreateContext`
        - `MdsAsyncApi_AddChannel`
        - `MdsAsyncApi_Start`
        - `MdsAsyncApi_Stop`
        - `MdsAsyncApi_ReleaseContext`
     - 样例代码参见:
         - *`mds_api/samples/mds_sample/01_mds_async_tcp_sample.c`*
  6. 为异步API增加对UDP行情数据的本地订阅/过滤功能
      - 订阅方式与TCP行情订阅方式相同, 相关订阅接口如下:
         - `MdsAsyncApi_SubscribeMarketData`
         - `MdsAsyncApi_SubscribeByString`
         - `MdsAsyncApi_SubscribeByStringAndPrefixes`
      - 对UDP行情数据的订阅和过滤功能默认不开启, 可通过接口或配置予以控制
      - 样例代码参见:
         - *`mds_api/samples/mds_sample/03_mds_async_udp_sample.c`*
  7. 调整示例代码的目录位置

### 2. 服务端更新概要

  1. fix: 修复因为指令乱序而导致快照行情发生数据重复和数据丢失的风险
  2. 其它细节完善和性能优化

### 3. 注意事项说明

  1. 异步接口发生了变化, 无法兼容之前版本的异步API, 请参考样例代码进行升级处理


---

MDS_0.15.9.4 / 2019-12-24
-----------------------------------

### 1. API更新概要

  1. 服务端兼容 v0.15.5.1 版本API, 客户可以选择不升级
  2. 证券静态信息结构体中增加字段 '总股本 `outstandingShare`' 和 '流通股数量 `publicFloatShare`' (API保持兼容)
  3. 行情数据类型 `eMdsMdStreamTypeT` 中新增类型 '深交所国证指数快照行情 `MDS_MD_STREAM_TYPE_SZSE_CN_INDEX`'
  4. 调整指数快照行情中今收盘指数 `CloseIdx` 字段的注释信息, 深圳指数快照行情也将提供收盘指数
  5. 调整行情组播频道的划分, 划分为两个独立的沪/深快照频道和两个独立的沪/深逐笔频道, 共4个组播频道 (API及配置文件保持兼容, 但频道内容发生了变化)
     - 快照-频道1 (Snap1): 上海L1/L2快照
     - 快照-频道2 (Snap2): 深圳L1/L2快照
     - 逐笔-频道1 (Tick1): 上海逐笔成交
     - 逐笔-频道2 (Tick2): 深圳逐笔成交/逐笔委托

### 2. 服务端更新概要

  1. fix: 为深圳逐笔委托中的委托价格增加最大价格上限(1999999999)。如果逐笔委托中的委托价格超出该值, 则将赋值为该值, 避免当委托价格过大时溢出为负数
  2. 增加对深交所国证指数快照行情的支持
  3. 调整行情组播频道的划分, 划分为两个独立的沪/深快照频道和两个独立的沪/深逐笔频道, 共4个组播频道 (API及配置文件保持兼容, 但频道内容发生了变化)
  4. 其它细节完善

### 3. 注意事项说明

  1. 行情组播频道做了调整, 沪深的快照和逐笔都将分别通过不同的独立频道广播
     - 快照-频道1 (Snap1): 上海L1/L2快照
     - 快照-频道2 (Snap2): 深圳L1/L2快照
     - 逐笔-频道1 (Tick1): 上海逐笔成交
     - 逐笔-频道2 (Tick2): 深圳逐笔成交/逐笔委托
  2. 行情数据类型 `eMdsMdStreamTypeT` 中新增了数据类型 '深交所国证指数快照行情 `MDS_MD_STREAM_TYPE_SZSE_CN_INDEX`',
     可以通过该类型来区分普通指数快照和国证指数快照。


---

MDS_0.15.9.1 / 2019-08-16
-----------------------------------

### 1. API更新概要

  1. 服务端兼容 v0.15.5.1 版本API, 客户可以选择不升级
  2. 增加异步API接口, 以支持行情异步接收和处理
     - 支持异步行情接收
     - 支持自动的连接管理 (自动识别异常并重建连接)
     - 样例代码参见:
       - *`mds_api/samples/01_mds_async_tcp_sample.c`*
  3. 增加对重建到的逐笔数据的订阅和推送处理
     - 行情订阅条件和订阅配置中增加 '逐笔数据的数据重建标识 `tickRebuildFlag`', 用于标识是否订阅重建到的逐笔数据
  4. 增加批量订阅模式, 以支持更灵活可控的行情订阅 `@see eMdsSubscribeModeT`
  5. 完善对行情订阅条件中起始时间和初始行情标志的支持, 允许在会话过程中的任意时间指定 `beginTime` 和 `isRequireInitialMktData` 订阅参数
  6. 增加为 `SubscribeByString` 设置附加订阅参数的API接口
     - `MdsApi_SetThreadSubscribeTickType`
     - `MdsApi_SetThreadSubscribeTickRebuildFlag`
     - `MdsApi_SetThreadSubscribeTickExpireType`
     - `MdsApi_SetThreadSubscribeRequireInitMd`
     - `MdsApi_SetThreadSubscribeBeginTime`
  7. 增加可以禁止API在登录后自动执行行情订阅操作的接口
     - `MdsApi_SetThreadAutoSubscribeOnLogon`
  8. 增加用于解析CVS格式行情数据文件的辅助API接口
     - `MdsCsvParser_DecodeCsvRecord`
  9. 新增错误码

    | 错误码 | 描述 |
    | ---- | --------------- |
    | 1037 | 集群编号不匹配 |
    | 1038 | 无此操作权限 |
  10. 替换头文件中使用的 `likely` / `unlikely` 关键字, 以避免与第三方库的同名函数冲突
      - 若编译时提示 `likely` 与 `boost` 库中的函数名称冲突, 可以通过指定宏 `__NO_SHORT_LIKELY` 来解决

### 2. 服务端更新概要

  1. 修复和优化逐笔行情的数据重建处理
  2. 其它细节完善


---

MDS_0.15.9 / 2019-05-31
-----------------------------------

### 1. API更新概要

  1. 服务端兼容 v0.15.5.1 版本API, 客户可以选择不升级 (建议升级, 以兼容深圳证券业务开关数量的调整)
  2. fix: 修复API无法支持取值大于1024的文件描述符的问题
  3. 扩大深圳证券业务开关的最大数量, 以应对行情数据内容的更新
     - 该修改可能会对之前版本的API造成影响: 会影响到证券实时状态消息的延迟计算, 旧版本会读取到无效的打点时间
  4. 调整行情数据类型 `mdStreamType`, `eMdsMdStreamTypeT` 的取值, 使其可以标识出快照行情的具体数据类型
     - 该修改会存在兼容性问题, 客户端程序可以通过编译错误来识别需要调整的地方 (如果没有编译错误就不需要调整)
     - 行情数据类型的取值将尽量与消息类型保持一致, 但以下类型比较特殊：
       - 深圳成交量统计指标
       - 上交所 Level1 行情快照-债券
       - 上交所 Level1 行情快照-基金
  5. 重命名 `securityType` => `mdProductType`, 以避免与交易端的证券类型混淆
    - `securityType` => `mdProductType`
    - `eMdsSecurityTypeT` => `eMdsMdProductTypeT`
  6. 删除已经废弃的虚拟集合竞价消息的消息定义和数据类型定义
  7. API新增如下接口

    | API                          | 描述
    | ---------------------------- | ---------------
    | MdsApi_QueryStockStaticInfo  | 查询证券(股票/债券/基金)静态信息
    | MdsApi_QuerySnapshotList     | 批量查询行情快照接口
    | MdsApi_InitAllByCfgStruct    | 按照配置信息结构体初始化客户端环境接口
    | MdsApi_SendChangePasswordReq | 修改客户端登录密码
    | MdsApi_SetCustomizedIp       | 设置客户端自定义的本地IP地址
    | MdsApi_GetCustomizedIp       | 获取客户端自定义的本地IP
    | MdsApi_SetCustomizedMac      | 设置客户端自定义的本地MAC地址
    | MdsApi_GetCustomizedMac      | 获取客户端自定义的本地MAC
    | MdsApi_SetCustomizedDriverId | 设置客户端自定义的本地设备序列号
    | MdsApi_GetCustomizedDriverId | 获取客户端自定义的本地设备序列号
  8. 新增错误码1029、1034、1035、1036, 调整错误码1007、1022的描述信息

    | 错误码 | 描述
    | ---- | ---------------
    | 1007 | 非服务开放时间
    | 1022 | 尚不支持或尚未开通此业务
    | 1029 | 密码未改变
    | 1034 | 密码强度不足
    | 1035 | 非法的产品类型
    | 1036 | 未通过黑白名单检查

### 2. 服务端更新概要

  1. fix: 修复上海L2快照初始的最低价没有被设置的问题
  2. fix: 解决 `MdsApi_SubscribeByString` 接口的字符串缓存太小, 最多只能订阅14000只证券的问题
  3. fix: 取消不必要的期权行情降噪处理, 以修复期权行情数量相对较少的问题（35%）
  4. fix: 修复在追加订阅行情时, 会把全市场订阅标志冲掉的问题
  5. fix: 修复市场总览行情中交易所发送时间为负数的问题
  6. 完善单条行情快照查询, 允许对交易所代码和行情数据进行模糊匹配(不必再明确指定交易所代码);
     并且当不存在L1快照时将尝试检索L2快照, 而查询结果统一转换为五档快照返回
  7. 调整L2逐笔数据的行情组播频道, 按照频道号混合推送逐笔成交和逐笔委托, 取代之前逐笔成交/逐笔委托分别推送的方式(API保持兼容, 但频道的内容发生了变化)
     - 快照-频道1 (L1): L1快照/指数/期权 (沪深Level-1快照)
     - 快照-频道2 (L2): L2快照 (沪深Level-2快照)
     - 逐笔-频道1 (Tick1): L2逐笔-频道1 (沪深逐笔成交/逐笔委托)
     - 逐笔-频道2 (Tick2): L2逐笔-频道2 (沪深逐笔成交/逐笔委托)
  8. 优化快照行情的去重处理
  9. 修复其他系统缺陷, 完善安全机制

### 3. 注意事项说明

  1. 以下两处重构可能会引起编译错误, 这两个字段通常不会使用, 可以通过编译错误来识别是否有需要调整的地方, 如果没有编译错误就不需要调整
     - 重命名 `securityType` => `mdProductType`
     - 调整行情数据类型 `mdStreamType`, `eMdsMdStreamTypeT` 的取值
  2. 服务器端对L2逐笔数据的行情组播频道做了调整, 将按照频道号混合推送逐笔成交和逐笔委托, 以保留逐笔委托和逐笔成交之间时序关系
     - 之前的逐笔成交/逐笔委托是通过两个频道分别推送的, 需要留意调整以后对处理逻辑是否有影响
  3. 服务器端对快照行情的去重处理进行了优化
     - 当以 `tickType=0` 的模式订阅行情时, 服务器端会对重复的快照行情做去重处理, 不会再推送重复的数据
     - 如果需要获取到所有时点的快照, 可以使用 `tickType=1` 的模式订阅行情。该模式会和之前版本的行为保持一致, 只要行情时间有变化, 即使数据重复也会对下游推送


---

MDS_0.15.7.4 / 2018-08-31
-----------------------------------

### 1. API更新概要

  1. 服务端兼容 v0.15.5.1 版本API，客户可以选择不升级 (建议升级)
  2. fix: 修复当多个线程同时初始化API日志时, 会导致日志信息重复输出的问题
  3. API新增如下接口
    | API                          | 描述
    | ---------------------------- | ---------------
    | MdsApi_SetLastError          | 设置当前线程的API错误号
    | MdsApi_GetLastError          | 获取当前线程最近一次API调用失败的错误号
    | MdsApi_HasMoreCachedData     | 获取回报通道中尚未被回调函数处理的缓存数据长度
    | MdsApi_SetThreadUsername     | 设置当前线程的登录用户名
    | MdsApi_SetThreadPassword     | 设置当前线程的登录密码
  4. 新增部分错误码
    | 错误码 | 描述
    | ---- | ---------------
    | 1031 | 非法的加密类型
    | 1033 | 无可用节点
  5. 重构 SubscribeByString 接口, 增强行情订阅的灵活性
  6. 增加可订阅的数据种类 (DataType), 以支持单独订阅指数行情和期权行情
  7. 增加可以处理压缩过的消息的 MdsApi_WaitOnMsgCompressible 接口, 以支持对接压缩后的行情数据

### 2. 服务端更新概要

  1. fix: 修复上海L2增量快照中最低价没有正确更新(长时间为0)的BUG
  2. fix: 修复行情订阅列表的计数器缺陷（把相同的证券代码分别作为指数代码和股票代码指定了两遍时，会订阅不到指数行情的问题）
  3. fix: 修复没有拦截掉9:25前后已经过时的虚拟集合竞价消息的问题
  4. 修复其他系统缺陷，完善管理功能和故障恢复处理


---

MDS_0.15.5.16 / 2018-08-31
-----------------------------------

### 1. API更新概要

  1. 服务端兼容 v0.15.5.1 版本API, 客户可以选择不升级 (建议升级)
  2. fix: 修复当多个线程同时初始化API日志时, 会导致日志信息重复输出的问题
  3. 增加 MdsApi_HasMoreCachedData 接口, 用于返回已经接收到但尚未被回调函数处理的缓存数据长度
  4. 增加 MdsApi_GetLastError 接口, 用于返回最近一次API调用失败的错误号
  5. 增加设置当前线程登录用户名/登录密码的接口
  6. 重构 SubscribeByString 接口, 增强行情订阅的灵活性
  7. 增加可订阅的数据种类 (DataType), 以支持单独订阅指数行情和期权行情
  8. 增加可以处理压缩过的消息的 MdsApi_WaitOnMsgCompressible 接口, 以支持对接压缩后的行情数据

### 2. 服务端更新概要

  1. fix: 修复上海L2增量快照中最低价没有正确更新(长时间为0)的BUG
  2. fix: 修复行情订阅列表的计数器缺陷（把相同的证券代码分别作为指数代码和股票代码指定了两遍时, 会订阅不到指数行情的问题）
  3. fix: 修复没有拦截掉9:25前后已经过时的虚拟集合竞价消息的问题
  4. 修复系统缺陷, 完善管理功能和故障恢复处理


---

MDS_0.15.5.11 / 2018-06-20
-----------------------------------

### 1. API更新概要

  1. 服务端兼容 v0.15.5.1 版本API, 客户可以选择不升级 (建议升级)
  2. fix: 扩大Level2增量更新消息支持的最大价位变更数量和委托明细数量, 修复在巨幅波动场景下会因为支持的价位数量不足导致丢失价位信息的BUG
     - 如果使用的是旧版本的API, 那么服务器端将不再推送增量更新消息 (只推送全量快照), 以此来保持兼容
     - 如果需要使用增量更新消息的话, 就需要更新到最新版本的API, 否则不需要更新API
  3. 行情订阅条件和订阅配置中增加 '逐笔数据的过期时间类型 tickExpireType' (兼容之前版本)

### 2. 服务端更新概要

  1. fix: 修复因行情发布服务和转发服务使用了相同的请求队列, 导致偶发行情订阅请求无法被正确处理的BUG
  2. fix: 修复在行情巨幅波动场景下因为支持的增量更新价位数量不足导致丢失价位信息的BUG
  3. fix: 修正上海L2（增量）快照会出现最新价不在最低价和最高价之间的问题
  4. fix: 避免因为把相同的证券代码分别作为指数代码和股票代码指定了两遍, 导致指数的已订阅数量为0, 进而不推送指数行情的问题
  5. 添加面向低速网络的行情转发服务
  6. 优化深证行情的转发处理, 改善早盘高峰时期行情延迟过大的问题


---

MDS_0.15.5.4 / 2018-02-22
-----------------------------------

### 1. API更新概要

  1. 服务端兼容 v0.15.5.1 版本API, 客户可以选择不升级
  2. fix: 解决在Windows下的兼容性问题
  3. 调整接口 MdsApi_InitAll, 增加一个函数参数 (pUdpTickOrderAddrKey), 以支持分别订阅逐笔成交和逐笔委托的行情组播
     - 因为增加了接口参数, 需要对程序进行修改, 对该参数传 NULL 即可
  4. 增加接口 MdsApi_GetLastRecvTime、MdsApi_GetLastSendTime, 用于获取通道最近接受/发送消息的时间
  5. 登录失败时, 可以通过 errno/SPK_GET_ERRNO() 获取到具体失败原因

### 2. 服务端更新概要

  1. fix: 优化行情推送, 改善推送服务的公平性
  2. fix: 修复在计算深圳逐笔成交的成交金额时没有转换为int64, 导致溢出的BUG
  3. fix: 修复上海L1指数快照的 securityType 不正确, 取值都是 1 的BUG
  4. fix: 修复查询L1快照时, 未按照查询条件 securityType 进行匹配的问题
  5. fix: 修复 mds_tester 查询功能无法使用的问题
  6. 支持指定行情组播发送端的端口号
  7. 优化深证行情采集, 改善早盘高峰时期行情延迟过大的问题
  8. 优化行情组播的发送延迟
