/**
 * 行情接口类
 **/

#ifndef __MD_XELE_H__
#define __MD_XELE_H__

#include <string>
#include <functional>
#include <vector>
#include <map>

/**
 * @brief 回调函数指针定义
 * @param[in]: uint8_t * 指向接收到的数据缓冲区首地址
 * @param[in]: int 表示数据的有效长度（字节数）
 **/
typedef void (*Handler)(uint8_t *, int);

/**
 * @brief function/bind回调函数对象定义
 * @param[in]: uint8_t * 指向接收到的数据缓冲区首地址
 * @param[in]: int 表示数据的有效长度（字节数）
 **/
typedef std::function<void(uint8_t *, int)> BindHandler;

#define MSG_TYPE_STATIC_SSE             0x0F   //上交静态信息

class CPacketReceiver;

enum ENicType
{
    E_NIC_NORMAL,          //普通网卡，通过操作系统协议栈接收
    E_NIC_SOLARFLARE_EFVI, //solarflare网卡通过efvi接收，能降低延迟和丢包可能性
};

//接收线程传入结构体定义
typedef struct _MdParam
{
    char m_interfaceName[64];      //接收网卡名
    char m_localIp[32];            //接收网卡ip
    char m_mcastIp[32];            //组播ip
    uint16_t m_mcastPort;          //组播端口
    int m_bindCpuId;               //接收线程绑定cpuid，-1代表不绑核
    ENicType m_nicType;            //网卡类型

    /**
     * 支持回调函数以及function/bind，赋值其中任意一个即可。
     * 两者的区别：
     * - handler采用回调函数指针，性能更好，可减少几十个纳秒；
     * - bindHandler采用回调函数对象，使用更方便；
     * 回调函数需尽快返回，否则会影响到后续的包
     **/
    Handler handler = nullptr;
    BindHandler bindHandler = nullptr;

    bool m_polling = true;         //是否使用忙等待模式收包，忙等待模式延迟更小，但会增大cpu占用
    /**
     * api缓存报文的chche大小，单位MB
     * 0代表不使用cache
     * cache模式下cache存满api会丢弃部分包 */
    int m_cache = 0;
    int m_cacheCpuId = -1;         //cache模式下消费线程绑核cpuid，-1代表不绑核

    /**
     * 主备自动切换功能，仅支持盘中切换，在主组播上一定时间内收不到包自动切换到备用组播，
     * 主组播有包后再自动切回来
     **/
	char m_backupIntName[64];      //备用接收网卡名
    char m_backupLocalIp[32];      //备用接收网卡ip
    char m_backupMcastIp[32];      //备用组播ip
    uint16_t m_backupMcastPort = 0;  //备用组播端口,配成0时不启用主备切换功能
    ENicType m_backupNicType;      //备用网卡类型
    int  m_backupCpuId;            //备用报文接收线程绑核cpuid，-1代表不绑核           
} MdParam;


//上交静态信息结构体
#pragma pack(push)
#pragma pack(1)

typedef struct _StaticInfo
{
  uint8_t  messageType;               //消息类型，静态消息类型为0xf
  uint32_t sequence;                  //udp输出包序号，从1开始
  uint8_t  exchangeID;                //交易所id，上交所：1，深交所：2
  char     securityID[9];             //证券代码
/*
 涨跌停限制类型
‘N’表示交易规则（2013修订版）3.4.13规定的有涨跌幅限制类型或者权证管理办法第22条规定
‘R’表示交易规则（2013修订版）3.4.15和3.4.16规定的无涨跌幅限制类型
‘S’表示回购涨跌幅控制类型
‘F’表示基于参考价格的涨跌幅控制
‘P’表示IPO上市首日的涨跌幅控制类型
‘U’表示无任何价格涨跌幅控制类型
*/
  char    priceLimitType;  
  double  upperLimitPrice;           //涨停价
  double  lowerLimitPrice;            //跌停价
  uint64_t buyUnit;                  //买数量单位
  uint64_t sellUnit;                 //卖数量单位
  uint64_t upperQuantityLimitPriceDeclare;   //限价申报数量上限
  uint64_t lowerQuantityLimitPriceDeclare;     //限价申报数量下限
  double priceGear;    //价格档位,申报价格的最小变动单位
  uint64_t upperQuantityMarketPriceDeclare;   //市价申报数量上限
  uint64_t lowerQuantityMarketPriceDeclare;    //市价申报数量下限
  /** 
   * 证券类别
   * ‘ES’表示股票；‘EU’表示基金；‘D’表示债券； ‘RWS’表示权证；‘FF’表示期货；
   * 'CB'表示公募REITs。（参考ISO10962），集合资产管理计划、债券预发行、定向可转债取‘D’
  */
  char     securityType[7];    
  char   securityName[9];      //证券名称

  char fileDate[9];   //文件日期(YYYYMMDD)
  char resv[3];       //保留字段

  char     securitySubType[4];    //详细证券类别,参考《上海证券市场竞价撮合平台市场参与者接口规格说明书》
  char financeFlag;     //融资标的标志,‘T’表示是融资标的证券,‘F’表示不是融资标的证券
  char shortSaleFlag;   //融券标的标志,‘T’表示是融券标的证券,‘F’表示不是融券标的证券
  char productStatus[21];   //产品状态,参考《上海证券市场竞价撮合平台市场参与者接口规格说明书》
  char listDate[9];    //上市日期(YYYYMMDD)

} StaticInfo;

#pragma pack(pop)

//
/**
 * @brief 上交债券行情快照合约状态到tradingphasecode转换函数
 * @param[in] instrumentStatus：快照合约状态
 * @param[out] tradingPhaseCode：转换后的tradingPhaseCode，只给出第一个字节，后面紧跟结束符'\0'
 * @return 
 **/
void insStatusToPhaseCode(const char* instrumentStatus, char* tradingPhaseCode);

class XeleMd
{
public:
    /**
     * @brief 获取API版本号
     * @return API版本字符串
     **/
    static const char *GetVersion();

    /**
     * @brief 初始化API模块
     * @param[in] param：指向传入结构体指针数组的指针
     * @param[in] count：传入结构体指针数组大小
     * @return 初始化结果,0代表成功，-1代表失败
     **/
    int Init(MdParam **const param, const int count);

    /**
     * @brief 开始接收行情
     * @return 接收执行结果,0代表成功，-1代表失败
     **/
    int Start();

    /**
     * @brief 查询指定合约的静态信息
     * @param[in] security：合约ID
     * @return 查询到的静态信息结构体指针，查不到返回nullptr
     **/
    const StaticInfo* QueryStatic(const char* securityId);

    /**
     * @brief 查询所有合约的静态信息
     * @return 包含所有合约静态信息的map，键是合约id，值是静态信息结构体指针
     **/
    const std::map<const std::string, const StaticInfo*>* QueryAllStatic();

    /**
     * @brief 停止接收行情
     * @return 停止执行结果,0代表成功，-1代表失败
     **/
    int Stop();

private:
    CPacketReceiver **m_receiver = nullptr;
    int m_iCount;
};

#endif
