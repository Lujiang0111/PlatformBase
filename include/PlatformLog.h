#ifndef PLATFORM_LOG_H_
#define PLATFORM_LOG_H_

#include "PlatformBaseApi.h"

#ifdef __cplusplus
extern "C"
{
#endif

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

#ifdef __cplusplus
}
#endif

#endif // !PLATFORM_LOG_H_