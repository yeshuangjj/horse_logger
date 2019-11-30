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
			日志的初始化配置信息，可以在 exe__LoggerConfig.ini修改配置
		***/
		Logger(
			const char *module,
			const char *outputDir = nullptr,              // 如果为nullptr,表示使用 ./logs/module/
			severity_level min_level = level_warning,     // 允许最低等级的日志输出
			bool bOutputFile = true,                      // 是否允许文件日志输出
			bool bOutputLogcat = false,                   // 是否允许日志窗口输出
			bool bOutputConsole = false,                  // 是否允许控制台日志输出
			bool bSync = true,                            // 文件日志是否采用异步方式
			size_t rotationSize = 1024 * 1024,            // 文件日志旋转尺寸			
			const char *prefix = nullptr,                 // 如果为nullptr,表示使用 module + "_" 为前缀;
			const char *extension = nullptr,              // 如果为nullptr,表示使用 .log 为前缀;
			const char *configPath = nullptr              // 如果为nullptr,表示使用 exe__LoggerConfig.ini为配置文件
		);
		~Logger();

		Logger(const Logger&) = delete;
		Logger& operator=(const Logger&) = delete;

		static void stop(); //存在异步日志时，必须调用此函数,且 此函数必须在main函数结束前调用
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




