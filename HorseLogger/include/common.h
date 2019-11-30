#ifndef HORSE_LOGGER_COMMON
#define HORSE_LOGGER_COMMON
#pragma once

/***
说明：
  支持不同的模块，输出到不同的文件中

一、库的使用
  如果需要使用动态库，需要定义宏 HORSE_LOGGER_USE_DLLS
  HORSE_LOGGER_USE_DLLS

二：思考：
  如果全局静态对象obj在main函数推出后才开始析构，在析构函数中调用了 log()
  a.同部日志是否有问题
  b.异步日志是否有问题

三：例子
1.如何初始化日志的相关参数
  参数顺序
  const char *module,                              // 模块名
  const char *outputDir = nullptr,                 // 如果为nullptr,表示使用 ./logs/module/
  severity_level min_level = level_warning,        // 允许最低等级的日志输出
  bool bOutputFile = true,                         // 是否允许文件日志输出
  bool bOutputLogcat = false,                      // 是否允许日志窗口输出
  bool bOutputConsole = false,                     // 是否允许控制台日志输出
  bool bSync = true,                               // 文件日志是否采用异步方式
  size_t rotationSize = 1024 * 1024,               // 文件日志旋转尺寸
  const char *prefix = nullptr,                    // 如果为nullptr,表示使用 module + "_" 为前缀;
  const char *extension = nullptr,                 // 如果为nullptr,表示使用 .log 为前缀;
  const char *configPath = nullptr                 // 如果为nullptr,表示使用 exe__LoggerConfig.ini为配置文件

  (1)
  HORSE_LOGGER_INIT_ARGS,该宏的作用：程序第一次运行时使用的参数，之后就会使用 configPath.ini文件内的配置
  
  直接修改 本文件中的 HORSE_LOGGER_INIT_ARGS 的宏定义,以改变初始化参数 []
  或者
  在stdafx.h中定义#define HORSE_LOGGER_INIT_ARGS   "common","D:/Maintop/horse_test_logger",horse_logger::level_warning ......

  (2)
  在stdafx.cpp 添加
  GLOBAL_DEFAULT_HORSE_LOGGER

2.异步文件日志时注意事项
//不要再全局对象 或 全局静态对象 或 局部静态对象 的析构函数调用 log()  [经过多次严重会阻塞在 spAsyncSink->stop()中]
(1).必须使用 STOP_HORSE_LOGGER 或者 AUTO_STOP_HORSE_LOGGER [用法如下]

    例如：
    int main()
    {
      //具体逻辑....
      STOP_HORSE_LOGGER
      return 0;
    }

    或者
    int main()
    {
      AUTO_STOP_HORSE_LOGGER
      //具体逻辑....
      return 0;
    }

****/

#ifndef HORSE_LOGGER_MASTER
#include "HorseLogger.h"

#ifdef _DEBUG
#pragma comment(lib,"HorseLoggerd.lib")
#else
#pragma comment(lib,"HorseLogger.lib")
#endif

#ifndef HORSE_LOGGER_INIT_ARGS
//#define HORSE_LOGGER_INIT_ARGS   "common","D:/HORSE/horse_test_logger",horse_logger::level_warning , true, true,true,true,1024*1024,nullptr,nullptr,"D:/" 
#define HORSE_LOGGER_INIT_ARGS   "common"                          /* 模块名 */                                                            \
                                 ,"D:/horse_logger/logs"           /* 如果为nullptr,表示使用 ./logs/module/ */                              \
                                 ,horse_logger::level_warning      /* 允许最低等级的日志输出 */                                              \
                                 ,true                             /* 是否允许文件日志输出 */                                                \
                                 ,true                             /* 是否允许日志窗口输出 */                                                \
                                 ,true                             /* 是否允许控制台日志输出 */                                              \
                                 ,true                             /* 文件日志是否采用异步方式 */                                            \
                                 ,1024*1024                        /* 文件日志旋转尺寸 */                                                   \
                                 ,nullptr                          /* 如果为nullptr,表示使用 module + "_" 为前缀 */                          \
                                 ,nullptr                          /* 如果为nullptr,表示使用 .log 为前缀 */                                  \
                                 ,"D:/horse_logger/config.ini"     /* 如果为nullptr,表示使用 exe__LoggerConfig.ini为配置文件 */
#endif


//注意：需要用户在stdafx.cpp添加 GLOBAL_DEFAULT_HORSE_LOGGER
extern horse_logger::Logger g_default_horse_logger;

class AutoStopHorseLogger
{
public:
  AutoStopHorseLogger() {}
  ~AutoStopHorseLogger() { horse_logger::Logger::stop(); }
};


//定义默认的horse logger
#define GLOBAL_DEFAULT_HORSE_LOGGER         horse_logger::Logger g_default_horse_logger(HORSE_LOGGER_INIT_ARGS);

//停止日志，主要针对异步文件日志
#define STOP_HORSE_LOGGER                   horse_logger::Logger::stop();
#define AUTO_STOP_HORSE_LOGGER              AutoStopHorseLogger autoStopHorseLogger;

//#define DISABLE_DEFAULT_HORSE_LOGGER
#ifndef DISABLE_DEFAULT_HORSE_LOGGER
#define HORSE_TRACE(msg,...)                g_default_horse_logger.trace(msg,__VA_ARGS__)
#define HORSE_DEBUG(msg,...)                g_default_horse_logger.debug(msg,__VA_ARGS__)
#define HORSE_INFO(msg,...)                 g_default_horse_logger.info(msg,__VA_ARGS__)
#define HORSE_WARN(msg,...)                 g_default_horse_logger.warn(msg,__VA_ARGS__)
#define HORSE_ERROR(msg,...)                g_default_horse_logger.error(msg,__VA_ARGS__)
#define HORSE_CRITICAL(msg,...)             g_default_horse_logger.critical(msg,__VA_ARGS__)
#else
#define HORSE_TRACE(msg,...)                (void)0
#define HORSE_DEBUG(msg,...)                (void)0
#define HORSE_INFO(msg,...)                 (void)0
#define HORSE_WARN(msg,...)                 (void)0
#define HORSE_ERROR(msg,...)                (void)0
#define HORSE_CRITICAL(msg,...)             (void)0
#endif

#endif



#endif
