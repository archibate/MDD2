#文件目录
├── api_client                    api测试工具，具体使用查看api_client内的README
├── config                        api运行配置文件夹，里面配置api启动所需的相关参数
├── demo                          API的demo文件
│   ├── encode                    加密工具
│   ├── example_option            期权demo
│   │   └── config_file_process   处理配置文件
│   ├── example_order_delay       用来测试报单接口半链路延迟和全链路延迟的demo
│   │   └── config_file_process   处理配置文件
│   ├── example_stock             股票demo
│   │   └── config_file_process   处理配置文件
│   ├── supervise_option          期权windows平台下获取穿透式监管信息的小程序
│   └── supervise_stock           股票windows平台下获取穿透式监管信息的小程序
├── docs                          Xele-Trade-Securities-交易员API接口手册
├── include                       头文件
└── lib                           库文件


---
#使用指南
-参考本文件的demo使用

---
#运行环境
-Linux

---
#编译器，若gcc版本大于等于5.1，则需要增加编译参数-D_GLIBCXX_USE_CXX11_ABI=0进行编译，若出现编译失败的情况，且是第三方库依赖过多导致，则需要厂商重新提供api版本库
-cmake >=3.12
- 4.8.5 <= g++ (GCC) < 5.1

---
#开发语言版本
-c++ 11

---
# API使用方法及API的demo使用
# API的Demo示例程序

- 股票示例example_stock文件夹:
- demo_main.cpp

---
# 编译步骤
-  步骤1：cmake CMakeLists.txt
-  步骤2: make

---
# 配置
- api_config.txt
- api连接配置

---
# 运行
- ./manager_demo accountID password node market
- 例如：./manager_demo  121228 111111 0 1 表示登录账号为121228，节点号为0，密码为111111，登录柜台为上交柜台
- demo自测结果：通过
-
-accountID表示资金账户
-password表示集中交易密码
-node表示用户登录节点号
-market表示用户登录柜台类型
-可以使用./demo来获取帮助信息


# 说明
-replace_api_func_name.py是用来替换api (version:2.3)发生命名改变的函数的脚本
