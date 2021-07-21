#include <stdlib.h>
#include <iostream>
#include <vector>
#include <thread>
#include "PlatformLog.h"
#include "TestLog.h"

#define log_printf(logHdl, level, fmt, ...)\
	{\
		LogOut(logHdl, level, __FILE__, __LINE__, fmt, ##__VA_ARGS__);\
	}

extern bool bAppStart;

static void LogOut(PlatformLogHandle hdl, EPlatformLogLevel level, const char *fileName, int fileLine, const char *fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	PlatformLogSendVa(hdl, level, 0, fileName, fileLine, fmt, va);
	va_end(va);
}

void TestLogLimit(int argc, char *argv[])
{
	PlatformLogHandle logHdl = PlatformLogCreate("Log", 10000, 1 << 27, 10000);
	int64_t cnt = 0;
	while (bAppStart)
	{
		//std::this_thread::sleep_for(std::chrono::nanoseconds(100));

		++cnt;
		log_printf(logHdl, PL_LEVEL_DEBUG, "log_%lld", cnt);
	}
	PlatformLogDestroy(&logHdl);
}

void LogThread(PlatformLogHandle logHdl, int i)
{
	int64_t cnt = 0;
	std::chrono::time_point<std::chrono::steady_clock> lastTime = std::chrono::steady_clock::now();
	while (bAppStart)
	{
		std::this_thread::sleep_for(std::chrono::nanoseconds(100));
		std::chrono::time_point<std::chrono::steady_clock> nowTime = std::chrono::steady_clock::now();
		if (nowTime < lastTime + std::chrono::nanoseconds(1000000))
		{
			continue;
		}
		lastTime = nowTime;

		++cnt;
		log_printf(logHdl, PL_LEVEL_DEBUG, "thread_%d_%lld", i, cnt);
	}
}

void TestMultiThreadLogLimit(int argc, char *argv[])
{
	int threadCnt;

	if (argc >= 3)
	{
		threadCnt = atoi(argv[2]);
	}
	else
	{
		std::cout << "Input number of thread:";
		std::cin >> threadCnt;
	}

	std::cout << "number of thread:" << threadCnt << std::endl;

	PlatformLogHandle logHdl = PlatformLogCreate("Log", 10000, 1 << 27, 10000);


	std::vector<std::thread> vecThread(threadCnt);
	for (int i = 0; i < threadCnt; ++i)
	{
		vecThread[i] = std::thread(LogThread, logHdl, i);
	}

	while (bAppStart)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}

	for (int i = 0; i < threadCnt; ++i)
	{
		if (vecThread[i].joinable())
		{
			vecThread[i].join();
		}
	}

	PlatformLogDestroy(&logHdl);
}