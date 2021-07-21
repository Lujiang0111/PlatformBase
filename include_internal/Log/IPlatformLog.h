#ifndef I_PLATFORM_LOG_H_
#define I_PLATFORM_LOG_H_

#include <thread>
#include <mutex>
#include <condition_variable>
#include "PlatformLog.h"
#include "Base/IPlatformBaseApi.h"
#include "Log/IPlatformLogCtx.h"

class PlatformLog
{
public:
	PlatformLog(const char *logPath, size_t clearMs, size_t maxSize, size_t maxCount);
	virtual ~PlatformLog();

	bool Start();
	void Stop();

	void SendLog(EPlatformLogLevel level, bool needPrintScreen, const char *fileName, int fileLine, const char *fmt, va_list vl);

private:
	PlatformLog() = delete;
	PlatformLog(const PlatformLog&) = delete;
	PlatformLog& operator=(const PlatformLog&) = delete;

	void Update(const PlatformLogCtx *logCtx);
	void DoClear();

	void WorkThread();

private:
	static size_t _id;

	std::string _logPath;
	size_t _clearMs;
	size_t _maxSize;
	size_t _maxCount;

	bool _bWorkThread;
	std::thread _workThread;

	int _logHours;
	FILE *_fp;

	bool _isLogFull;

	PlatformLogCtx _logDummy;
	PlatformLogCtx *_logTail;
	size_t _logInCnt;
	int _isLogReady;
	std::mutex _mutLog;
	std::condition_variable _condLog;
};

#endif // !I_PLATFORM_LOG_H_