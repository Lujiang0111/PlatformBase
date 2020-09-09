#ifndef I_PLATFORM_LOG_H_
#define I_PLATFORM_LOG_H_

#include <vector>
#include <atomic>
#include <thread>
#include <mutex>
#include "Log/IPlatformLogCtx.h"

/*****************************
PlatformBase库内日志输出调用
*****************************/
void PBLogOut(EPlatformLogLevel level, const char *fmt, ...);

class IPlatformLog
{
public:
	IPlatformLog(const char *logPath, int64_t spanMs, int64_t clearMs, uint64_t maxLogSize, uint64_t maxQueLen);
	virtual ~IPlatformLog();

	bool Start();
	void Stop();

	void SendLog(EPlatformLogLevel level, bool needPrintScreen, const char *fileName, int fileLine, const char *fmt, va_list vl);
	void SendLog(EPlatformLogLevel level, bool needPrintScreen, const char *fileName, int fileLine, const char *content);

private:
	IPlatformLog() = delete;
	IPlatformLog(const IPlatformLog&) = delete;
	IPlatformLog& operator=(const IPlatformLog&) = delete;

	void Update(const IPlatformLogCtx *logCtx);
	void DoClear();

	static void WorkThreadEntry(void *hdl);
	void WorkThread();

private:
	static std::atomic<uint64_t> _id;

	std::string _logPath;
	int64_t _spanMs;
	int64_t _clearMs;
	uint64_t _maxLogSize;
	int64_t _maxQueLen;

	bool _isRunning;
	std::mutex _mutex;
	std::thread *_workThread;

	int _hoursCnt;
	FILE *_fp;

	IPlatformLogCtx _logCtxDummy;
	IPlatformLogCtx *_logCtxTail;
	int _logCtxCnt;
	std::vector<char> _contentBuf;
};

#endif // !I_PLATFORM_LOG_H_