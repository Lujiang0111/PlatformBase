#ifndef I_LOG_CTX_H_
#define I_LOG_CTX_H_

#include <time.h>
#include <fstream>
#include <chrono>
#include <string>
#include "Base/IPlatformBaseApi.h"

class PlatformLogCtx
{
public:
	PlatformLogCtx(uint64_t id, EPlatformLogLevel level, bool needPrintScreen, const char *fileName, int fileLine, const char *content);
	virtual ~PlatformLogCtx();

	void Init();
	void PrintScreen() const;
	void PrintFile(std::fstream &fout) const;

	const std::chrono::time_point<std::chrono::system_clock> &GetTime() const;
	const struct tm &GetTm() const;
	bool NeedPrintScreen() const;
	void AddLostCnt();

	PlatformLogCtx *GetNext();
	void SetNext(PlatformLogCtx *next);

private:
	PlatformLogCtx() = delete;
	PlatformLogCtx(const PlatformLogCtx&) = delete;
	PlatformLogCtx& operator=(const PlatformLogCtx&) = delete;

private:
	uint64_t _id;
	EPlatformLogLevel _level;
	bool _needPrintScreen;
	std::string _fileName;
	int _fileLine;
	std::string _content;

	int _lostCnt;
	std::chrono::time_point<std::chrono::system_clock> _time;
	struct tm _tm;

	// 单链结构
	PlatformLogCtx *_next;
};

#endif // ! I_LOG_CTX_H_