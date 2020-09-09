#ifndef PLATFORM_FILE_H_
#define PLATFORM_FILE_H_

#if defined(WIN32) || defined(_WINDLL)
#if defined(PLATFORM_BASE_EXPORT)
#define PLATFORM_BASE_API   __declspec(dllexport)
#else
#define PLATFORM_BASE_API   __declspec(dllimport)
#endif
#else
#define PLATFORM_BASE_API
#endif

#ifdef __cplusplus
extern "C"
{
#endif

	typedef enum PlatformFileMode_
	{
		PF_MODE_UNKNOW = 0,
		PF_MODE_NORMAL_FILE,				// 普通文件
		PF_MODE_DIRECTORY,					// 目录
		PF_MODE_LINK,						// 链接,如快捷方式,linux下的符号链接等
	}EPlatformFileMode;

	typedef enum PlatformFileSortMode_
	{
		PF_SORT_MODE_NONE = 0,				// 不排序，效率最高
		PF_SORT_MODE_NAME,					// 按文件名排序
		PF_SORT_MODE_MODIFY_TIME,			// 按修改时间排序
	}EPlatformFileSortMode;

	typedef struct PlatformFileInfo_
	{
		char *name;							// 文件名
		char *fullName;						// 包含路径的完整文件名
		EPlatformFileMode mode;				// 文件类型
		uint64_t size;						// 如果是普通文件，则代表文件大小；如果是目录，则代表目录的总大小
		int64_t lastModifyTime;				// 最后修改时间戳

		struct PlatformFileInfo_ *parent;	// 父目录
		struct PlatformFileInfo_ *child;	// 子目录的第一个文件
		struct PlatformFileInfo_ *next;		// 同目录的下一个文件
	}SPlatformFileInfo;

	PLATFORM_BASE_API SPlatformFileInfo *GetFileInfo(const char* fileName, EPlatformFileSortMode sortMode);

	PLATFORM_BASE_API void FreeFileInfo(SPlatformFileInfo **hdl);

	PLATFORM_BASE_API int PFMakeDirectory(const char *pathName);

	PLATFORM_BASE_API int PFRemoveFile(const char *fileName);

#ifdef __cplusplus
}
#endif

#endif // !PLATFORM_FILE_H_