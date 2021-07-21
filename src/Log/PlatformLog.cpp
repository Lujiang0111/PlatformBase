#include <string.h>
#include <algorithm>
#include "File/IPlatformFile.h"
#include "Log/IPlatformLog.h"

size_t PlatformLog::_id(0);

PlatformLog::PlatformLog(const char *logPath, size_t clearMs, size_t maxSize, size_t maxCount)
	: _logDummy(PL_LEVEL_DEBUG, true, nullptr, 0, nullptr, nullptr)
{
	_logPath = (logPath) ? logPath : "Log";
	std::replace(_logPath.begin(), _logPath.end(), PATH_SPLIT_CHAR_OTHER, PATH_SPLIT_CHAR);
	if (PATH_SPLIT_CHAR != _logPath[_logPath.length() - 1])
	{
		_logPath += PATH_SPLIT_CHAR;
	}

	_clearMs = (clearMs > 0) ? clearMs : 60000;
	_maxSize = (maxSize > 0) ? maxSize : (5LL << 30);
	_maxCount = (maxCount > 0) ? maxCount : 10000;

	_bWorkThread = false;
	_logHours = 0;
	_fp = nullptr;

	_isLogFull = false;

	_logTail = &_logDummy;
	_logInCnt = 0;
	_isLogReady = false;
}

PlatformLog::~PlatformLog()
{
	Stop();
}

bool PlatformLog::Start()
{
	PFMakeDirectory(_logPath.c_str());

	_bWorkThread = true;
	_workThread = std::thread(&PlatformLog::WorkThread, this);

	return true;
}

void PlatformLog::Stop()
{
	_bWorkThread = false;
	_isLogReady = true;
	_condLog.notify_all();
	if (_workThread.joinable())
	{
		_workThread.join();
	}

	PlatformLogCtx *head = _logDummy.GetNext();
	while (head)
	{
		PlatformLogCtx *logCtx = head;
		head = head->GetNext();
		delete logCtx;
	}
	_logTail = nullptr;
	_logInCnt = 0;
	_isLogReady = false;
}

void PlatformLog::SendLog(EPlatformLogLevel level, bool needPrintScreen, const char *fileName, int fileLine, const char *fmt, va_list vl)
{
	if (!_bWorkThread || _isLogFull)
	{
		std::lock_guard<std::mutex> lock(_mutLog);
		++_id;
		++_logInCnt;
		return;
	}

	PlatformLogCtx *logCtx = new PlatformLogCtx(level, needPrintScreen, fileName, fileLine, fmt, vl);

	{
		std::lock_guard<std::mutex> lock(_mutLog);
		++_id;
		logCtx->SetId(_id);

		logCtx->PrintScreen();

		++_logInCnt;
		if (_logInCnt > _maxCount)
		{
			_isLogFull = true;
		}

		_logTail->SetNext(logCtx);
		_logTail = logCtx;
		_isLogReady = true;
		_condLog.notify_one();
	}
}

void PlatformLog::Update(const PlatformLogCtx *logCtx)
{
	std::chrono::hours logHours = std::chrono::duration_cast<std::chrono::hours>(logCtx->GetTime().time_since_epoch());

	if ((!_fp) || (_logHours != logHours.count()))
	{
		_logHours = logHours.count();

		if (_fp)
		{
			fclose(_fp);
			_fp = nullptr;
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
		_fp = fopen(logPath, "a");
	}
}

static void ClearLog(SPlatformFileInfo *fileInfo, size_t &clearSize)
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

	if (fileInfo->size > _maxSize)
	{
		size_t clearSize = fileInfo->size - _maxSize / 10 * 8;
		ClearLog(fileInfo, clearSize);
	}

	FreeFileInfo(&fileInfo);
}

void PlatformLog::WorkThread()
{
	std::chrono::time_point<std::chrono::steady_clock> nowTime = std::chrono::steady_clock::now();
	std::chrono::time_point<std::chrono::steady_clock> nextClearTime = nowTime;
	while (_bWorkThread)
	{
		nowTime = std::chrono::steady_clock::now();
		if (nowTime >= nextClearTime)
		{
			if (_fp)
			{
				fclose(_fp);
				_fp = nullptr;
			}

			DoClear();
			nextClearTime = nowTime + std::chrono::milliseconds(_clearMs);
		}

		size_t logCnt = 0;
		PlatformLogCtx *logCtx = nullptr;
		{
			std::unique_lock<std::mutex> lock(_mutLog);
			_condLog.wait(lock, [this] {return _isLogReady; });
			logCtx = _logDummy.GetNext();
			logCnt = _logInCnt;

			_logDummy.SetNext(nullptr);
			_logTail = &_logDummy;
			_logInCnt = 0;
			_isLogReady = false;
			if (_isLogFull)
			{
				_isLogFull = false;
			}
		}

		if (logCnt > _maxCount)
		{
			printf("\n\n\n---log cache full, lost %zu logs---\n\n\n", logCnt);
			if (_fp)
			{
				fprintf(_fp, "\n\n\n---log cache full, lost %zu logs---\n\n\n", logCnt);
			}

			while (logCtx)
			{
				PlatformLogCtx *oldLogCtx = logCtx;
				logCtx = logCtx->GetNext();
				delete oldLogCtx;
			}
		}
		else
		{
			while (logCtx)
			{
				Update(logCtx);
				logCtx->PrintFile(_fp);

				PlatformLogCtx *oldLogCtx = logCtx;
				logCtx = logCtx->GetNext();
				delete oldLogCtx;
			}
		}
	}

	if (_fp)
	{
		fclose(_fp);
		_fp = nullptr;
	}
}

/******************************  C API START  ******************************/
PlatformLogHandle PlatformLogCreate(const char *logPath, size_t clearMs, size_t maxSize, size_t maxCount)
{
	PlatformLog *h = new PlatformLog(logPath, clearMs, maxSize, maxCount);
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

	*hdl = nullptr;
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

	h->SendLog(level, needPrintScreen, fileName, fileLine, content, nullptr);
}
/*******************************  C API END  *******************************/