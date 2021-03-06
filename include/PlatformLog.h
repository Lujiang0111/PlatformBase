﻿#ifndef PLATFORM_LOG_H_
#define PLATFORM_LOG_H_

#include "PlatformBaseApi.h"

#ifdef __cplusplus
extern "C"
{
#endif

	typedef void *PlatformLogHandle;

	/*****************************
	创建日志句柄
	logPath		[in]	日志存放路径
	clearMs		[in]	日志清理周期
	maxSize		[in]	日志最大存放空间，单位字节
	maxCount	[in]	单次处理的最大日志数，为0则由库内部自动设置
	return				日志句柄
	*****************************/
	PLATFORM_BASE_API PlatformLogHandle PlatformLogCreate(const char *logPath, size_t clearMs, size_t maxSize, size_t maxCount);

	/*****************************
	销毁创建日志句柄
	*****************************/
	PLATFORM_BASE_API void PlatformLogDestroy(PlatformLogHandle *hdl);

	/*****************************
	记录日志相关接口
	*****************************/
	PLATFORM_BASE_API void PlatformLogSend(PlatformLogHandle hdl, EPlatformLogLevel level, int needPrintScreen, const char *fileName, int fileLine, const char *fmt, ...);
	PLATFORM_BASE_API void PlatformLogSendVa(PlatformLogHandle hdl, EPlatformLogLevel level, int needPrintScreen, const char *fileName, int fileLine, const char *fmt, va_list vl);
	PLATFORM_BASE_API void PlatformLogSendContent(PlatformLogHandle hdl, EPlatformLogLevel level, int needPrintScreen, const char *fileName, int fileLine, const char *content);

#ifdef __cplusplus
}
#endif

#endif // !PLATFORM_LOG_H_