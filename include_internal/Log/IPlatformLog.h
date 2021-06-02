#ifndef I_PLATFORM_LOG_H_
#define I_PLATFORM_LOG_H_

#include <vector>
#include <atomic>
#include <thread>
#include <mutex>
#include "PlatformLog.h"
#include "Base/IPlatformBaseApi.h"
#include "Log/IPlatformLogCtx.h"

class PlatformLog
{
public:
	PlatformLog(const char *logPath, size_t spanMs, size_t clearMs, size_t maxLogSize, size_t maxQueLen);
	virtual ~PlatformLog();

	bool Start();
	void Stop();

	void SendLog(EPlatformLogLevel level, bool needPrintScreen, const char *fileName, int fileLine, const char *fmt, va_list vl);
	void SendLog(EPlatformLogLevel level, bool needPrintScreen, const char *fileName, int fileLine, const char *content);

private:
	PlatformLog() = delete;
	PlatformLog(const PlatformLog&) = delete;
	PlatformLog& operator=(const PlatformLog&) = delete;

	static size_t IncreaseId();

	void Update(const PlatformLogCtx *logCtx);
	void DoClear();

	static void WorkThreadEntry(void *hdl);
	void WorkThread();

private:
	static std::atomic<size_t> _id;

	std::string _logPath;
	size_t _spanMs;
	size_t _clearMs;
	size_t _maxLogSize;
	size_t _maxQueLen;

	bool _isRunning;
	std::mutex _mutex;
	std::thread *_workThread;

	int _logHours;
	FILE *_fp;

	PlatformLogCtx _logCtxDummy;
	PlatformLogCtx *_logCtxTail;
	size_t _logCtxCnt;

	std::vector<char> _contentBuf;
};

#endif // !I_PLATFORM_LOG_H_