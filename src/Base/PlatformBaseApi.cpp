#include <stdio.h>
#include "PlatformBaseApi.h"

void PBLogCbDefault(EPlatformLogLevel level, const char *fmt, va_list vl)
{
	vprintf(fmt, vl);
	printf("\n");
}

static void(*pBLogCb)(EPlatformLogLevel level, const char *fmt, va_list vl) = PBLogCbDefault;

void PBLogOut(EPlatformLogLevel level, const char *fmt, ...)
{
	va_list vl;
	va_start(vl, fmt);
	void(*cb)(EPlatformLogLevel level, const char *fmt, va_list vl) = pBLogCb;
	if (cb)
	{
		cb(level, fmt, vl);
	}
	va_end(vl);
}

void PlatformBaseLogSetCallback(void(*cb)(EPlatformLogLevel level, const char *fmt, va_list vl))
{
	pBLogCb = cb;
}