#ifndef PLATFORM_BASE_API_H_
#define PLATFORM_BASE_API_H_

#include <stdint.h>
#include <stdarg.h>

#if defined(WIN32) || defined(_WINDLL)
#if defined(PLATFORM_BASE_EXPORT)
#define PLATFORM_BASE_API	__declspec(dllexport)
#else
#define PLATFORM_BASE_API	__declspec(dllimport)
#endif
#else
#define PLATFORM_BASE_API
#endif

typedef enum PlatformLogLevel_
{
	PL_LEVEL_DEBUG = 0,
	PL_LEVEL_INFO,
	PL_LEVEL_WARNING,
	PL_LEVEL_ERROR,
	PL_LEVEL_FATAL,
}EPlatformLogLevel;

/*****************************
设置PlatformBase库内日志输出回调
*****************************/
PLATFORM_BASE_API void SetPlatformBaseLogSetCallback(void(*cb)(EPlatformLogLevel level, const char *fmt, va_list vl));

#endif // !PLATFORM_BASE_API_H_