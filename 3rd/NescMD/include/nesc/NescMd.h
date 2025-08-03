#ifndef __NESC_MD_H__
#define __NESC_MD_H__

#include <map>
#include <memory>
#include <thread>
#include "Const.h"
#include "NescMdDataType.h"

namespace NescForesight {

  class AuthApi;
  class MsgFilter;

  typedef void (*Handler)(const uint8_t *, int);
  struct InitMdParam
  {
    Handler handler[16] = {nullptr} ;    //回调函数列表
    char cpulist[16] = {0};           //cpu绑核列表
    int switch_time = 15;             //主备切换时间单位秒 0表示不启用主备切换
    int nic_type = 1;                 //网卡类别 1代表solarflare低延迟网卡  0代表普通网卡  建议保持默认值
  };

  void bindThreadToCore(std::thread* thread, int core);

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
      
      /*
       * @brief 初始化
       * @param param: 初始化参数
       * @retval 0: 成功
      */
      int Init(InitMdParam* param);

      /*
       * @brief 开始接收行情
       * @retval 0: 成功
      */
      int Start();

      /*
       * @brief 停止接收行情,释放所有资源 --请勿轻易调用
       * @retval 0: 成功
      */
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
       * @param interfaceName: 行情网口
       * @retval 0: 成功
      */
      int SubscribeMarketData(const EMdMsgType* messageTypes, int mCount, MarketType marketType, const char* securityIds[], int sCount=4000, const char* interfaceName="");

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
       * @param interfaceName: 行情网口
       * @retval 0: 成功
      */
      int SubscribeAll(const EMdMsgType* messageTypes, int count, const char* interfaceName="");
      /**
       * @brief 取消订阅所有行情数据
       * @param messageTypes: 行情类型
       * @param count: 行情类型数量
       * @retval 0: 成功
      */
      int UnSubscribeAll(const EMdMsgType* messageTypes, int count);

     
      /**  新增辅助函数
       * @brief 网卡接收不同行情的接收通道总数
       * @retval int 通道总数
      */
      int GetQuoteRecvChannelNumber();

      /** 新增辅助函数 类型绑核 优先级最高
       * @brief 将特定类型接收线程绑核
       * @retval bool 
      */
      bool bindThreadCpuCore(EMdMsgType messageType, int coreId);
  private:
      AuthApi *authApi;
      MsgFilter *msgFilter;
  };                 
}
#endif