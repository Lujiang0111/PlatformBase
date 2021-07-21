#if (defined(WIN32)) || (defined(_WINDLL))
#include <windows.h>
#else
#endif

#include <string.h>
#include <iomanip>
#include "Log/IPlatformLogCtx.h"

// 日志等级
constexpr auto L_DEBUG		= "DEBUG ";
constexpr auto L_INFO		= "INFO  ";
constexpr auto L_WARNING	= "WARN  ";
constexpr auto L_ERROR		= "ERROR ";
constexpr auto L_FAULT		= "FAULT ";

// 屏幕打印字体颜色
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

PlatformLogCtx::PlatformLogCtx(EPlatformLogLevel level, bool needPrintScreen, const char *fileName, int fileLine, const char *fmt, va_list vl)
{
	_id = 0;

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
	_content.assign(1024, 0);
	_next = NULL;

	_time = std::chrono::system_clock::now();
	time_t logTime = std::chrono::system_clock::to_time_t(_time);
#if defined(WIN32) || defined(_WINDLL)
	localtime_s(&_tm, &logTime);
#else
	localtime_r(&logTime, &_tm);
#endif

	if (!fmt)
	{
		return;
	}

	if (vl)
	{
		while (true)
		{
			va_list vp;
			va_copy(vp, vl);
			int ret = vsnprintf(&_content[0], _content.size(), fmt, vp);
			va_end(vp);
			if ((ret >= 0) && (ret < static_cast<int>(_content.size())))
			{
				break;
			}

			if (ret > 0)
			{
				_content.resize(ret + 1);
			}
			else
			{
				return;
			}
		}
	}
	else
	{
		_content.resize(strlen(fmt) + 1);
		memcpy(&_content[0], fmt, strlen(fmt) + 1);
	}
}

PlatformLogCtx::~PlatformLogCtx()
{

}

void PlatformLogCtx::PrintScreen() const
{
	if (!_needPrintScreen)
	{
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

	const char *cLevel = nullptr;
	switch (_level)
	{
	case PL_LEVEL_DEBUG:
		cLevel = L_DEBUG;
		break;
	case PL_LEVEL_INFO:
		cLevel = L_INFO;
		break;
	case PL_LEVEL_WARNING:
		cLevel = L_WARNING;
		break;
	case PL_LEVEL_ERROR:
		cLevel = L_ERROR;
		break;
	case PL_LEVEL_FATAL:
		cLevel = L_FAULT;
		break;
	default:
		cLevel = L_DEBUG;
		break;
	}

	if (_fileName.length() > 0)
	{
		if (_fileLine > 0)
		{
			printf("%s%s:%d, %s\n", cLevel, _fileName.c_str(), _fileLine, &_content[0]);
		}
		else
		{
			printf("%s%s, %s\n", cLevel, _fileName.c_str(), &_content[0]);
		}
	}
	else
	{
		printf("%s%s\n", cLevel, &_content[0]);
	}

	color = (FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
#else
	const char *cLevel = nullptr;
	const char *cColor = nullptr;
	switch (_level)
	{
	case PL_LEVEL_DEBUG:
		cLevel = L_DEBUG;
		cColor = C_GREEN;
		break;
	case PL_LEVEL_INFO:
		cLevel = L_INFO;
		cColor = C_BLUE;
		break;
	case PL_LEVEL_WARNING:
		cLevel = L_WARNING;
		cColor = C_YELLOW;
		break;
	case PL_LEVEL_ERROR:
		cLevel = L_ERROR;
		cColor = C_RED;
		break;
	case PL_LEVEL_FATAL:
		cLevel = L_FAULT;
		cColor = C_RED_HL;
		break;
	default:
		cLevel = L_DEBUG;
		cColor = C_GREEN;
		break;
	}

	if (_fileName.length() > 0)
	{
		if (_fileLine > 0)
		{
			printf("%s%s%s:%d, %s%s\n", cColor, cLevel, _fileName.c_str(), _fileLine, &_content[0], C_NONE);
		}
		else
		{
			printf("%s%s%s, %s%s\n", cColor, cLevel, _fileName.c_str(), &_content[0], C_NONE);
		}
	}
	else
	{
		printf("%s%s%s%s\n", cColor, cLevel, &_content[0], C_NONE);
	}
#endif
}

void PlatformLogCtx::PrintFile(FILE *fp) const
{
	if (!fp)
	{
		return;
	}

	std::chrono::milliseconds logMs = std::chrono::duration_cast<std::chrono::milliseconds>(_time.time_since_epoch());
	fprintf(fp, "[%zu] %04d-%02d-%02d %02d:%02d:%02d %03d ", _id,
		_tm.tm_year + 1900, _tm.tm_mon + 1, _tm.tm_mday, _tm.tm_hour, _tm.tm_min, _tm.tm_sec, static_cast<int>(logMs.count() % 1000));

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

	if (_fileName.length() > 0)
	{
		if (_fileLine > 0)
		{
			fprintf(fp, "%s:%d, ", _fileName.c_str(), _fileLine);
		}
		else
		{
			fprintf(fp, "%s, ", _fileName.c_str());
		}
	}

	fprintf(fp, "%s\n", &_content[0]);
	fflush(fp);
}

void PlatformLogCtx::SetId(size_t id)
{
	_id = id;
}

const std::chrono::time_point<std::chrono::system_clock> &PlatformLogCtx::GetTime() const
{
	return _time;
}

const struct tm &PlatformLogCtx::GetTm() const
{
	return _tm;
}

PlatformLogCtx *PlatformLogCtx::GetNext()
{
	return _next;
}

void PlatformLogCtx::SetNext(PlatformLogCtx *next)
{
	_next = next;
}