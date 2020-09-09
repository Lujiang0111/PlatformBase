#ifndef I_LOG_CTX_H_
#define I_LOG_CTX_H_

#include <chrono>
#include <time.h>
#include "PlatformLog.h"

class IPlatformLogCtx
{
public:
	IPlatformLogCtx(uint64_t id, EPlatformLogLevel level, bool needPrintScreen, const char *fileName, int fileLine, const char *content);
	virtual ~IPlatformLogCtx();

	void Init();
	void PrintScreen() const;
	void PrintFile(FILE *fp) const;

	const std::chrono::time_point<std::chrono::system_clock> &GetTime() const;
	const struct tm &GetTm() const;
	bool NeedPrintScreen() const;
	void AddLostCnt();

	IPlatformLogCtx *GetNext();
	void SetNext(IPlatformLogCtx *next);

private:
	IPlatformLogCtx() = delete;
	IPlatformLogCtx(const IPlatformLogCtx&) = delete;
	IPlatformLogCtx& operator=(const IPlatformLogCtx&) = delete;

private:
	uint64_t _id;
	EPlatformLogLevel _level;
	bool _needPrintScreen;
	char *_fileName;
	int _fileLine;
	char *_content;

	int _lostCnt;
	std::chrono::time_point<std::chrono::system_clock> _time;
	struct tm _tm;

	// µ¥Á´½á¹¹
	IPlatformLogCtx *_next;
};

#endif // ! I_LOG_CTX_H_