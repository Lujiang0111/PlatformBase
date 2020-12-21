#ifndef I_PLATFORM_FILE_H_
#define I_PLATFORM_FILE_H_

#include "PlatformFile.h"
#include "Base/IPlatformBaseApi.h"

#if defined(WIN32) || defined(_WINDLL)
constexpr auto PATH_SPLIT_CHAR = '\\';
constexpr auto PATH_SPLIT_CHAR_OTHER = '/';

constexpr auto PATH_MAX_LEN = 260;
#else
constexpr auto PATH_SPLIT_CHAR = '/';
constexpr auto PATH_SPLIT_CHAR_OTHER = '\\';

constexpr auto PATH_MAX_LEN = 2048;
#endif

#endif // !I_PLATFORM_FILE_H_