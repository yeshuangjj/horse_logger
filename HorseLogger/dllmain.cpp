// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "stdafx.h"
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

#ifdef _TEST

#include "../include/HorseLogger.h"
#include <string>
#include <iostream>

#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/atomic.hpp>

const int g_count = 1000;
class TestCase
{
	typedef boost::shared_ptr<horse_logger::Logger> Logger_sptr;
public:
	TestCase()
		:index_(0)
	{
		init();
	}

	void init()
	{
		spLg1_ = boost::make_shared<horse_logger::Logger>("module1");
		spLg2_ = boost::make_shared<horse_logger::Logger>("module2");
		spLg3_ = boost::make_shared<horse_logger::Logger>("module3");
		spLg4_ = boost::make_shared<horse_logger::Logger>("module4");
	}

	void module1(const std::string &tag)
	{
		static boost::atomic<int> index;
		for (int i = 0; i < g_count; ++i)
		{
			++index;
			spLg1_->trace("index:%d %s 时间：%d-%d-%d__%s", index, tag.c_str(), 1, 2, 3, "好的");
		}
	}

	void module2(const std::string &tag)
	{
		static boost::atomic<int> index;
		for (int i = 0; i < g_count; ++i)
		{
			++index;
			spLg2_->trace("index:%d %s 时间：%d-%d-%d__%s", index, tag.c_str(), 1, 2, 3, "好的");
		}
	}

	void module3(const std::string &tag)
	{
		static boost::atomic<int> index;
		for (int i = 0; i < g_count; ++i)
		{
			++index;
			spLg3_->trace("index:%d %s 时间：%d-%d-%d__%s", index, tag.c_str(), 1, 2, 3, "好的");
		}
	}

	void module4(const std::string &tag)
	{
		static boost::atomic<int> index;
		for (int i = 0; i < g_count; ++i)
		{
			++index;
			spLg4_->trace("index:%d %s 时间：%d-%d-%d__%s", index, tag.c_str(), 1, 2, 3, "好的");
		}
	}
private:
	Logger_sptr spLg1_;
	Logger_sptr spLg2_;
	Logger_sptr spLg3_;
	Logger_sptr spLg4_;

	boost::atomic<int> index_;
};

int main()
{
	TestCase obj;
	boost::thread_group thdGroup;
	thdGroup.create_thread(boost::bind(&TestCase::module1, &obj, "《module1[1]》"));
	thdGroup.create_thread(boost::bind(&TestCase::module1, &obj, "《module1[2]》"));
	thdGroup.create_thread(boost::bind(&TestCase::module1, &obj, "《module1[3]》"));

	thdGroup.create_thread(boost::bind(&TestCase::module2, &obj, "《module2[1]》"));
	thdGroup.create_thread(boost::bind(&TestCase::module2, &obj, "《module2[2]》"));
	thdGroup.create_thread(boost::bind(&TestCase::module2, &obj, "《module2[3]》"));

	thdGroup.create_thread(boost::bind(&TestCase::module3, &obj, "《module3[1]》"));
	thdGroup.create_thread(boost::bind(&TestCase::module3, &obj, "《module3[2]》"));
	thdGroup.create_thread(boost::bind(&TestCase::module3, &obj, "《module3[3]》"));

	thdGroup.create_thread(boost::bind(&TestCase::module4, &obj, "《module4[1]》"));
	thdGroup.create_thread(boost::bind(&TestCase::module4, &obj, "《module4[2]》"));
	thdGroup.create_thread(boost::bind(&TestCase::module4, &obj, "《module4[3]》"));

	std::system("pause");

	HORSE_TRACE("tiankong天空");
	HORSE_TRACE("haolan好蓝：%d-%lf", 3, 0.5);


	return 0;
}
#endif

