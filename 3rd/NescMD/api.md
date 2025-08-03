# 接口说明

## 1、L1&L2客户端订阅UDP组播

### 1.1 初始化函数

初始化组播组，返回0表示成功，返回1表示失败

```c++
int Init(InitMdParam **param)
```

| 参数       | 类型       | 含义                 |
| ---------- | ---------- | -------------------- |
| param      | InitMdParam * | 组播接收线程相关配置 |

### 1.2 登录接口

登录认证,当主鉴权服务连接出现错误时，若配置了备节点，会切换备节点进行鉴权。

```c++
bool Login(const char *ip, uint16_t port, const char *username, const char *password, const char* backupIP = nullptr, uint16_t backupPort = 0);
```

| 参数       | 类型         | 含义               |
| ---------- | ------------ | ------------------ |
| ip         | const char*  | 认证服务器ip       |
| port       | uint16_t     | 认证服务器端口     |
| username   | const char * | 认证用户名         |
| password   | const char * | 认证密码           |
| backupIP   | const char*  | 备用认证服务器ip   |
| backupPort | uint16_t     | 备用认证服务器端口 |

### 1.3  开始接收

开始接收行情，返回0表示成功，返回1表示失败。

```c++
int Start();
```

参数：无

### 1.4 停止接收

停止接受行情，返回值0表示成功，返回1表示失败。

```c++
int Stop();
```

参数：无

### 1.5 获取上交所全市场静态信息

获取上交所全市场静态信息，返回0表示成功，返回1表示失败。

```c++
int QuerySseStaticInfo(SseStaticInfoField &staticInfoField);
```

#### 示例

```c++
// staticInfoField 用于存储查询的返回值
SseStaticInfoField staticInfo;
int rtn = client.QuerySseStaticInfo(staticInfo);
if (rtn == 0) {
    std::cout << "QueryStaticInfo success" << std::endl;
    for (int i = 0; i < staticInfo.count;i++) {
        std::cout << staticInfo.staticInfos[i].securityID << std::endl;
    }
}
```

### 1.6 获取深交所全市场静态信息

获取深交所全市场静态信息，返回0表示成功，返回1表示失败。

```c++
int QuerySzStaticInfo(SzStaticInfoField &staticInfoField);
```

#### 示例

```c++
// staticInfoField 用于存储查询的返回值
SzStaticInfoField staticInfo;
int rtn = client.QuerySzStaticInfo(staticInfo);
if (rtn == 0) {
    std::cout << "QueryStaticInfo success" << std::endl;
    for (int i = 0; i < staticInfo.count;i++) {
        std::cout << staticInfo.staticInfos[i].securityID << std::endl;
    }
}
```

### 1.7 按标的列表获取上交所静态信息

按标的列表获取上交所静态信息，返回0表示成功，返回1表示失败。

```c++
int QuerySseStaticInfoByIDs(SseStaticInfoField &staticInfoField, const char* securityIds[], int count=4000);
```

| 参数            | 含义                                                         | 类型               |
| --------------- | ------------------------------------------------------------ | ------------------ |
| staticInfoField | 存储查询到的上交所静态信息                                   | SseStaticInfoField |
| securityIds     | 查询的标的列表                                               | const char *[]     |
| count           | 标的列表里的标的数，若不填,需要securityIds最后一个加入空字符或"END",否则会有报错风险；默认4000 | int                |

#### 示例

```c++
// staticInfoField 用于存储查询的返回值
SseStaticInfoField staticInfo;
const char* securiryIDs[] = {"600958", "603688", "END"};
int rtn = client.QuerySseStaticInfoByIDs(staticInfo, securiryIDs);
if (rtn == 0) {
    std::cout << "QueryStaticInfo success" << std::endl;
    for (int i = 0; i < staticInfo.count;i++) {
        std::cout << staticInfo.staticInfos[i].securityID << std::endl;
    }
}
```

### 1.8 按标的列表获取深交所静态信息

按标的列表获取深交所静态信息，返回0表示成功，返回1表示失败。

```c++
int QuerySzStaticInfoByIDs(SzStaticInfoField &staticInfoField, const char* securityIds[], int count=4000);
```

| 参数            | 含义                                                         | 类型              |
| --------------- | ------------------------------------------------------------ | ----------------- |
| staticInfoField | 存储查询到的深交所静态信息                                   | SzStaticInfoField |
| securityIds     | 查询的标的列表                                               | const char *[]    |
| count           | 标的列表里的标的数，若不填,需要securityIds最后一个加入空字符或"END",否则会有报错风险；默认4000 | int               |

#### 示例

```c++
// staticInfoField 用于存储查询的返回值
SzStaticInfoField staticInfo;
const char* securiryIDs[] = {"000686", "002865", ""};
int rtn = client.QuerySzStaticInfoByIDs(staticInfo, securiryIDs);
if (rtn == 0) {
    std::cout << "QueryStaticInfo success" << std::endl;
    for (int i = 0; i < staticInfo.count;i++) {
        std::cout << staticInfo.staticInfos[i].securityID << std::endl;
    }
}
```

### 1.9 按标的列表与行情类型订阅行情

按标的列表与行情类型订阅行情数据，返回0则为成功，此接口支持追加订阅，订阅会进行去重处理。

```c++
int SubscribeMarketData(const EMdMsgType* messageTypes, int mCount, MarketType marketType, const char* securityIds[], int sCount=4000, const char* interfaceName="");
```

| 参数          | 类型              | 含义                                                         |
| ------------- | ----------------- | ------------------------------------------------------------ |
| messageTypes  | const EMdMsgType* | 行情类型                                                     |
| mCount        | int               | 行情类型数量，此字段需与行情类型列表数量一致。               |
| marketType    | MarketType        | 市场类型                                                     |
| securityIds[] | const char*       | 标的列表，仅数字无后缀，例如浦发银行为600000                 |
| sCount        | int               | 标的列表里的标的数，此字段需与标的代码列表数量一致。***\*若不填,默认值4000，同时需要securityIds最后加入空字符或"END",否则会有报错风险。若输入数量大于4000标的，此字段必填。\**** |
| interfaceName | const char*       | 行情网口，不填或则为空字符串代表使用默认行情网口               |

### 2.10 按标的列表与行情类型取消订阅行情

按标的列表与行情类型取消订阅行情数据，返回0则为成功。

```c++
int UnSubscribeMarketData(const EMdMsgType* messageTypes, int mCount, MarketType marketType, const char* securityIds[], int sCount=4000);  
```

| 参数          | 类型              | 含义                                                         |
| ------------- | ----------------- | ------------------------------------------------------------ |
| messageTypes  | const EMdMsgType* | 行情类型                                                     |
| mCount        | int               | 行情类型数量，此字段需与行情类型列表数量一致。               |
| marketType    | MarketType        | 市场类型                                                     |
| securityIds[] | const char*       | 标的列表，仅数字无后缀，例如浦发银行为600000                 |
| sCount        | int               | 标的列表里的标的数，此字段需与标的代码列表数量一致。***\*若不填,默认值4000，同时需要securityIds最后加入空字符或"END",否则会有报错风险。若输入数量大于4000标的，此字段必填。\**** |

### 2.11 按行情类型订阅行情数据

按行情类型订阅行情数据，返回0则为成功，此功能支持追加订阅。

```c++
int SubscribeAll(const EMdMsgType* messageTypes, int count, const char* interfaceName="");
```

| 参数         | 类型              | 含义                                           |
| ------------ | ----------------- | ---------------------------------------------- |
| messageTypes | const EMdMsgType* | 行情类型                                       |
| count        | int               | 行情类型数量，此字段需与行情类型列表数量一致。 |
| interfaceName | const char*       | 行情网口，不填或则为空字符串代表使用默认行情网口    |
### 2.12 按行情类型取消订阅行情数据

按行情类型取消订阅行情数据，返回0则为成功。

```c++
int UnSubscribeAll(const EMdMsgType* messageTypes, int count);  
```

| 参数         | 类型              | 含义                                                   |
| ------------ | ----------------- | ------------------------------------------------------ |
| messageTypes | const EMdMsgType* | 行情类型[[见5.4行情类型枚举值\]](#_5.4 行情类型枚举值) |
| count        | int               | 行情类型数量，此字段需与行情类型列表数量一致。         |



### * demo

可以参考 `examples\NescMdUDPClient.cpp`

demo的编译方法如下

cd examples
mkdir build
cd build
cmake ..
make
