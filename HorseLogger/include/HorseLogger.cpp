#include "stdafx.h"
#include "HorseLogger.h"

// #define BOOST_LOG_USE_CHAR
// #define BOOST_ALL_DYN_LINK 1
// #define BOOST_LOG_DYN_LINK 1
#include <windows.h>

#include <string>
#include <iostream>
#include <unordered_map>
#include <codecvt>

#include <boost/filesystem.hpp>  
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/lock_guard.hpp>

#include <boost/log/common.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/attributes/timer.hpp>
#include <boost/log/attributes/named_scope.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/support/date_time.hpp>

//日志窗口日志后台
#include <boost/log/sinks/debug_output_backend.hpp>

//异步日志start
//#include <boost/date_time/posix_time/posix_time.hpp>
//#include <boost/log/sinks/async_frontend.hpp>
#include <boost/log/sinks.hpp>
#include <boost/log/utility/record_ordering.hpp>
//异步日志end

namespace logging = boost::log;
namespace sinks = boost::log::sinks;
namespace attrs = boost::log::attributes;
namespace src = boost::log::sources;
namespace expr = boost::log::expressions;
namespace keywords = boost::log::keywords;

namespace horse_logger
{
	typedef src::severity_channel_logger_mt<severity_level, std::string> logger_t;
	typedef boost::shared_ptr<logger_t> logger_sptr;
	typedef boost::lock_guard<boost::mutex> lock_t;

	//字面常量存储在静态区中
	static const char* __COMMON = "common";

	/**
	原始的debug_output_backend不会自动 换行
	*/
	class my_debug_output_backend :public sinks::debug_output_backend
	{
		typedef sinks::debug_output_backend base_type;
	public:
		void consume(logging::BOOST_LOG_VERSION_NAMESPACE::record_view const& rec, string_type const& formatted_message)
		{
			base_type::consume(rec, formatted_message.c_str());
			base_type::consume(rec, "\n");
			//::OutputDebugStringA(formatted_message.c_str());
			//::OutputDebugStringA("\n");
		}
	};

	template< typename CharT, typename TraitsT >
	inline std::basic_ostream< CharT, TraitsT >& operator<< (
		std::basic_ostream< CharT, TraitsT >& strm, severity_level lvl)
	{
		static const char* const str[] =
		{
			"trace",
			"debug",
			"info",
			"warning",
			"error",
			"critical"

			//"T",
			//"D",
			//"I",
			//"W",
			//"E",
			//"C"
		};
		if (static_cast<std::size_t>(lvl) < (sizeof(str) / sizeof(*str)))
			strm << str[lvl];
		else
			strm << static_cast<int>(lvl);
		return strm;
	}

	//*********************************************************************************************************************************

	class Tool
	{
	public:
		static bool createDir(const std::string &dir)
		{
			if (boost::filesystem::exists(dir) == false)
			{
				if (!boost::filesystem::create_directories(dir))
					return false;
			}
			return true;
		}

	};

	class CharsetConvert
	{
	public:
		typedef boost::shared_ptr<std::wstring> wstring_sptr;
		typedef boost::shared_ptr<std::string> string_sptr;
		//字符串转化
		//返回智能指针，提高性能
		// MBCS->WCHAR
		static wstring_sptr str2wstr(const std::string &strMBCS, UINT nCodePage)
		{
			wstring_sptr spStr(new std::wstring());
			if (strMBCS.empty())
				return spStr;

			int nLength = ::MultiByteToWideChar(nCodePage, 0, strMBCS.c_str(), -1, NULL, 0);
			if (nLength <= 0)
				return spStr;
			else {
				spStr->resize(nLength - 1);
				MultiByteToWideChar(nCodePage, 0, strMBCS.c_str(), -1, &(spStr->at(0)), nLength);
				return spStr;
			}
		}

		static wstring_sptr str2wstr(const char *lpszMBCS, UINT nCodePage)
		{
			BOOST_ASSERT(lpszMBCS);
			wstring_sptr spStr(new std::wstring());
			if (lpszMBCS == nullptr)
				return spStr;

			int nLength = ::MultiByteToWideChar(nCodePage, 0, lpszMBCS, -1, NULL, 0);
			if (nLength <= 0)
				return spStr;
			else {
				spStr->resize(nLength - 1);
				MultiByteToWideChar(nCodePage, 0, lpszMBCS, -1, &(spStr->at(0)), nLength);
				return spStr;
			}
		}

		// WCHAR->MBCS
		static string_sptr wstr2str(const std::wstring &strWCHAR, UINT nCodePage)
		{
			string_sptr spStr(new std::string());
			if (strWCHAR.empty())
				return spStr;

			int nLength = ::WideCharToMultiByte(nCodePage, 0, strWCHAR.c_str(), -1, NULL, 0, NULL, NULL);
			if (nLength <= 0)
				return spStr;
			else {
				spStr->resize(nLength - 1);
				::WideCharToMultiByte(nCodePage, 0, strWCHAR.c_str(), -1, &(spStr->at(0)), nLength, NULL, NULL);
				return spStr;
			}
		}

		static string_sptr wstr2str(const wchar_t *lpszWCHAR, UINT nCodePage)
		{
			BOOST_ASSERT(lpszWCHAR);
			string_sptr spStr(new std::string());
			if (lpszWCHAR == nullptr)
				return spStr;

			int nLength = ::WideCharToMultiByte(nCodePage, 0, lpszWCHAR, -1, NULL, 0, NULL, NULL);
			if (nLength <= 0)
				return spStr;
			else {
				spStr->resize(nLength - 1);
				::WideCharToMultiByte(nCodePage, 0, lpszWCHAR, -1, &(spStr->at(0)), nLength, NULL, NULL);
				return spStr;
			}
		}

		static wstring_sptr gbk_utf16(const std::string& strSrc) {
			return str2wstr(strSrc, 936/*CP_GBK*/);
		}

		static wstring_sptr gbk_utf16(const char *strSrc) {
			return str2wstr(strSrc, 936/*CP_GBK*/);
		}

		static wstring_sptr utf8_utf16(const std::string& strSrc) {
			return str2wstr(strSrc, CP_UTF8);
		}

		static wstring_sptr utf8_utf16(const char *strSrc) {
			return str2wstr(strSrc, CP_UTF8);
		}

		static string_sptr utf16_utf8(const std::wstring& strSrc) {
			return wstr2str(strSrc, CP_UTF8);
		}

		static string_sptr utf16_utf8(const wchar_t *strSrc) {
			return wstr2str(strSrc, CP_UTF8);
		}

#define CP_GBK	936
		static string_sptr utf16_gbk(const std::wstring& strSrc) {
			return wstr2str(strSrc, CP_GBK);
		}

		static string_sptr utf16_gbk(const wchar_t *strSrc) {
			return wstr2str(strSrc, CP_GBK);
		}

		static string_sptr utf8_gbk(const std::string& strSrc) {
			return utf16_gbk(*(utf8_utf16(strSrc)));
		}

		static string_sptr utf8_gbk(const char *strSrc) {
			return utf16_gbk(*(utf8_utf16(strSrc)));
		}

		static string_sptr gbk_utf8(const std::string& strSrc) {
			return utf16_utf8(*(gbk_utf16(strSrc)));
		}

		static string_sptr gbk_utf8(const char *strSrc) {
			return utf16_utf8(*(gbk_utf16(strSrc)));
		}
	};

	typedef struct tagConfigInitData
	{
		std::string    configPath_;
		std::string    module_;
		severity_level min_level_;
		//日志总开关
		//bool           bEnableLogger_;
		//输出控制
		bool           bOutputConsole_;
		bool           bOutputLogcat_;
		bool           bOutputFile_;

		//是否采用异步
		bool           bSync_;

		//路径配置 [文件输出时]
		std::string    outputDir_;			// eg: d:test/
		std::string    prefix_;			    // eg: login_
		std::string    extension_; 		    // eg: .log 
		size_t         rotationSize_;		// eg: 1024*1024

		tagConfigInitData(const char *module = __COMMON)
			: module_(module)
			, min_level_(level_warning)
			, bOutputConsole_(false)
			, bOutputLogcat_(false)
			, bOutputFile_(true)
			, bSync_(true)
			, rotationSize_(1024 * 1024)
		{
			BOOST_ASSERT(module != nullptr && module[0] != '\0');
			if (module == nullptr || module[0] == '\0')
				module_ = __COMMON;

			configPath_ = getDefaultConfigPath();
			outputDir_ = "./logs/" + module_;
			prefix_ = module_ + "_";
			extension_ = ".log";
		}

		void setModule(const char * module)
		{
			if (module != nullptr && module[0] != '\0')
				module_ = module;
		}

		void setConfigPath(const char * configPath)
		{
			if (configPath != nullptr && configPath[0] != '\0')
				configPath_ = configPath;
		}

		void setOutputDir(const char * outputDir)
		{
			if (outputDir != nullptr && outputDir[0] != '\0')
				outputDir_ = outputDir;
		}

		void setPrefix(const char * prefix)
		{
			if (prefix != nullptr && prefix[0] != '\0')
				prefix_ = prefix;
			else
				prefix_ = module_ + "_";
		}

		void setExtension(const char * extension)
		{
			if (extension != nullptr && extension[0] != '\0')
				extension_ = extension;
		}

		void setMinLevel(severity_level level) { min_level_ = level; }
		void setOutputConsoleStatus(bool b) { bOutputConsole_ = b; }
		void setOutputLogcatStatus(bool b) { bOutputLogcat_ = b; }
		void setOutputFileStatus(bool b) { bOutputFile_ = b; }
		void setSyncStatus(bool b) { bSync_ = b; }
		void setRotationSize(size_t size) { rotationSize_ = size; }

		static std::string getDefaultConfigPath()
		{
			enum { buf_size = 512 };
			char szFileName[buf_size] = { 0 };
			::GetModuleFileNameA(NULL, szFileName, buf_size);

			std::string str(szFileName);
			size_t pos = str.find_last_of('.');
			BOOST_ASSERT(pos != std::string::npos);
			str = str.substr(0, pos);
			std::string defaultConfigPath = str + "_LoggerConfig.ini";   //eg: d:test/Helper.exe 其配置文件为 d:test/Helper__LoggerConfig.ini
			return defaultConfigPath;
		}

	}ConfigInitData;

	class Config
	{
	public:
		Config(const ConfigInitData &initData);
		~Config() {}
		inline bool isInitedByIni()const { return bInitedByIni_; }
		inline const std::string &module()const { return module_; }
		inline severity_level minLevel()const { return min_level_; }
		inline bool isEnableLogger()const { return bEnableLogger_ && (bOutputConsole_ == true || bOutputLogcat_ == true || bOutputFile_ == true); }
		inline bool isOutputConsole()const { return bEnableLogger_ && bOutputConsole_; }
		inline bool isOutputLogcat()const { return bEnableLogger_ && bOutputLogcat_; }
		inline bool isOutputFile()const { return bEnableLogger_ && bOutputFile_; }
		inline bool isSync()const { return bSync_; }

		inline const std::string& outputDir()const { return outputDir_; }
		inline const std::string& prefix()const { return prefix_; }
		inline const std::string& extension()const { return extension_; }
		inline size_t rotationSize()const { return rotationSize_; }
	private:
		void read();
	private:
		enum { buf_size = 512 };
		bool           bInitedByIni_;  //是否是通过 ini配置初始化的
		std::string    configPath_;
		std::string    module_;

		//等级控制
		severity_level min_level_;
		//日志总开关
		bool           bEnableLogger_;
		//输出控制
		bool           bOutputConsole_;
		bool           bOutputLogcat_;
		bool           bOutputFile_;

		//是否采用异步
		bool           bSync_;

		//路径配置 [文件输出时]
		std::string    outputDir_;			// eg: d:test/
		std::string    prefix_;			    // eg: login_
		std::string    extension_; 		    // eg: .log 
		size_t         rotationSize_;		// eg: 1024*1024

		static boost::mutex mutex_;
	};
	boost::mutex Config::mutex_;

	Config::Config(const ConfigInitData &initData)
		: bInitedByIni_(false)
		, configPath_(initData.configPath_)
		, module_(initData.module_)
		, min_level_(initData.min_level_)
		, bEnableLogger_(true)
		, bOutputConsole_(initData.bOutputConsole_)
		, bOutputLogcat_(initData.bOutputLogcat_)
		, bOutputFile_(initData.bOutputFile_)
		, bSync_(initData.bSync_)
		, outputDir_(initData.outputDir_)
		, prefix_(initData.prefix_)
		, extension_(initData.extension_)
		, rotationSize_(initData.rotationSize_)
	{
		BOOST_ASSERT(module_.empty() == false && configPath_.empty() == false);

		//确保defaultOutputDir_ 以 / 结尾
		if (outputDir_.back() == '\\' || outputDir_.back() == '/')
			;
		else
			outputDir_ += "/";

    //确保configPath_所在文件夹存在,确保configPath_以.ini为扩展名
    boost::filesystem::path path(configPath_);  
    if(path.extension().string()!=".ini")
      configPath_ += ".ini";
    Tool::createDir(path.parent_path().string());
    
		read();
	}

	void Config::read()
	{
		char szTemp[buf_size] = { 0 };

		//configPath_文件是否存在
		std::string defaultOutputDir = outputDir_;
		std::string defaultPrefix = prefix_;
		std::string defaultExtension = extension_;
		int defaultRotationSize = rotationSize_;

		lock_t lock(mutex_);  //多线程安全读写同一 ini文件

		bInitedByIni_ = ::GetPrivateProfileIntA(module_.c_str(), "inited", 0, configPath_.c_str()) == 1;
		if (!bInitedByIni_)
		{
			::WritePrivateProfileStringA(module_.c_str(), "inited", "1", configPath_.c_str());

			memset(szTemp, 0, sizeof(szTemp));
			_itoa_s(min_level_, szTemp, 10);
			::WritePrivateProfileStringA(module_.c_str(), "min_level", szTemp, configPath_.c_str());

			memset(szTemp, 0, sizeof(szTemp));
			_itoa_s(bEnableLogger_ ? 1 : 0, szTemp, 10);
			::WritePrivateProfileStringA(module_.c_str(), "enable_logger", szTemp, configPath_.c_str());

			memset(szTemp, 0, sizeof(szTemp));
			_itoa_s(bOutputConsole_ ? 1 : 0, szTemp, 10);
			::WritePrivateProfileStringA(module_.c_str(), "output_console", szTemp, configPath_.c_str());

			memset(szTemp, 0, sizeof(szTemp));
			_itoa_s(bOutputLogcat_ ? 1 : 0, szTemp, 10);
			::WritePrivateProfileStringA(module_.c_str(), "output_logcat", szTemp, configPath_.c_str());

			memset(szTemp, 0, sizeof(szTemp));
			_itoa_s(bOutputFile_ ? 1 : 0, szTemp, 10);
			::WritePrivateProfileStringA(module_.c_str(), "output_file", szTemp, configPath_.c_str());

			memset(szTemp, 0, sizeof(szTemp));
			_itoa_s(bSync_ ? 1 : 0, szTemp, 10);
			::WritePrivateProfileStringA(module_.c_str(), "sync", szTemp, configPath_.c_str());

			//
			memset(szTemp, 0, sizeof(szTemp));
			_itoa_s(defaultRotationSize, szTemp, 10);
			::WritePrivateProfileStringA(module_.c_str(), "rotation_size", szTemp, configPath_.c_str());

			::WritePrivateProfileStringA(module_.c_str(), "outputDir", defaultOutputDir.c_str(), configPath_.c_str());
			::WritePrivateProfileStringA(module_.c_str(), "prefix", defaultPrefix.c_str(), configPath_.c_str());
			::WritePrivateProfileStringA(module_.c_str(), "extension", defaultExtension.c_str(), configPath_.c_str());
		}
		else
		{
			int min_level = ::GetPrivateProfileIntA(module_.c_str(), "min_level", level_trace, configPath_.c_str());
			if (min_level < level_trace)
				min_level = level_trace;
			if (min_level > level_critical)
				min_level = level_critical;
			min_level_ = static_cast<severity_level>(min_level);
			bEnableLogger_ = ::GetPrivateProfileIntA(module_.c_str(), "enable_logger", bEnableLogger_, configPath_.c_str()) >= 1;
			bOutputConsole_ = ::GetPrivateProfileIntA(module_.c_str(), "output_console", bOutputConsole_, configPath_.c_str()) >= 1;
			bOutputLogcat_ = ::GetPrivateProfileIntA(module_.c_str(), "output_logcat", bOutputLogcat_, configPath_.c_str()) >= 1;
			bOutputFile_ = ::GetPrivateProfileIntA(module_.c_str(), "output_file", bOutputFile_, configPath_.c_str()) >= 1;
			bSync_ = ::GetPrivateProfileIntA(module_.c_str(), "sync", bSync_, configPath_.c_str()) == 1;

			rotationSize_ = ::GetPrivateProfileIntA(module_.c_str(), "rotation_size", defaultRotationSize, configPath_.c_str());

			memset(szTemp, 0, sizeof(szTemp));
			::GetPrivateProfileStringA(module_.c_str(), "outputDir", defaultOutputDir.c_str(), szTemp, sizeof(szTemp) / sizeof(szTemp[0]), configPath_.c_str());
			outputDir_ = szTemp;

			memset(szTemp, 0, sizeof(szTemp));
			::GetPrivateProfileStringA(module_.c_str(), "prefix", defaultPrefix.c_str(), szTemp, sizeof(szTemp) / sizeof(szTemp[0]), configPath_.c_str());
			prefix_ = szTemp;

			memset(szTemp, 0, sizeof(szTemp));
			::GetPrivateProfileStringA(module_.c_str(), "extension", defaultExtension.c_str(), szTemp, sizeof(szTemp) / sizeof(szTemp[0]), configPath_.c_str());
			extension_ = szTemp;
		}

		//确保正确
		bool bValid = false;
		//确保strDir以 / 结尾
		if (outputDir_.back() == '\\' || outputDir_.back() == '/')
			;
		else
			outputDir_ += "/";

		//确保prefix_开头,无 \ /	
		bValid = (prefix_.find('\\') == std::string::npos && prefix_.find('/') == std::string::npos);
		if (!bValid)
			prefix_ = defaultPrefix;

		//确保extension_以 . 开头,无 \ /		
		bValid = (extension_.find('\\') == std::string::npos && extension_.find('/') == std::string::npos);
		if (extension_.empty() == false && bValid)
		{
			if (extension_.front() == '.')
				;
			else
				extension_ = "." + extension_;
		}
		else
			extension_ += defaultExtension;

		//检测 outputDir_文件夹是否存在
		if (bEnableLogger_ && bOutputFile_)
		{
			bool bSuccess = Tool::createDir(outputDir_.c_str());
			BOOST_ASSERT(bSuccess);
		}
	}

	//*********************************************************************************************************************************
	class Impl
	{
		typedef boost::shared_ptr<Config> Config_sptr;

		//console
		typedef sinks::synchronous_sink<sinks::text_ostream_backend> sync_console_sink_t;
		typedef boost::shared_ptr<sync_console_sink_t> sync_console_sink_sptr;

		//logcat
		typedef sinks::synchronous_sink<my_debug_output_backend> sync_logcat_sink_t;
		typedef boost::shared_ptr<sync_logcat_sink_t> sync_logcat_sink_sptr;

		//logcat
		typedef sinks::synchronous_sink<sinks::text_file_backend> sync_file_sink_t;
		typedef boost::shared_ptr<sync_file_sink_t> sync_file_sink_sptr;;

		//file
		typedef sinks::asynchronous_sink<
			sinks::text_file_backend,
			sinks::bounded_ordering_queue<
			logging::attribute_value_ordering< unsigned int, std::less< unsigned int > >,
			10 * 1024,                        // queue no more than 10 * 1024 log records
			sinks::block_on_overflow     // wait until records are processed
			>
		> async_file_sink_t;
		typedef boost::shared_ptr<async_file_sink_t> async_file_sink_sptr;

	public:
		Impl(const ConfigInitData& initData)
			: logger_(keywords::channel = initData.module_)
		{
			BOOST_ASSERT(initData.module_.empty() == false);
			spConfig_ = boost::make_shared<Config>(initData);
			init();
		}

		~Impl()
		{
			stop();
			removeSinks();

#ifdef _DEBUG			
			::OutputDebugStringA("destroy Impl:");
			::OutputDebugStringA(spConfig_->module().c_str());
			::OutputDebugStringA("\n");
#endif
		}

		bool isInitedByIni()const { return spConfig_->isInitedByIni(); }

		void stop()
		{
			if (spSyncConsoleSink_)
			{
				spSyncConsoleSink_->flush();
			}

			if (spSyncLogcatSink_)
			{
				spSyncLogcatSink_->flush();
			}

			if (spSyncFileSink_)
			{
				BOOST_ASSERT(spConfig_->isOutputFile() && spConfig_->isSync() == true);
				spSyncFileSink_->flush();
			}

			if (spAsyncFileSink_)
			{
				BOOST_ASSERT(spConfig_->isOutputFile() && spConfig_->isSync() == false);

				try
				{
					OutputDebugStringA("stop begin module:");
					OutputDebugStringA(spConfig_->module().c_str());
					OutputDebugStringA("\n");

					spAsyncFileSink_->stop();
					spAsyncFileSink_->flush();

					OutputDebugStringA("stop end module:");
					OutputDebugStringA(spConfig_->module().c_str());
					OutputDebugStringA("\n");
				}
				catch (...)
				{

				}
			}
		}

	private:
		void removeSinks()
		{
			if (spSyncConsoleSink_)
			{
				logging::core::get()->remove_sink(spSyncConsoleSink_);
			}

			if (spSyncLogcatSink_)
			{
				logging::core::get()->remove_sink(spSyncLogcatSink_);
			}

			if (spSyncFileSink_)
			{
				BOOST_ASSERT(spConfig_->isOutputFile() && spConfig_->isSync() == true);
				logging::core::get()->remove_sink(spSyncFileSink_);
			}

			if (spAsyncFileSink_)
			{
				BOOST_ASSERT(spConfig_->isOutputFile() && spConfig_->isSync() == false);
				logging::core::get()->remove_sink(spAsyncFileSink_);
			}
		}

		void init()
		{
			add_console_log(spConfig_->module());
			add_debug_output_log(spConfig_->module());
			add_file_log(spConfig_->outputDir(), spConfig_->prefix(), spConfig_->extension(), spConfig_->minLevel(), spConfig_->module());

			logging::add_common_attributes();
			logging::core::get()->add_global_attribute("Uptime", attrs::timer());
			logging::core::get()->add_global_attribute("RecordID", attrs::counter< unsigned int >());
		}

	public:
		void log(severity_level level, const char *pMsg)
		{
			BOOST_ASSERT(pMsg);
			if (pMsg == nullptr)
				return;

			if (spConfig_->isEnableLogger() == false)
				return;

			if (spConfig_->isOutputFile() && spConfig_->isSync() == false)
			{
				//BOOST_LOG_SCOPED_THREAD_TAG("ThreadID", boost::this_thread::get_id()); //异步的时候添加ThreadID
				BOOST_LOG_SEV(logger_, level) << pMsg;
			}
			else
				BOOST_LOG_SEV(logger_, level) << pMsg;

		}

		inline severity_level minLevel()
		{
			return spConfig_->minLevel();
		}

		inline  const std::string & module()const
		{
			return spConfig_->module();
		}

		inline bool isEnableLogger()const { return spConfig_->isEnableLogger(); }
	private:
		void add_console_log(const std::string &channel)
		{
			if (spConfig_->isOutputConsole() == false)
				return;
			auto level = spConfig_->minLevel();

			spSyncConsoleSink_ = logging::add_console_log(std::clog,
				keywords::filter = expr::attr< severity_level >("Severity") >= level && expr::attr<std::string>("Channel") == channel,
				keywords::format = expr::stream
				<< "[" << expr::attr< unsigned int >("RecordID")
				<< "][" << expr::format_date_time< boost::posix_time::ptime >("TimeStamp", "%Y-%m-%d %H:%M:%S.%f")
				<< "][" << expr::format_date_time< attrs::timer::value_type >("Uptime", "%O:%M:%S")
				<< "][" << expr::attr< boost::log::aux::thread::id >("ThreadID")
				<< "]<" << expr::attr< severity_level >("Severity")
				<< "> " << expr::message
			);
		}

		void add_debug_output_log(const std::string &channel)
		{
#if defined(_WIN32) && defined(_DEBUG)
			if (spConfig_->isOutputLogcat() == false)
				return;

			//typedef sinks::debug_output_backend backend_t;
			typedef my_debug_output_backend backend_t;
			auto spBackend = boost::make_shared<backend_t>(); // 注意使用boost::make_shared

			typedef sinks::synchronous_sink< backend_t > sink_t;
			auto spSink = boost::make_shared< sink_t >(spBackend); // 注意使用boost::make_shared

			auto level = spConfig_->minLevel();
			spSink->set_filter(
				expr::is_debugger_present()
				&& expr::attr< severity_level >("Severity") >= level
				&& expr::attr<std::string>("Channel") == channel
			);

			spSink->set_formatter(
				expr::stream
				<< "[" << expr::attr< unsigned int >("RecordID")
				<< "][" << expr::format_date_time< boost::posix_time::ptime >("TimeStamp", "%Y-%m-%d %H:%M:%S.%f")
				<< "][" << expr::format_date_time< attrs::timer::value_type >("Uptime", "%O:%M:%S")
				<< "][" << expr::attr< boost::log::aux::thread::id >("ThreadID")
				<< "]<" << expr::attr< severity_level >("Severity")
				<< "> " << expr::message
			);

			logging::core::get()->add_sink(spSink);
			spSyncLogcatSink_ = spSink;
#endif
		}

		void add_file_log(const std::string &dir, const std::string &prefix, const std::string &extension, severity_level level, const std::string &channel)  //eg dir: D:/test/     prefix: moudle_one_    extension:log
		{
			BOOST_ASSERT(dir.empty() == false && prefix.empty() == false && extension.empty() == false);

			if (spConfig_->isOutputFile() == false)
				return;

			std::stringstream filePath;
			filePath << dir.c_str() << prefix << "%Y%m%d_%N_" << ::GetCurrentProcessId() << extension.c_str();

			//同步日志
			if (spConfig_->isSync())
			{
				spSyncFileSink_ = logging::add_file_log
				(
					keywords::open_mode = std::ios::app,
					keywords::file_name = filePath.str(),
					keywords::rotation_size = spConfig_->rotationSize(),  //1024*1024
					keywords::time_based_rotation = sinks::file::rotation_at_time_point(0, 0, 0),
					keywords::auto_flush = true,   //每写一条日志，就flush
					keywords::filter = expr::attr< severity_level >("Severity") >= level && expr::attr<std::string>("Channel") == channel,
					keywords::format = expr::stream
					<< "[" << expr::attr< unsigned int >("RecordID")
					<< "][" << expr::format_date_time< boost::posix_time::ptime >("TimeStamp", "%Y-%m-%d %H:%M:%S.%f")
					<< "][" << expr::format_date_time< attrs::timer::value_type >("Uptime", "%O:%M:%S")
					<< "][" << expr::attr< boost::log::aux::thread::id >("ThreadID")
					<< "]<" << expr::attr< severity_level >("Severity")
					<< "> " << expr::message
				);
			}
			else
			{
				//异步日志
				boost::shared_ptr< sinks::text_file_backend > spBackend =
					boost::make_shared< sinks::text_file_backend >(
						keywords::open_mode = std::ios::app,
						keywords::file_name = filePath.str(),
						keywords::rotation_size = spConfig_->rotationSize(),  //1024*1024
						keywords::time_based_rotation = sinks::file::rotation_at_time_point(0, 0, 0)
						//,keywords::auto_flush = true   //每写一条日志，就flush
						);

				boost::shared_ptr< async_file_sink_t > spSink(new async_file_sink_t(
					spBackend,
					// We'll apply record ordering to ensure that records from different threads go sequentially in the file
					keywords::order = logging::make_attr_ordering("RecordID", std::less< unsigned int >())));

				spSink->set_filter(expr::attr< severity_level >("Severity") >= level && expr::attr<std::string>("Channel") == channel);
				spSink->set_formatter
				(
					expr::stream
					<< "[" << expr::attr< unsigned int >("RecordID")
					<< "][" << expr::format_date_time< boost::posix_time::ptime >("TimeStamp", "%Y-%m-%d %H:%M:%S.%f")
					<< "][" << expr::format_date_time< attrs::timer::value_type >("Uptime", "%O:%M:%S")
					//<< "][" << expr::attr< boost::thread::id >("ThreadID")
					<< "][" << expr::attr< boost::log::aux::thread::id >("ThreadID")
					<< "]<" << expr::attr< severity_level >("Severity")
					<< "> " << expr::message
				);

				logging::core::get()->add_sink(spSink);
				spAsyncFileSink_ = spSink;
			}

		}

	private:
		Config_sptr            spConfig_;
		logger_t               logger_;
		sync_console_sink_sptr spSyncConsoleSink_;
		sync_logcat_sink_sptr  spSyncLogcatSink_;
		sync_file_sink_sptr    spSyncFileSink_;
		async_file_sink_sptr   spAsyncFileSink_;
	};
	typedef boost::shared_ptr<Impl> Impl_sptr;
	typedef boost::weak_ptr<Impl>   Impl_wptr;

	//******************************************************************************************************************************************
	//******************************************************************************************************************************************	
	class ImplCenter;
	typedef boost::shared_ptr<ImplCenter> ImplCenter_sptr;
	typedef boost::weak_ptr<ImplCenter>   ImplCenter_wptr;
	class ImplCenter
	{
		typedef ImplCenter this_type;
	private:
		ImplCenter()
		{
#ifdef _DEBUG
			::OutputDebugStringA("create ImplCenter \n");
#endif
		}

	public:
		~ImplCenter()
		{
#ifdef _DEBUG
			::OutputDebugStringA("destroy ImplCenter \n");
#endif
		}
	public:
		static ImplCenter_sptr instance()
		{
			if (spInstance_ == nullptr)
			{
				lock_t lock(mutexInstance_);
				if (spInstance_ == nullptr)
					spInstance_ = ImplCenter_sptr(new this_type());
			}

			BOOST_ASSERT(spInstance_);
			return spInstance_;
		}

		Impl_sptr getImpl(const ConfigInitData &initData)
		{
			lock_t lock(mutexImps_);
			return getImplWithoutLock(initData);
		}

		bool removeImpl(const std::string &module)
		{
			lock_t lock(mutexImps_);
			return removeImplWithoutLock(module);
		}

		void show()
		{
			lock_t lock(mutexImps_);
			std::cout << "ImplCenter-addr:" << this << std::endl;
			std::cout << "ImplCenter-size:" << impls_.size() << std::endl;
			for (auto &elem : impls_)
			{
				auto spImpl = elem.second.lock();
				if (spImpl)
					std::cout << "module:" << spImpl->module() << std::endl;
			}
		}

		void stop()
		{
			lock_t lock(mutexImps_);
			for (auto &elem : impls_)
			{
				auto spImpl = elem.second.lock();
				if (spImpl)
					spImpl->stop();
			}
		}

		static void destroy()
		{
			spInstance_.reset();
		}

	private:
		Impl_sptr getImplWithoutLock(const ConfigInitData &initData)
		{
			auto itr = impls_.find(initData.module_);
			Impl_sptr spImpl;
			if (itr != impls_.end())
			{
				spImpl = itr->second.lock();
			}

			if (spImpl == nullptr)
			{
#if defined(_WIN32) && defined(_DEBUG)

				enum { buf_size = 256 };
				char szTemp[buf_size] = { 0 };
				_snprintf_s(szTemp, buf_size, "********************  horse logger info -- create logger:[%s] !!!  ******************** \n", initData.module_.c_str());
				::OutputDebugStringA(szTemp);
#endif

				spImpl = boost::make_shared<Impl>(initData);
				impls_.insert(std::make_pair(initData.module_, spImpl));
			}
			BOOST_ASSERT(spImpl);
			return spImpl;
		}

		bool removeImplWithoutLock(const std::string &module)
		{
			auto itr = impls_.find(module);
			if (itr != impls_.end())
			{
				impls_.erase(itr);
			}
			return true;
		}

	private:
		boost::mutex mutexImps_;
		//std::unordered_map<std::string, Impl_sptr> impls_;
		std::unordered_map<std::string, Impl_wptr> impls_;

		static boost::mutex mutexInstance_;
		static ImplCenter_sptr spInstance_;
	};
	boost::mutex ImplCenter::mutexInstance_;
	ImplCenter_sptr ImplCenter::spInstance_;


	//******************************************************************************************************************************************
	//为何在此包装一次?
	//目的：不让Logger类直接拥有 Impl_sptr成员 [避免dll编译时的警告]
	class Logger::Impl
	{
	public:
		Impl(const ConfigInitData &initData)
		{
			BOOST_ASSERT(initData.module_.empty() == false);
			spImpl_ = ImplCenter::instance()->getImpl(initData);
			BOOST_ASSERT(spImpl_);
		}

		~Impl()
		{
			//ImplCenter::instance()->removeImpl(spImpl_->module()); //不能在这里移除
		}

		static void stop()
		{
			ImplCenter::instance()->stop();
		}

		static void show()
		{
			ImplCenter::instance()->show();
		}

		static void destroy()
		{
			ImplCenter::destroy();
		}

		inline void log(severity_level level, const char *pMsg)
		{
			spImpl_->log(level, pMsg);
		}

		inline severity_level minLevel()
		{
			return spImpl_->minLevel();
		}

		inline bool isEnableLogger()const { return spImpl_->isEnableLogger(); }
	private:
		Impl_sptr spImpl_;
	};

	Logger::Logger()
	{
		ConfigInitData initData(__COMMON);
		pImpl_ = new Logger::Impl(initData);
	}

	Logger::Logger(
		const char *module,
		const char *outputDir /*= nullptr*/,
		severity_level min_level /*= level_warning*/,
		bool bOutputFile /*= true*/,
		bool bOutputLogcat /*= false*/,
		bool bOutputConsole/* = false*/,
		bool bSync /*= true*/,
		size_t rotationSize /*= 1024 * 1024*/,
		const char *prefix /*= nullptr*/,
		const char *extension /*= nullptr*/,
		const char *configPath/* = nullptr*/
	)
	{
		BOOST_ASSERT(module != nullptr && module[0] != '\0');
		ConfigInitData initData(module);
		initData.setConfigPath(configPath);
		initData.setOutputDir(outputDir);
		initData.setPrefix(prefix);
		initData.setExtension(extension);
		initData.setMinLevel(min_level);
		initData.setOutputConsoleStatus(bOutputConsole);
		initData.setOutputLogcatStatus(bOutputLogcat);
		initData.setOutputFileStatus(bOutputFile);
		initData.setSyncStatus(bSync);
		initData.setRotationSize(rotationSize);

		pImpl_ = new Logger::Impl(initData);
	}

	Logger::~Logger()
	{
		if (pImpl_)
		{
			delete pImpl_;
			pImpl_ = nullptr;
		}
	}

	void Logger::stop()
	{
		Logger::Impl::stop();
	}

	void Logger::show()
	{
		Logger::Impl::show();
	}

	void Logger::destroy()
	{
		Logger::Impl::destroy();
	}

	severity_level Logger::minLevel() const
	{
		return pImpl_->minLevel();
	}

	void Logger::log(severity_level level, const char * fmt, ...)
	{
		if (pImpl_->minLevel() > level)
			return;

		enum { stack_buf_size = 1024 };
		va_list args;
		va_start(args, fmt);
		int length = _vscprintf(fmt, args);
		va_end(args);

		char szStackBuf[stack_buf_size] = { 0 };
		char *pHeapBuf = nullptr;

		char *pMsg = nullptr;
		size_t count = length + 1;
		if (count <= stack_buf_size)
		{
			pMsg = szStackBuf;
		}
		else
		{
			pHeapBuf = new char[count];
			memset(pHeapBuf, 0, count * sizeof(char));
			pMsg = pHeapBuf;
		}

		va_start(args, fmt);
		_vsnprintf_s(pMsg, count, _TRUNCATE, fmt, args);
		va_end(args);

		//打印日志
		pImpl_->log(level, pMsg);

		if (pHeapBuf)
		{
			delete pHeapBuf;
			pHeapBuf = nullptr;
		}
	}

	//宽字节 转为 utf-8 进行输出日志
	void Logger::log(severity_level level, const wchar_t * fmt, ...)
	{
		if (pImpl_->isEnableLogger() == false)
			return;

		if (pImpl_->minLevel() > level)
			return;

		enum { stack_buf_size = 1024 };
		va_list args;
		va_start(args, fmt);
		int length = _vscwprintf(fmt, args);
		va_end(args);

		wchar_t szStackBuf[stack_buf_size] = { 0 };
		wchar_t *pHeapBuf = nullptr;

		wchar_t *pMsg = nullptr;
		size_t count = length + 1;
		if (count <= stack_buf_size)
		{
			pMsg = szStackBuf;
		}
		else
		{
			pHeapBuf = new wchar_t[count];
			memset(pHeapBuf, 0, count * sizeof(wchar_t));
			pMsg = pHeapBuf;
		}

		va_start(args, fmt);
		_vsnwprintf_s(pMsg, count, _TRUNCATE, fmt, args);
		va_end(args);

		auto spMsg = CharsetConvert::utf16_utf8(pMsg);  //使用utf-8保存
		//auto spMsg = CharsetConvert::utf16_gbk(pMsg);

		//打印日志
		pImpl_->log(level, spMsg->c_str());

		if (pHeapBuf)
		{
			delete pHeapBuf;
			pHeapBuf = nullptr;
		}
	}

}