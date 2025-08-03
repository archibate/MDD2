#ifndef __NESC_MD_H__
#define __NESC_MD_H__

#include <map>
#include <memory>
#include <thread>

#include "Const.h"
#include "NescMdDataType.h"

namespace NescForesight {
  struct Receiver;
  class AuthApi;
  class MsgFilter;
  typedef void (*Handler)(const uint8_t *, int);
  void bindThreadToCore(std::thread* thread, int core);
  enum MDType {
      ALL,
      L1Stock,
      L1Index,
      L1Bond,
      L1Fund
  };

  enum ENicType
  {
    E_NIC_NORMAL,          //普通网卡，通过操作系统协议栈接收
    E_NIC_SOLARFLARE_EFVI, //solarflare网卡通过efvi接收，能降低延迟和丢包可能性
  };

  //接收线程传入结构体定义
  typedef struct _MdParam
  {
    char m_interfaceName[64]; //接收网卡名
    char m_localIp[32];       //接收网卡ip
    char m_mcastIp[32];       //组播ip
    uint16_t m_mcastPort;     //组播端口
    int m_bindCpuId;          //接收线程绑定cpuid，-1代表不绑核
    ENicType m_nicType;       //网卡类型

    // 回调函数
    Handler handler = nullptr;

    /**
       * 主备自动切换功能，仅支持盘中切换，在主组播上一定时间内收不到包自动切换到备用组播，
       * 主组播有包后再自动切回来
       **/
    char m_backupIntName[64];       //备用接收网卡名
    char m_backupLocalIp[32];       //备用接收网卡ip
    char m_backupMcastIp[32];       //备用组播ip
    uint16_t m_backSwitchTime = 15; //备切换时间,单位秒
    uint16_t m_backupMcastPort = 0; //备用组播端口,配成0时不启用主备切换功能
    ENicType m_backupNicType;       //备用网卡类型
  } MdParam;

  class NescMdUDPClient {
  public:
      NescMdUDPClient();
      ~NescMdUDPClient();
      /**
       * @brief 登录认证,当主鉴权服务连接出现错误时，若配置了备节点，会切换备节点进行鉴权。
       * @param ip: 认证服务器ip
       * @param port: 认证服务器端口
       * @param username: 认证用户名
       * @param password: 认证密码
       * @param backupIP: 备用认证服务器ip
       * @param backupPort: 备用认证服务器端口
       * @return bool
       * @retval true: 登录成功
       * @retval false: 登录失败
       * @bug None
      */
      bool Login(const char *ip, uint16_t port, const char *username, const char *password, const char* backupIP = nullptr, uint16_t backupPort = 0);
      int Init(MdParam **param, int channelNum);
      int Start();
      int Stop();
      /*
       * @brief 获取上交所全市场静态信息
       * @param staticInfoField: 上交所全市场静态信息
       * @retval 0: 成功
      */
      int QuerySseStaticInfo(SseStaticInfoField &staticInfoField);
      /*
       * @brief 获取深交所全市场静态信息
       * @param staticInfoField: 深交所全市场静态信息
       * @retval 0: 成功
      */
      int QuerySzStaticInfo(SzStaticInfoField &staticInfoField);
      /**
       * @brief 按标的列表获取上交所静态信息
       * @param sseStaticInfoField: 上交所静态信息
       * @param securityIds: 标的列表
       * @param count: 标的列表数量
       * @retval 0: 成功
      */
      int QuerySseStaticInfoByIDs(SseStaticInfoField &staticInfoField, const char* securityIds[], int count=4000);
      /**
       * @brief 按标的列表获取深交所静态信息
       * @param szStaticInfoField: 深交所静态信息
       * @param securityIds: 标的列表
       * @param count: 标的列表数量
       * @retval 0: 成功
      */
      int QuerySzStaticInfoByIDs(SzStaticInfoField &staticInfoField, const char* securityIds[], int count=4000);

      /**
       * @brief 按标的和类型订阅行情数据
       * @param messageTypes: 行情类型
       * @param mCount: 行情类型数量
       * @param marketType: 市场类型
       * @param securityIds: 标的列表
       * @param sCount: 标的列表数量，若不填,需要securityIds最后一个加入空字符或"END",否则会有报错风险；默认4000
       * @retval 0: 成功
      */
      int SubscribeMarketData(const EMdMsgType* messageTypes, int mCount, MarketType marketType, const char* securityIds[], int sCount=4000);

      /**
       * @brief 取消订阅行情数据
       * @param messageTypes: 行情类型
       * @param mCount: 行情类型数量
       * @param marketType: 市场类型
       * @param securityIds: 标的列表
       * @param sCount: 标的列表数量，若不填,需要securityIds最后一个加入空字符或"END",否则会有报错风险；默认4000
       * @retval 0: 成功
      */
      int UnSubscribeMarketData(const EMdMsgType* messageTypes, int mCount, MarketType marketType, const char* securityIds[], int sCount=4000);
      /**
       * @brief 订阅所有行情数据
       * @param messageTypes: 行情类型
       * @param count: 行情类型数量
       * @retval 0: 成功
      */
      int SubscribeAll(const EMdMsgType* messageTypes, int count);
      /**
       * @brief 取消订阅所有行情数据
       * @param messageTypes: 行情类型
       * @param count: 行情类型数量
       * @retval 0: 成功
      */
      int UnSubscribeAll(const EMdMsgType* messageTypes, int count);
  private:
      
      Receiver **receivers;
      int numReceivers;
      AuthApi *authApi;
      MsgFilter *msgFilter;
  };
}
#endif