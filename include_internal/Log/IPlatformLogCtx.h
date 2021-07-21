#ifndef I_LOG_CTX_H_
#define I_LOG_CTX_H_

#include <stdio.h>
#include <time.h>
#include <chrono>
#include <vector>
#include "Base/IPlatformBaseApi.h"

class PlatformLogCtx
{
public:
	PlatformLogCtx(EPlatformLogLevel level, bool needPrintScreen, const char *fileName, int fileLine, const char *fmt, va_list vl);
	virtual ~PlatformLogCtx();

	void PrintScreen() const;
	void PrintFile(FILE *fp) const;

	void SetId(size_t id);

	const std::chrono::time_point<std::chrono::system_clock> &GetTime() const;
	const struct tm &GetTm() const;

	PlatformLogCtx *GetNext();
	void SetNext(PlatformLogCtx *next);

private:
	PlatformLogCtx() = delete;
	PlatformLogCtx(const PlatformLogCtx&) = delete;
	PlatformLogCtx& operator=(const PlatformLogCtx&) = delete;

private:
	size_t _id;
	EPlatformLogLevel _level;
	bool _needPrintScreen;
	std::string _fileName;
	int _fileLine;
	std::vector<char> _content;

	std::chrono::time_point<std::chrono::system_clock> _time;
	struct tm _tm;

	// 单链结构
	PlatformLogCtx *_next;
};

#endif // ! I_LOG_CTX_H_