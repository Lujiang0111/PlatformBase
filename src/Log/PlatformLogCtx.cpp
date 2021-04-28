#if (defined(WIN32)) || (defined(_WINDLL))
#include <windows.h>
#else
#endif

#include <iostream>
#include <iomanip>
#include "Log/IPlatformLogCtx.h"

//设置屏幕打印字体颜色
constexpr auto C_NONE		= "\033[0m";
constexpr auto C_BLUE		= "\033[0;34m";
constexpr auto C_BLUE_HL	= "\033[1;34m";
constexpr auto C_GREEN		= "\033[0;32m";
constexpr auto C_GREEN_HL	= "\033[1;32m";
constexpr auto C_CYAN		= "\033[0;36m";
constexpr auto C_CYAN_HL	= "\033[1;36m";
constexpr auto C_RED		= "\033[0;31m";
constexpr auto C_RED_HL		= "\033[1;31m";
constexpr auto C_MAGENTA	= "\033[0;35m";
constexpr auto C_MAGENTA_HL	= "\033[1;35m";
constexpr auto C_YELLOW		= "\033[0;33m";
constexpr auto C_YELLOW_HL	= "\033[1;33m";

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
		_fileName = fileName;
	}

	_fileLine = fileLine;

	if (content)
	{
		_content = content;
	}

	_lostCnt = 0;
	_time = std::chrono::system_clock::now();

	_next = NULL;
}

PlatformLogCtx::~PlatformLogCtx()
{
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
		std::cout << "\n\n\n---log cache full, lost " << _lostCnt << " logs---\n\n" << std::endl;
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
		std::cout << "DEBUG ";
		break;
	case PL_LEVEL_INFO:
		std::cout << "INFO  ";
		break;
	case PL_LEVEL_WARNING:
		std::cout << "WARN  ";
		break;
	case PL_LEVEL_ERROR:
		std::cout << "ERROR ";
		break;
	case PL_LEVEL_FATAL:
		std::cout << "FAULT ";
		break;
	default:
		std::cout <<"DEBUG  ";
		break;
	}

	if (_fileLine > 0)
	{
		std::cout << _fileName << _fileLine << ", " << _content << std::endl;
	}
	else
	{
		std::cout << _fileName << ", " << _content << std::endl;
	}

	color = (FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
#else
	switch (_level)
	{
	case PL_LEVEL_DEBUG:
		std::cout << C_GREEN << "DEBUG ";
		break;
	case PL_LEVEL_INFO:
		std::cout << C_BLUE << "INFO  ";
		break;
	case PL_LEVEL_WARNING:
		std::cout << C_YELLOW << "WARN  ";
		break;
	case PL_LEVEL_ERROR:
		std::cout << C_RED << "ERROR ";
		break;
	case PL_LEVEL_FATAL:
		std::cout << C_RED_HL << "FAULT ";
		break;
	default:
		std::cout << C_GREEN << "DEBUG ";
		break;
	}

	if (_fileLine > 0)
	{
		std::cout << _fileName << _fileLine << ", " << _content << C_NONE << std::endl;
	}
	else
	{
		std::cout << _fileName << ", " << _content << C_NONE << std::endl;
	}
#endif
}

void PlatformLogCtx::PrintFile(std::fstream &fout) const
{
	if (!fout.is_open())
	{
		return;
	}

	if (_lostCnt > 0)
	{
		fout << "\n\n\n---log cache full, lost " << _lostCnt << " logs---\n\n" << std::endl;
		return;
	}

	std::chrono::milliseconds logMs = std::chrono::duration_cast<std::chrono::milliseconds>(_time.time_since_epoch());
	fout << _id <<
		std::setw(4) << std::setfill('0') << _tm.tm_year + 1900 << "-" <<
		std::setw(2) << std::setfill('0') << _tm.tm_mon + 1 << "-" <<
		std::setw(2) << std::setfill('0') << _tm.tm_mday << " " <<
		std::setw(2) << std::setfill('0') << _tm.tm_hour << ":" <<
		std::setw(2) << std::setfill('0') << _tm.tm_min << ":" <<
		std::setw(2) << std::setfill('0') << _tm.tm_sec << " " <<
		logMs.count() % 1000 << " ";

	switch (_level)
	{
	case PL_LEVEL_DEBUG:
		fout << "DEBUG ";
		break;
	case PL_LEVEL_INFO:
		fout << "INFO  ";
		break;
	case PL_LEVEL_WARNING:
		fout << "WARN  ";
		break;
	case PL_LEVEL_ERROR:
		fout << "ERROR ";
		break;
	case PL_LEVEL_FATAL:
		fout << "FAULT ";
		break;
	default:
		fout << "DEBUG ";
		break;
	}

	if (_fileLine > 0)
	{
		fout << _fileName << _fileLine << ", " << _content << std::endl;
	}
	else
	{
		fout << _fileName << ", " << _content << std::endl;
	}
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