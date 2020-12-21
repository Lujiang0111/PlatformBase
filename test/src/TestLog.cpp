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
	PlatformLogSendVa(hdl, level, 1, fileName, fileLine, fmt, va);
	va_end(va);
}

void TestLogLimit(int argc, char *argv[])
{
	PlatformLogHandle logHdl = PlatformLogCreate("Log", 50, 10000, 1 << 27);
	int64_t cnt = 0;
	while (bAppStart)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));

		++cnt;
		log_printf(logHdl, PL_LEVEL_DEBUG, "%s_%lld", "log", cnt);
	}
	PlatformLogDestroy(&logHdl);
}