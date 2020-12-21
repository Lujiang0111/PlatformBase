#if (defined(WIN32)) || (defined(_WINDLL))
#include <windows.h>
#else
#endif

#include <stdio.h>
#include <string.h>
#include "Log/IPlatformLogCtx.h"

//设置屏幕打印字体颜色
#define C_NONE          "\033[0m"
#define C_BLUE          "\033[0;34m"
#define C_BLUE_HL       "\033[1;34m"
#define C_GREEN         "\033[0;32m"
#define C_GREEN_HL      "\033[1;32m"
#define C_CYAN          "\033[0;36m"
#define C_CYAN_HL       "\033[1;36m"
#define C_RED           "\033[0;31m"
#define C_RED_HL        "\033[1;31m"
#define C_MAGENTA       "\033[0;35m"
#define C_MAGENTA_HL    "\033[1;35m"
#define C_YELLOW        "\033[0;33m"
#define C_YELLOW_HL     "\033[1;33m"

PlatformLogCtx::PlatformLogCtx(uint64_t id, EPlatformLogLevel level, bool needPrintScreen, const char *fileName, int fileLine, const char *content)
{
	_id = id;

	_level = level;
	if (_level < PL_LEVEL_DEBUG)
	{
		_level = PL_LEVEL_DEBUG;
	}
	else if (_level > PL_LEVEL_FATAL)
	{
		_level = PL_LEVEL_FATAL;
	}

	_needPrintScreen = needPrintScreen;

	if (fileName)
	{
		_fileName = new char[strlen(fileName) + 1];
		strcpy(_fileName, fileName);
	}
	else
	{
		_fileName = new char[1]();
	}

	_fileLine = fileLine;

	if (content)
	{
		_content = new char[strlen(content) + 1];
		strcpy(_content, content);
	}
	else
	{
		_content = new char[1]();
	}

	_lostCnt = 0;
	_time = std::chrono::system_clock::now();

	_next = NULL;
}

PlatformLogCtx::~PlatformLogCtx()
{
	if (_fileName)
	{
		delete[]_fileName;
		_fileName = NULL;
	}

	if (_content)
	{
		delete[]_content;
		_content = NULL;
	}
}

void PlatformLogCtx::Init()
{
	time_t logTime = std::chrono::system_clock::to_time_t(_time);
#if defined(WIN32) || defined(_WINDLL)
	localtime_s(&_tm, &logTime);
#else
	localtime_r(&logTime, &_tm);
#endif
}

void PlatformLogCtx::PrintScreen() const
{
	if (_lostCnt > 0)
	{
		printf("\n\n\nERROR log cache full, lost %d logs\n\n\n", _lostCnt);
		return;
	}

#if defined(WIN32) || defined(_WINDLL)
	WORD color = 0;
	color |= FOREGROUND_INTENSITY;
	switch (_level)
	{
	case PL_LEVEL_DEBUG:
		color |= FOREGROUND_GREEN;
		break;
	case PL_LEVEL_INFO:
		color |= FOREGROUND_BLUE;
		break;
	case PL_LEVEL_WARNING:
		color |= (FOREGROUND_RED | FOREGROUND_GREEN);  //yellow
		break;
	case PL_LEVEL_ERROR:
		color |= FOREGROUND_RED;
		break;
	case PL_LEVEL_FATAL:
		color |= (FOREGROUND_RED | FOREGROUND_BLUE);
		break;
	default:
		color |= FOREGROUND_GREEN;
		break;
	}
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);

	switch (_level)
	{
	case PL_LEVEL_DEBUG:
		printf("DEBUG ");
		break;
	case PL_LEVEL_INFO:
		printf("INFO  ");
		break;
	case PL_LEVEL_WARNING:
		printf("WARN  ");
		break;
	case PL_LEVEL_ERROR:
		printf("ERROR ");
		break;
	case PL_LEVEL_FATAL:
		printf("FAULT ");
		break;
	default:
		printf("DEBUG  ");
		break;
	}

	if (_fileLine > 0)
	{
		printf("%s%d:%s%s", _fileName, _fileLine, _content, "\n");
	}
	else
	{
		printf("%s:%s%s", _fileName, _content, "\n");
	}

	color = (FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
#else
	switch (_level)
	{
	case PL_LEVEL_DEBUG:
		printf(C_GREEN"DEBUG ");
		break;
	case PL_LEVEL_INFO:
		printf(C_BLUE"INFO  ");
		break;
	case PL_LEVEL_WARNING:
		printf(C_YELLOW"WARN  ");
		break;
	case PL_LEVEL_ERROR:
		printf(C_RED"ERROR ");
		break;
	case PL_LEVEL_FATAL:
		printf(C_RED_HL"FAULT ");
		break;
	default:
		printf(C_GREEN"DEBUG ");
		break;
	}

	if (_fileLine > 0)
	{
		printf("%s%d:%s%s" C_NONE, _fileName, _fileLine, _content, "\n");
	}
	else
	{
		printf("%s:%s%s" C_NONE, _fileName, _content, "\n");
	}
#endif
}

void PlatformLogCtx::PrintFile(FILE *fp) const
{
	if (!fp)
	{
		return;
	}

	if (_lostCnt > 0)
	{
		fprintf(fp, "\n\n\nERROR log cache full, lost %d logs\n\n\n", _lostCnt);
		return;
	}

	std::chrono::milliseconds logMs = std::chrono::duration_cast<std::chrono::milliseconds>(_time.time_since_epoch());
	fprintf(fp, "[%llu] %04d-%02d-%02d %02d:%02d:%02d %03lld ", _id,
		_tm.tm_year + 1900, _tm.tm_mon + 1, _tm.tm_mday, _tm.tm_hour, _tm.tm_min, _tm.tm_sec, logMs.count() % 1000);

	switch (_level)
	{
	case PL_LEVEL_DEBUG:
		fprintf(fp, "DEBUG ");
		break;
	case PL_LEVEL_INFO:
		fprintf(fp, "INFO  ");
		break;
	case PL_LEVEL_WARNING:
		fprintf(fp, "WARN  ");
		break;
	case PL_LEVEL_ERROR:
		fprintf(fp, "ERROR ");
		break;
	case PL_LEVEL_FATAL:
		fprintf(fp, "FAULT ");
		break;
	default:
		fprintf(fp, "DEBUG ");
		break;
	}

	if (_fileLine > 0)
	{
		fprintf(fp, "%s%d:%s%s", _fileName, _fileLine, _content, "\n");
	}
	else
	{
		fprintf(fp, "%s:%s%s", _fileName, _content, "\n");
	}
	fflush(fp);
}

const std::chrono::time_point<std::chrono::system_clock> &PlatformLogCtx::GetTime() const
{
	return _time;
}

const struct tm &PlatformLogCtx::GetTm() const
{
	return _tm;
}

bool PlatformLogCtx::NeedPrintScreen() const
{
	return _needPrintScreen;
}

void PlatformLogCtx::AddLostCnt()
{
	++_lostCnt;
}

PlatformLogCtx *PlatformLogCtx::GetNext()
{
	return _next;
}

void PlatformLogCtx::SetNext(PlatformLogCtx *next)
{
	_next = next;
}