# Swordfish-MDS API    {#mainpage}

箭鱼行情数据服务系统API使用说明

---

## Quick Start

### 一、API目录结构

~~~{.c}
    MdsApi_X.XX.X-YYYYMMDD
    │
    ├── 行情API (MdsApi) 更新说明_vX.XX.X.pdf
    │
    ├── api_docs-X.XX.X                         # API文档
    │   ├── mds_api.chm
    │   └── mds_api_html
    │       └── index.html
    │
    ├── mds_libs-X.XX.X-release
    │   │
    │   ├── include
    │   │   ├── mds_api
    │   │   │   ├── errors
    │   │   │   │   └── mds_errors.h            # 行情错误码定义
    │   │   │   ├── mds_api.h                   # 行情API同步接口定义文件
    │   │   │   └── mds_async_api.h             # 行情API异步接口定义文件
    │   │   │
    │   │   ├── mds_global                      # 行情API领域模型定义文件目录
    │   │   │   ├── mds_base_model.h
    │   │   │   ├── mds_mkt_packets.h
    │   │   │   └── mds_qry_packets.h
    │   │   │
    │   │   └── oes_global                      # 行情API、交易API共用的领域模型定义文件目录
    │   │       ├── oes_base_constants.h
    │   │       ├── oes_base_model.h
    │   │       ├── oes_base_model_credit.h
    │   │       └── oes_base_model_option.h
    │   │
    │   ├── linux64                             # API库文件目录 (适用于Linux系统)
    │   │   ├── dll
    │   │   │   └── libmds_api.so               # 动态库文件
    │   │   ├── libmds_api.a                    # 普通的静态库文件
    │   │   └── pic
    │   │       └── libmds_api.pic.a            # 位置无关代码(PIC)版本的静态库文件
    │   │
    │   ├── win32                               # API库文件目录 (适用于32位Windows系统)
    │   │   ├── mds_api.dll                     # 动态库文件
    │   │   ├── mds_api.lib
    │   │   └── mingw
    │   │       ├── libmds_api.a                # 普通的静态库文件
    │   │       └── pic
    │   │           └── libmds_api.pic.a        # 位置无关代码(PIC)版本的静态库文件
    │   │
    │   ├── win64                               # API库文件目录 (适用于64位Windows系统)
    │   │   ├── mds_api.dll                     # 动态库文件
    │   │   ├── mds_api.lib
    │   │   └── mingw
    │   │       ├── libmds_api.a                # 普通的静态库文件
    │   │       └── pic
    │   │           └── libmds_api.pic.a        # 位置无关代码(PIC)版本的静态库文件
    │   │
    │   ├── macos_arm                           # API库文件目录 (适用于arm架构的MAC系统)
    │   │   ├── dll
    │   │   │   └── libmds_api.so               # 动态库文件
    │   │   ├── libmds_api.a                    # 普通的静态库文件
    │   │   └── pic
    │   │       └── libmds_api.pic.a            # 位置无关代码(PIC)版本的静态库文件
    │   │
    │   ├── macos_x86                           # API库文件, 适用于x86架构的MAC系统
    │   │   ├── dll
    │   │   │   └── libmds_api.so               # 动态库文件
    │   │   ├── libmds_api.a                    # 普通的静态库文件
    │   │   └── pic
    │   │       └── libmds_api.pic.a            # 位置无关代码(PIC)版本的静态库文件
    │   │
    │   └── samples
    │       ├── mds_sample                      # 行情API样例目录
    │       └── vs2015_project                  # vs2015项目文件目录
    │
    └── mds_tester-X.XX.X-release               # API测试工具目录
        ├── linux64
        ├── macos_arm
        ├── macos_x86
        ├── readme.txt
        └── win32
~~~

### 二、示例代码的编译和运行

1. 进入样例代码目录

    - ``cd mds_libs-xxx/samples/mds_sample/``

2. 编译代码

    - ``make -f Makefile.sample``

3. 修改配置文件，确认服务地址、用户名等正确

    - ``vi mds_client_sample.conf``

4. 运行样例程序

    - ``./01_mds_async_tcp_sample``
    - ``./03_mds_async_udp_sample``

5. 样例文件说明

~~~{.c}
    samples
    └── mds_sample
        ├── 01_mds_async_tcp_sample.c                       # TCP行情对接的样例代码 (基于异步API实现)
        ├── 02_mds_async_tcp_sample.minimal.c               # TCP行情对接的样例代码 (精简版本, 基于异步API实现)
        ├── 03_mds_async_udp_sample.c                       # UDP行情对接的样例代码 (基于异步API实现)
        ├── 04_mds_sync_tcp_sample.c                        # TCP行情对接的样例代码 (基于同步API实现)
        ├── 05_mds_sync_udp_sample.c                        # UDP行情对接的样例代码 (基于同步API实现)
        ├── 06_mds_query_sample.c                           # 证券静态信息查询和快照行情查询的样例代码
        ├── 07_mds_strerror_sample.c                        # 打印错误号对应错误信息的样例代码
        ├── 08_mds_subscribe_by_query_detail_sample.c       # TCP行情对接的样例代码 (基于异步API实现, 通过查询证券静态信息来订阅行情)
        ├── 09_mds_tick_resend_sample.c                     # 逐笔数据重传的样例代码 (基于异步API实现)
        ├── 10_mds_generate_ciphertext_password_sample.c    # 生成加密后的密文密码字符串的样例代码, 用于不允许配置文件中出现明文密码的场景
        ├── 11_mds_binfile_restore_sample.c                 # 基于API接口读取二进制行情数据文件的样例代码 (基于同步API实现)
        ├── Makefile.sample
        └── mds_client_sample.conf                          # 行情API配置文件
~~~

### 三、版本升级指引及修改历史

- 版本升级指引参见 <@ref update_guide>
- 版本修改历史参见 <@ref changelog>

### 四、组播行情对接说明

- 组播行情对接说明参见 <@ref multicast_guide>

### 五、错误码表

- 错误码表参见 <@ref err_code>

### 六、常见问题

1. 同步接口和异步接口的区别

    异步接口的底层也是基于同步接口实现的，相对于同步接口主要是易用性和功能方面的提升，包括:

    1. 异常处理方面：内置了异常检测和连接重建机制；
    2. 性能方面：内置了之前需要客户端实现的优化技术，包括CPU绑定、忙等待、大页内存、异步处理、延迟统计、异步数据落地等；
    3. 易用性方面：提供更加简单的接口，特别是更易于对接多频道的组播行情；
    4. 功能方面：针对组播行情，提供了本地行情订阅功能，可以使用和TCP相同的接口来对组播行情进行订阅和过滤。

    总的来说，异步接口是在不牺牲性能的前提下，最大程度的平衡和满足性能和易用性方面的需要。而如果需要完全自主控制并以此实现最极致的性能和资源利用率的话，就需要基于同步接口来开发

2. API有哪些内置线程

    - 同步接口的行情API没有内置线程, 需要用户自行管理线程

    - 异步接口的行情API有如下内置线程:
      1. 通信线程 (mdsapi_communication): 用于网络通信, 为关键工作线程
      2. 回调线程 (mdsapi_callback): 用于执行回调, 为关键工作线程, 关闭时会借用通信线程执行回调; 可通过调整配置 ``mds_client.async_api.  isAsyncCallbackAble = no`` 关闭
      3. 连接管理线程 (mdsapi_connect): 默认关闭, 借用通信线程执行连接管理; 可通过调整配置 ``mds_client.async_api.isAsyncConnectAble = yes`` 打开
      4. 异步I/O线程 (mdsapi_io_thread): 用于落地数据和执行统计工作, 默认关闭; 可通过调整配置 ``mds_client.async_api.ioThread.enable = yes`` 打开

3. 价格和金额单位

    - MDS中的所有价格均为`int32`类型的整型数值，单位精确到元后四位, 即: 1元=10000
    - MDS中的所有金额均为`int64`类型的整型数值，单位精确到元后四位, 即: 1元=10000

4. 份额单位
    - MDS中的所有委托数量、成交数量等份额单位均为`int32`或`int64`类型的整型数值，不带小数位
    - **注意:** 上海债券的份额单位是 <b>'手'</b>，而不是 '张'，与其它不同

5. MDS中的行情时间(updateTime)是交易所时间吗？

    - 是的，该时间来源于交易所，是行情数据的生成时间或者上游发送时间（如果采集不到行情生成时间的话）
