#include <string.h>
#include <algorithm>
#include "File/IPlatformFile.h"
#include "Log/IPlatformLog.h"

std::atomic<uint64_t> PlatformLog::_id(0);

PlatformLog::PlatformLog(const char *logPath, int64_t spanMs, int64_t clearMs, uint64_t maxLogSize, uint64_t maxQueLen)
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
		// Ĭ��50ms��ӡһ����־
		_spanMs = 50;
	}

	if (clearMs > 0)
	{
		_clearMs = clearMs;
	}
	else
	{
		// Ĭ��һ��������һ����־
		_clearMs = 60000;
	}

	if (maxLogSize > 0)
	{
		_maxLogSize = maxLogSize;
	}
	else
	{
		// Ĭ����־�����СΪ5G
		_maxLogSize = 5LL << 30;
	}

	if (maxQueLen > 0)
	{
		_maxQueLen = maxQueLen;
	}
	else
	{
		// Ĭ����־������󳤶�Ϊ10000
		_maxQueLen = 10000;
	}
	
	_isRunning = false;
	_workThread = NULL;
	_hoursCnt = 0;
	_fp = NULL;

	_logCtxTail = &_logCtxDummy;
	_logCtxCnt = 0;

	_contentBuf.resize(1);
}

PlatformLog::~PlatformLog()
{
	Stop();

	PlatformLogCtx *head = _logCtxDummy.GetNext();
	while (head)
	{
		PlatformLogCtx *oldLogCtx = head;
		head = head->GetNext();
		delete oldLogCtx;
	}
}

bool PlatformLog::Start()
{
	if (_isRunning)
	{
		return true;
	}

	_isRunning = true;
	_workThread = new std::thread(WorkThreadEntry, this);

	return true;
}

void PlatformLog::Stop()
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

void PlatformLog::SendLog(EPlatformLogLevel level, bool needPrintScreen, const char *fileName, int fileLine, const char *fmt, va_list vl)
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
	PlatformLogCtx *logCtx = new PlatformLogCtx(_id, level, needPrintScreen, fileName, fileLine, &_contentBuf[0]);
	_logCtxTail->SetNext(logCtx);
	_logCtxTail = logCtx;
	++_logCtxCnt;
}

void PlatformLog::SendLog(EPlatformLogLevel level, bool needPrintScreen, const char *fileName, int fileLine, const char *content)
{
	std::lock_guard<std::mutex> lock(_mutex);

	if (_logCtxCnt >= _maxQueLen)
	{
		_logCtxTail->AddLostCnt();
		return;
	}

	++_id;
	PlatformLogCtx *logCtx = new PlatformLogCtx(_id, level, needPrintScreen, fileName, fileLine, content);
	_logCtxTail->SetNext(logCtx);
	_logCtxTail = logCtx;
	++_logCtxCnt;
}

void PlatformLog::Update(const PlatformLogCtx *logCtx)
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

void PlatformLog::DoClear()
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

void PlatformLog::WorkThreadEntry(void *hdl)
{
	PlatformLog *h = (PlatformLog *)hdl;
	return h->WorkThread();
}

void PlatformLog::WorkThread()
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
				nextClearTime = nowTime + std::chrono::milliseconds(_spanMs);
			}
		}

		_mutex.lock();
		PlatformLogCtx *logCtx = _logCtxDummy.GetNext();
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

			PlatformLogCtx *oldLogCtx = logCtx;
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

/******************************  C API START  ******************************/
PlatformLogHandle PlatformLogCreate(const char *logPath, int64_t spanMs, int64_t clearMs, int64_t maxLogSize)
{
	PlatformLog *h = new PlatformLog(logPath, spanMs, clearMs, maxLogSize, 10000);
	h->Start();

	return h;
}

void PlatformLogDestroy(PlatformLogHandle *hdl)
{
	if ((!hdl) || (!(*hdl)))
	{
		return;
	}

	PlatformLog *h = (PlatformLog *)(*hdl);
	delete h;

	*hdl = NULL;
}

void PlatformLogSend(PlatformLogHandle hdl, EPlatformLogLevel level, int needPrintScreen, const char *fileName, int fileLine, const char *fmt, ...)
{
	PlatformLog *h = (PlatformLog *)hdl;
	if (!h)
	{
		return;
	}

	va_list vl;
	va_start(vl, fmt);
	h->SendLog(level, needPrintScreen, fileName, fileLine, fmt, vl);
	va_end(vl);
}

void PlatformLogSendVa(PlatformLogHandle hdl, EPlatformLogLevel level, int needPrintScreen, const char *fileName, int fileLine, const char *fmt, va_list vl)
{
	PlatformLog *h = (PlatformLog *)hdl;
	if (!h)
	{
		return;
	}

	h->SendLog(level, needPrintScreen, fileName, fileLine, fmt, vl);
}

void PlatformLogSendContent(PlatformLogHandle hdl, EPlatformLogLevel level, int needPrintScreen, const char *fileName, int fileLine, const char *content)
{
	PlatformLog *h = (PlatformLog *)hdl;
	if (!h)
	{
		return;
	}

	h->SendLog(level, needPrintScreen, fileName, fileLine, content);
}
/*******************************  C API END  *******************************/