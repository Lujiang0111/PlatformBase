#include <algorithm>
#include "File/IPlatformFile.h"
#include "Log/IPlatformLog.h"

std::atomic<uint64_t> IPlatformLog::_id(0);

void PBLogCbDefault(EPlatformLogLevel level, const char *fmt, va_list vl)
{
	vprintf(fmt, vl);
	printf("\n");
}

static void(*pBLogCb)(EPlatformLogLevel level, const char *fmt, va_list vl) = PBLogCbDefault;

void SetPlatformBaseLogSetCallback(void(*cb)(EPlatformLogLevel level, const char *fmt, va_list vl))
{
	pBLogCb = cb;
}

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

IPlatformLog::IPlatformLog(const char *logPath, int64_t spanMs, int64_t clearMs, uint64_t maxLogSize, uint64_t maxQueLen)
	: _logCtxDummy(0, PL_LEVEL_DEBUG, true, NULL, 0, NULL)
{
	if (logPath)
	{
		_logPath = logPath;
	}
	else
	{
		_logPath = "Log";
	}

	std::replace(_logPath.begin(), _logPath.end(), PATH_SPLIT_CHAR_OTHER, PATH_SPLIT_CHAR);
	if (PATH_SPLIT_CHAR != _logPath[_logPath.length() - 1])
	{
		_logPath += PATH_SPLIT_CHAR;
	}

	if (spanMs > 0)
	{
		_spanMs = spanMs;
	}
	else
	{
		// 默认50ms打印一次日志
		_spanMs = 50;
	}

	if (clearMs > 0)
	{
		_clearMs = clearMs;
	}
	else
	{
		// 默认一分钟清理一次日志
		_clearMs = 60000;
	}

	if (maxLogSize > 0)
	{
		_maxLogSize = maxLogSize;
	}
	else
	{
		// 默认日志储存大小为5G
		_maxLogSize = 5LL << 30;
	}

	if (maxQueLen > 0)
	{
		_maxQueLen = maxQueLen;
	}
	else
	{
		// 默认日志队列最大长度为10000
		_maxQueLen = 10000;
	}
	
	_isRunning = false;
	_workThread = NULL;
	_hoursCnt = 0;
	_fp = NULL;

	_logCtxTail = &_logCtxDummy;
	_logCtxCnt = 0;
}

IPlatformLog::~IPlatformLog()
{
	Stop();

	IPlatformLogCtx *head = _logCtxDummy.GetNext();
	while (head)
	{
		IPlatformLogCtx *oldLogCtx = head;
		head = head->GetNext();
		delete oldLogCtx;
	}
}

bool IPlatformLog::Start()
{
	if (_isRunning)
	{
		return true;
	}

	_isRunning = true;
	_workThread = new std::thread(WorkThreadEntry, this);

	return true;
}

void IPlatformLog::Stop()
{
	if (!_isRunning)
	{
		return;
	}
	_isRunning = false;

	if (_workThread)
	{
		_workThread->join();
		delete _workThread;
		_workThread = NULL;
	}
}

void IPlatformLog::SendLog(EPlatformLogLevel level, bool needPrintScreen, const char *fileName, int fileLine, const char *fmt, va_list vl)
{
	std::lock_guard<std::mutex> lock(_mutex);
	if (_logCtxCnt >= _maxQueLen)
	{
		_logCtxTail->AddLostCnt();
		return;
	}

	while (true)
	{
		va_list vp;
		va_copy(vp, vl);
		int ret = vsnprintf(&_contentBuf[0], _contentBuf.size(), fmt, vp);
		va_end(vp);
		if ((ret >= 0) && (ret < _contentBuf.size()))
		{
			break;
		}

		if (ret > 0)
		{
			_contentBuf.resize(ret + 1);
		}
		else
		{
			return;
		}
	}

	++_id;
	IPlatformLogCtx *logCtx = new IPlatformLogCtx(_id, level, needPrintScreen, fileName, fileLine, &_contentBuf[0]);
	_logCtxTail->SetNext(logCtx);
	_logCtxTail = logCtx;
	++_logCtxCnt;
}

void IPlatformLog::SendLog(EPlatformLogLevel level, bool needPrintScreen, const char *fileName, int fileLine, const char *content)
{
	std::lock_guard<std::mutex> lock(_mutex);

	if (_logCtxCnt >= _maxQueLen)
	{
		_logCtxTail->AddLostCnt();
		return;
	}

	++_id;
	IPlatformLogCtx *logCtx = new IPlatformLogCtx(_id, level, needPrintScreen, fileName, fileLine, content);
	_logCtxTail->SetNext(logCtx);
	_logCtxTail = logCtx;
	++_logCtxCnt;
}

void IPlatformLog::Update(const IPlatformLogCtx *logCtx)
{
	std::chrono::hours logHours = std::chrono::duration_cast<std::chrono::hours>(logCtx->GetTime().time_since_epoch());

	if ((!_fp) || (_hoursCnt != logHours.count()))
	{
		_hoursCnt = logHours.count();

		if (_fp)
		{
			fclose(_fp);
			_fp = NULL;
		}

		const struct tm &logTm = logCtx->GetTm();
		char logPath[PATH_MAX_LEN] = { 0 };
		sprintf(logPath, "%s%c%04d-%02d-%02d%c", _logPath.c_str(), PATH_SPLIT_CHAR,
			logTm.tm_year + 1900, logTm.tm_mon + 1, logTm.tm_mday, PATH_SPLIT_CHAR);
		if (0 != PFMakeDirectory(logPath))
		{
			return;
		}

		sprintf(logPath + strlen(logPath), "%04d-%02d-%02d-%02d.log", logTm.tm_year + 1900, logTm.tm_mon + 1, logTm.tm_mday, logTm.tm_hour);
		_fp = fopen(logPath, "a+");
	}
}

static void ClearLog(SPlatformFileInfo *fileInfo, uint64_t &clearSize)
{
	while (fileInfo)
	{
		if (fileInfo->size < clearSize)
		{
			PFRemoveFile(fileInfo->fullName);
			clearSize -= fileInfo->size;
		}
		else if (PF_MODE_DIRECTORY == fileInfo->mode)
		{
			ClearLog(fileInfo->child, clearSize);
		}
		fileInfo = fileInfo->next;
	}
}

void IPlatformLog::DoClear()
{
	SPlatformFileInfo *fileInfo = GetFileInfo(_logPath.c_str(), PF_SORT_MODE_MODIFY_TIME);
	if (!fileInfo)
	{
		PBLogOut(PL_LEVEL_WARNING, "Cannot get file info, file=%s", _logPath.c_str());
		return;
	}

	if (fileInfo->size > _maxLogSize)
	{
		uint64_t clearSize = fileInfo->size - _maxLogSize / 10 * 8;
		ClearLog(fileInfo, clearSize);
	}

	FreeFileInfo(&fileInfo);
}

void IPlatformLog::WorkThreadEntry(void *hdl)
{
	IPlatformLog *h = (IPlatformLog *)hdl;
	return h->WorkThread();
}

void IPlatformLog::WorkThread()
{
	std::chrono::time_point<std::chrono::steady_clock> nowTime = std::chrono::steady_clock::now();
	std::chrono::time_point<std::chrono::steady_clock> nextSpanTime = nowTime + std::chrono::milliseconds(_spanMs);
	std::chrono::time_point<std::chrono::steady_clock> nextClearTime = nowTime + std::chrono::milliseconds(_clearMs);
	while (_isRunning)
	{
		nowTime = std::chrono::steady_clock::now();
		if (nextSpanTime > nowTime)
		{
			std::this_thread::sleep_for(nextSpanTime - nowTime);
		}
		else
		{
			nextSpanTime = nowTime;
			if (nowTime > nextClearTime)
			{
				nextClearTime = nowTime;
			}
		}

		_mutex.lock();
		IPlatformLogCtx *logCtx = _logCtxDummy.GetNext();
		_logCtxDummy.SetNext(NULL);

		_logCtxTail = &_logCtxDummy;
		_logCtxCnt = 0;
		_mutex.unlock();

		while (logCtx)
		{
			logCtx->Init();
			if (logCtx->NeedPrintScreen())
			{
				logCtx->PrintScreen();
			}

			Update(logCtx);
			logCtx->PrintFile(_fp);

			IPlatformLogCtx *oldLogCtx = logCtx;
			logCtx = logCtx->GetNext();
			delete oldLogCtx;
		}

		if (nowTime > nextClearTime)
		{
			if (_fp)
			{
				fclose(_fp);
				_fp = NULL;
			}

			DoClear();
			nextClearTime += std::chrono::milliseconds(_clearMs);
		}

		nextSpanTime += std::chrono::milliseconds(_spanMs);
	}
}