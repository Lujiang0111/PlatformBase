#ifndef PLATFORM_LOG_H_
#define PLATFORM_LOG_H_

#include <stdint.h>
#include <stdarg.h>

#if defined(WIN32) || defined(_WINDLL)
#if defined(PLATFORM_BASE_EXPORT)
#define PLATFORM_BASE_API   __declspec(dllexport)
#else
#define PLATFORM_BASE_API   __declspec(dllimport)
#endif
#else
#define PLATFORM_BASE_API
#endif

#ifdef __cplusplus
extern "C"
{
#endif

	typedef enum PlatformLogLevel_
	{
		PL_LEVEL_DEBUG = 0,
		PL_LEVEL_INFO,
		PL_LEVEL_WARNING,
		PL_LEVEL_ERROR,
		PL_LEVEL_FATAL,
	}EPlatformLogLevel;

	typedef void *PlatformLogHandle;

	/*****************************
	������־���
	logPath		[in]	��־���·��
	spanMs		[in]	��־��ӡ����
	return				��־���
	*****************************/
	PLATFORM_BASE_API PlatformLogHandle PlatformLogCreate(const char *logPath, int64_t spanMs, int64_t clearMs, int64_t maxLogSize);

	/*****************************
	���ٴ�����־���
	*****************************/
	PLATFORM_BASE_API void PlatformLogDestroy(PlatformLogHandle *hdl);

	/*****************************
	��¼��־��ؽӿ�
	*****************************/
	PLATFORM_BASE_API void PlatformLogSend(PlatformLogHandle hdl, EPlatformLogLevel level, int needPrintScreen, const char *fileName, int fileLine, const char *fmt, ...);
	PLATFORM_BASE_API void PlatformLogSendVa(PlatformLogHandle hdl, EPlatformLogLevel level, int needPrintScreen, const char *fileName, int fileLine, const char *fmt, va_list vl);
	PLATFORM_BASE_API void PlatformLogSendContent(PlatformLogHandle hdl, EPlatformLogLevel level, int needPrintScreen, const char *fileName, int fileLine, const char *content);

	/*****************************
	����PlatformBase������־����ص�
	*****************************/
	PLATFORM_BASE_API void SetPlatformBaseLogSetCallback(void(*cb)(EPlatformLogLevel level, const char *fmt, va_list vl));

#ifdef __cplusplus
}
#endif

#endif // !PLATFORM_LOG_H_