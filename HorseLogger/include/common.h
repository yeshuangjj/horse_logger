#ifndef HORSE_LOGGER_COMMON
#define HORSE_LOGGER_COMMON
#pragma once

/***
˵����
  ֧�ֲ�ͬ��ģ�飬�������ͬ���ļ���

һ�����ʹ��
  �����Ҫʹ�ö�̬�⣬��Ҫ����� HORSE_LOGGER_USE_DLLS
  HORSE_LOGGER_USE_DLLS

����˼����
  ���ȫ�־�̬����obj��main�����Ƴ���ſ�ʼ�����������������е����� log()
  a.ͬ����־�Ƿ�������
  b.�첽��־�Ƿ�������

��������
1.��γ�ʼ����־����ز���
  ����˳��
  const char *module,                              // ģ����
  const char *outputDir = nullptr,                 // ���Ϊnullptr,��ʾʹ�� ./logs/module/
  severity_level min_level = level_warning,        // ������͵ȼ�����־���
  bool bOutputFile = true,                         // �Ƿ������ļ���־���
  bool bOutputLogcat = false,                      // �Ƿ�������־�������
  bool bOutputConsole = false,                     // �Ƿ��������̨��־���
  bool bSync = true,                               // �ļ���־�Ƿ�����첽��ʽ
  size_t rotationSize = 1024 * 1024,               // �ļ���־��ת�ߴ�
  const char *prefix = nullptr,                    // ���Ϊnullptr,��ʾʹ�� module + "_" Ϊǰ׺;
  const char *extension = nullptr,                 // ���Ϊnullptr,��ʾʹ�� .log Ϊǰ׺;
  const char *configPath = nullptr                 // ���Ϊnullptr,��ʾʹ�� exe__LoggerConfig.iniΪ�����ļ�

  (1)
  HORSE_LOGGER_INIT_ARGS,�ú�����ã������һ������ʱʹ�õĲ�����֮��ͻ�ʹ�� configPath.ini�ļ��ڵ�����
  
  ֱ���޸� ���ļ��е� HORSE_LOGGER_INIT_ARGS �ĺ궨��,�Ըı��ʼ������ []
  ����
  ��stdafx.h�ж���#define HORSE_LOGGER_INIT_ARGS   "common","D:/Maintop/horse_test_logger",horse_logger::level_warning ......

  (2)
  ��stdafx.cpp ���
  GLOBAL_DEFAULT_HORSE_LOGGER

2.�첽�ļ���־ʱע������
//��Ҫ��ȫ�ֶ��� �� ȫ�־�̬���� �� �ֲ���̬���� �������������� log()  [����������ػ������� spAsyncSink->stop()��]
(1).����ʹ�� STOP_HORSE_LOGGER ���� AUTO_STOP_HORSE_LOGGER [�÷�����]

    ���磺
    int main()
    {
      //�����߼�....
      STOP_HORSE_LOGGER
      return 0;
    }

    ����
    int main()
    {
      AUTO_STOP_HORSE_LOGGER
      //�����߼�....
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
#define HORSE_LOGGER_INIT_ARGS   "common"                          /* ģ���� */                                                            \
                                 ,"D:/horse_logger/logs"           /* ���Ϊnullptr,��ʾʹ�� ./logs/module/ */                              \
                                 ,horse_logger::level_warning      /* ������͵ȼ�����־��� */                                              \
                                 ,true                             /* �Ƿ������ļ���־��� */                                                \
                                 ,true                             /* �Ƿ�������־������� */                                                \
                                 ,true                             /* �Ƿ��������̨��־��� */                                              \
                                 ,true                             /* �ļ���־�Ƿ�����첽��ʽ */                                            \
                                 ,1024*1024                        /* �ļ���־��ת�ߴ� */                                                   \
                                 ,nullptr                          /* ���Ϊnullptr,��ʾʹ�� module + "_" Ϊǰ׺ */                          \
                                 ,nullptr                          /* ���Ϊnullptr,��ʾʹ�� .log Ϊǰ׺ */                                  \
                                 ,"D:/horse_logger/config.ini"     /* ���Ϊnullptr,��ʾʹ�� exe__LoggerConfig.iniΪ�����ļ� */
#endif


//ע�⣺��Ҫ�û���stdafx.cpp��� GLOBAL_DEFAULT_HORSE_LOGGER
extern horse_logger::Logger g_default_horse_logger;

class AutoStopHorseLogger
{
public:
  AutoStopHorseLogger() {}
  ~AutoStopHorseLogger() { horse_logger::Logger::stop(); }
};


//����Ĭ�ϵ�horse logger
#define GLOBAL_DEFAULT_HORSE_LOGGER         horse_logger::Logger g_default_horse_logger(HORSE_LOGGER_INIT_ARGS);

//ֹͣ��־����Ҫ����첽�ļ���־
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
