#pragma once

#if defined(HORSE_LOGGER_USE_DLLS)
#ifdef HORSE_LOGGER_EXPORTS
#define HORSE_LOGGER_API __declspec(dllexport)
#else
#define HORSE_LOGGER_API __declspec(dllimport)
#endif
#else
#define HORSE_LOGGER_API
#endif

namespace horse_logger
{
	enum severity_level
	{
		level_trace,
		level_debug,
		level_info,
		level_warning,
		level_error,
		level_critical
	};

	class HORSE_LOGGER_API Logger
	{
	private:
		class Impl;
		Impl *pImpl_;
	public:
		Logger();
		/**
			��־�ĳ�ʼ��������Ϣ�������� exe__LoggerConfig.ini�޸�����
		***/
		Logger(
			const char *module,
			const char *outputDir = nullptr,              // ���Ϊnullptr,��ʾʹ�� ./logs/module/
			severity_level min_level = level_warning,     // ������͵ȼ�����־���
			bool bOutputFile = true,                      // �Ƿ������ļ���־���
			bool bOutputLogcat = false,                   // �Ƿ�������־�������
			bool bOutputConsole = false,                  // �Ƿ��������̨��־���
			bool bSync = true,                            // �ļ���־�Ƿ�����첽��ʽ
			size_t rotationSize = 1024 * 1024,            // �ļ���־��ת�ߴ�			
			const char *prefix = nullptr,                 // ���Ϊnullptr,��ʾʹ�� module + "_" Ϊǰ׺;
			const char *extension = nullptr,              // ���Ϊnullptr,��ʾʹ�� .log Ϊǰ׺;
			const char *configPath = nullptr              // ���Ϊnullptr,��ʾʹ�� exe__LoggerConfig.iniΪ�����ļ�
		);
		~Logger();

		Logger(const Logger&) = delete;
		Logger& operator=(const Logger&) = delete;

		static void stop(); //�����첽��־ʱ��������ô˺���,�� �˺���������main��������ǰ����
		static void show();
		static void destroy();

		severity_level minLevel()const;
		void log(severity_level level, const char *fmt, ...);
		void log(severity_level level, const wchar_t *fmt, ...);

		template<typename CHARSET = char, typename... Args>
		inline void trace(const CHARSET *fmt, const Args &... args)
		{
			log(level_trace, fmt, args...);
		}

		template<typename CHARSET = char, typename... Args>
		inline void debug(const CHARSET *fmt, const Args &... args)
		{
			log(level_debug, fmt, args...);
		}

		template<typename CHARSET = char, typename... Args>
		inline void info(const CHARSET *fmt, const Args &... args)
		{
			log(level_info, fmt, args...);
		}

		template<typename CHARSET = char, typename... Args>
		inline void warn(const CHARSET *fmt, const Args &... args)
		{
			log(level_warning, fmt, args...);
		}

		template<typename CHARSET = char, typename... Args>
		inline void error(const CHARSET *fmt, const Args &... args)
		{
			log(level_error, fmt, args...);
		}

		template<typename CHARSET = char, typename... Args>
		inline void critical(const CHARSET *fmt, const Args &... args)
		{
			log(level_critical, fmt, args...);
		}
	};

}




