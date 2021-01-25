#if defined(WIN32) || defined(_WINDLL)
#include <windows.h>
#include <direct.h>
#else
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif

#include <string.h>
#include <algorithm>
#include <string>
#include "Log/IPlatformLog.h"
#include "File/IPlatformFile.h"

/******************************	函数声明	********************************/
static SPlatformFileInfo *MergeTwoList(SPlatformFileInfo *lhs, SPlatformFileInfo *rhs, EPlatformFileSortMode sortMode);
/***********************************************************************/

// 归并排序
static SPlatformFileInfo *SortList(SPlatformFileInfo *head, EPlatformFileSortMode sortMode)
{
	if (PF_SORT_MODE_NONE == sortMode)
	{
		return head;
	}

	if ((!head) || (!head->next))
	{
		return head;
	}

	// 快慢指针法找到中间节点
	SPlatformFileInfo *fast = head->next;
	SPlatformFileInfo *slow = head;
	while ((fast) && (fast->next))
	{
		slow = slow->next;
		fast = fast->next->next;
	}

	SPlatformFileInfo *rhs = SortList(slow->next, sortMode);
	slow->next = NULL;
	SPlatformFileInfo *lhs = SortList(head, sortMode);
	return MergeTwoList(lhs, rhs, sortMode);
}

// 归并排序合并
static SPlatformFileInfo *MergeTwoList(SPlatformFileInfo *lhs, SPlatformFileInfo *rhs, EPlatformFileSortMode sortMode)
{
	SPlatformFileInfo dummy;
	SPlatformFileInfo *tail = &dummy;

	while ((lhs) && (rhs))
	{
		switch (sortMode)
		{
		case PF_SORT_MODE_NAME:
			if (strcmp(lhs->name, rhs->name) < 0)
			{
				tail->next = lhs;
				lhs = lhs->next;
			}
			else
			{
				tail->next = rhs;
				rhs = rhs->next;
			}
			break;

		case PF_SORT_MODE_MODIFY_TIME:
			if (lhs->lastModifyTime < rhs->lastModifyTime)
			{
				tail->next = lhs;
				lhs = lhs->next;
			}
			else
			{
				tail->next = rhs;
				rhs = rhs->next;
			}
			break;

		default:
			PBLogOut(PL_LEVEL_WARNING, "unknown sort mode %d", sortMode);
			tail->next = lhs;
			lhs = lhs->next;
			break;
		}
		tail = tail->next;
	}

	tail->next = (NULL != lhs) ? lhs : rhs;
	return dummy.next;
}

#if defined(WIN32) || defined(_WINDLL)
static SPlatformFileInfo *SearchFileInfo(const char *fileName, EPlatformFileSortMode sortMode, SPlatformFileInfo *parent, uint64_t &totalSize)
{
	std::string sSearchFileName = fileName;

	if (parent)
	{
		// Prepare string for use with FindFile functions.  First, copy the
		// string to a buffer, then append '\*' to the directory name.
		sSearchFileName += "\\*";
	}

	if (sSearchFileName.length() > MAX_PATH)
	{
		PBLogOut(PL_LEVEL_ERROR, "filename too long! filename=%s", fileName);
		return NULL;
	}

	// Find the first file in the directory.
	WIN32_FIND_DATA ffd;
	HANDLE hFind = FindFirstFile(sSearchFileName.c_str(), &ffd);
	if (INVALID_HANDLE_VALUE == hFind)
	{
		return NULL;
	}

	// List all the files in the directory with some info about them.
	SPlatformFileInfo dummy = { 0 };
	SPlatformFileInfo *tail = &dummy;
	do
	{
		// Find first file will always return "." and ".." as the first two directories.
		if ((ffd.cFileName) && (0 != strcmp(ffd.cFileName, ".")) && (0 != strcmp(ffd.cFileName, "..")))
		{
			SPlatformFileInfo *node = new SPlatformFileInfo;
			node->name = new char[strlen(ffd.cFileName) + 1];
			strcpy(node->name, ffd.cFileName);

			if (parent)
			{
				node->fullName = new char[strlen(fileName) + 1 + strlen(ffd.cFileName) + 1];
				sprintf(node->fullName, "%s%c%s", fileName, PATH_SPLIT_CHAR, ffd.cFileName);
			}
			else
			{
				node->fullName = new char[strlen(fileName) + 1];
				strcpy(node->fullName, fileName);
			}

			node->size = ffd.nFileSizeHigh;
			node->size = (node->size << 32) + ffd.nFileSizeLow;

			uint64_t totalUs = (((uint64_t)ffd.ftLastWriteTime.dwHighDateTime << 32) |
				((uint64_t)ffd.ftLastWriteTime.dwLowDateTime)) / 10 - 11644473600000000ull;
			node->lastModifyTime = (int64_t)(totalUs / 1000000);

			node->parent = parent;
			if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				node->mode = PF_MODE_DIRECTORY;
				uint64_t dirSize = 0;
				node->child = SearchFileInfo(node->fullName, sortMode, node, dirSize);
				node->size += dirSize;
			}
			else
			{
				node->mode = PF_MODE_NORMAL_FILE;
				node->child = NULL;
			}
			node->next = NULL;

			totalSize += node->size;
			tail->next = node;
			tail = tail->next;
		}
	} while (FindNextFile(hFind, &ffd)); //Find the next file.

	FindClose(hFind);

	// 对链表进行排序
	return SortList(dummy.next, sortMode);
}
#else
static SPlatformFileInfo *SearchFileInfo(const char *fileName, EPlatformFileSortMode sortMode, SPlatformFileInfo *parent, uint64_t &totalSize)
{
	struct stat fileStat;
	if (0 != lstat(fileName, &fileStat))
	{
		PBLogOut(PL_LEVEL_ERROR, "stat file [%s] fail!, errno=%d", fileName, errno);
		return NULL;
	}

	SPlatformFileInfo dummy = { 0 };
	SPlatformFileInfo *tail = &dummy;
	if (!parent)
	{
		SPlatformFileInfo *node = new SPlatformFileInfo;
		const char *pSplit = strrchr(fileName, PATH_SPLIT_CHAR);

		if (pSplit)
		{
			node->name = new char[strlen(pSplit + 1) + 1];
			strcpy(node->name, pSplit + 1);
		}
		else
		{
			node->name = new char[strlen(fileName) + 1];
			strcpy(node->name, fileName);
		}

		node->fullName = new char[strlen(fileName) + 1];
		strcpy(node->fullName, fileName);

		node->mode = PF_MODE_NORMAL_FILE;
		node->size = fileStat.st_size;
		node->lastModifyTime = fileStat.st_ctime;

		node->parent = parent;
		if (S_ISDIR(fileStat.st_mode))
		{
			node->mode = PF_MODE_DIRECTORY;
			uint64_t dirSize = 0;
			node->child = SearchFileInfo(node->fullName, sortMode, node, dirSize);
			node->size += dirSize;
		}
		else
		{
			node->mode = PF_MODE_NORMAL_FILE;
			node->child = NULL;
		}
		node->next = NULL;

		totalSize += node->size;
		tail->next = node;
		tail = tail->next;
	}
	else
	{
		DIR *dir = opendir(fileName);
		if (!dir)
		{
			PBLogOut(PL_LEVEL_ERROR, "open dir [%s] fail!, errno=%d", fileName, errno);
			return NULL;
		}

		struct dirent* pDirent = NULL;
		while (NULL != (pDirent = readdir(dir)))
		{
			if ((0 == strcmp(pDirent->d_name, ".")) || (0 == strcmp(pDirent->d_name, "..")))
			{
				continue;
			}

			char *fullName = new char[strlen(fileName) + 1 + strlen(pDirent->d_name) + 1];
			sprintf(fullName, "%s%c%s", fileName, PATH_SPLIT_CHAR, pDirent->d_name);

			if (0 != lstat(fullName, &fileStat))
			{
				PBLogOut(PL_LEVEL_ERROR, "stat file [%s] fail!, errno=%d", fullName, errno);
				delete[]fullName;
				continue;
			}

			SPlatformFileInfo *node = new SPlatformFileInfo;
			node->name = new char[strlen(pDirent->d_name) + 1];
			strcpy(node->name, pDirent->d_name);
			node->fullName = fullName;

			node->size = fileStat.st_size;
			node->lastModifyTime = fileStat.st_ctime;

			node->parent = parent;
			if (S_ISDIR(fileStat.st_mode))
			{
				node->mode = PF_MODE_DIRECTORY;
				uint64_t dirSize = 0;
				node->child = SearchFileInfo(node->fullName, sortMode, node, dirSize);
				node->size += dirSize;
			}
			else
			{
				node->mode = PF_MODE_NORMAL_FILE;
				node->child = NULL;
			}
			node->next = NULL;

			totalSize += node->size;
			tail->next = node;
			tail = tail->next;
		}

		closedir(dir);
	}

	// 对链表进行排序
	return SortList(dummy.next, sortMode);
}
#endif

SPlatformFileInfo *GetFileInfo(const char* fileName, EPlatformFileSortMode sortMode)
{
	if (!fileName)
	{
		PBLogOut(PL_LEVEL_WARNING, "filename not exist!");
		return NULL;
	}

	std::string sRootFileName = fileName;
	std::replace(sRootFileName.begin(), sRootFileName.end(), PATH_SPLIT_CHAR_OTHER, PATH_SPLIT_CHAR);

	while (PATH_SPLIT_CHAR == sRootFileName.back())
	{
		sRootFileName.pop_back();
	}

	uint64_t totalSize = 0;
	return SearchFileInfo(sRootFileName.c_str(), sortMode, NULL, totalSize);
}

void FreeFileInfo(SPlatformFileInfo **hdl)
{
	if ((!hdl) || (!(*hdl)))
	{
		return;
	}

	SPlatformFileInfo *h = *hdl;
	if (h->name)
	{
		delete[]h->name;
		h->name = NULL;
	}

	if (h->fullName)
	{
		delete[]h->fullName;
		h->fullName = NULL;
	}

	if (h->parent)
	{
		h->parent = NULL;
	}

	if (h->child)
	{
		FreeFileInfo(&h->child);
		h->child = NULL;
	}

	if (h->next)
	{
		FreeFileInfo(&h->next);
		h->next = NULL;
	}

	delete h;
	*hdl = NULL;
}

int PFMakeDirectory(const char *pathName)
{
	if (!pathName)
	{
		PBLogOut(PL_LEVEL_WARNING, "pathName not exist!");
		return 0;
	}

	if (strlen(pathName) > PATH_MAX_LEN)
	{
		PBLogOut(PL_LEVEL_ERROR, "pathName too long! pathName=%s", pathName);
		return -1;
	}

	char sTmp[PATH_MAX_LEN + 2];
	strcpy(sTmp, pathName);
	size_t len = strlen(sTmp);

	// 不同平台路径转换
	for (size_t i = 0; i < len; ++i)
	{
		if (PATH_SPLIT_CHAR_OTHER == sTmp[i])
		{
			sTmp[i] = PATH_SPLIT_CHAR;
		}
	}

	if (PATH_SPLIT_CHAR != sTmp[len - 1])
	{
		sTmp[len] = PATH_SPLIT_CHAR;
		sTmp[len + 1] = 0;
		++len;
	}

	for (size_t i = 1; i < len; ++i)
	{
		if (PATH_SPLIT_CHAR == sTmp[i])
		{
			sTmp[i] = 0;
#if defined(WIN32) || defined(_WINDLL)
			if ((mkdir(sTmp) != 0) && (EEXIST != errno))
#else
			if ((mkdir(sTmp, 0777) != 0) && (EEXIST != errno))
#endif
			{
				PBLogOut(PL_LEVEL_ERROR, "make directory [%s] fail!, errno=%d", pathName, errno);
				return -1;
			}
			sTmp[i] = PATH_SPLIT_CHAR;
		}
	}

	return 0;
}

#if defined(WIN32) || defined(_WINDLL)
static bool RemoveFileRecursive(const char *fileName, bool isRoot)
{
	bool ret = true;
	std::string sSearchFileName = fileName;

	if (!isRoot)
	{
		// Prepare string for use with FindFile functions.  First, copy the
		// string to a buffer, then append '\*' to the directory name.
		sSearchFileName += "\\*";
	}

	if (sSearchFileName.length() > MAX_PATH)
	{
		PBLogOut(PL_LEVEL_ERROR, "filename too long! filename=%s", fileName);
		return NULL;
	}

	// Find the first file in the directory.
	WIN32_FIND_DATA ffd;
	HANDLE hFind = FindFirstFile(sSearchFileName.c_str(), &ffd);
	if (INVALID_HANDLE_VALUE == hFind)
	{
		return NULL;
	}

	// List all the files in the directory with some info about them.
	do
	{
		// Find first file will always return "." and ".." as the first two directories.
		if ((ffd.cFileName) && (0 != strcmp(ffd.cFileName, ".")) && (0 != strcmp(ffd.cFileName, "..")))
		{
			char *fullName = NULL;
			if (!isRoot)
			{
				fullName = new char[strlen(fileName) + 1 + strlen(ffd.cFileName) + 1];
				sprintf(fullName, "%s\\%s", fileName, ffd.cFileName);
			}
			else
			{
				fullName = new char[strlen(fileName) + 1];
				strcpy(fullName, fileName);
			}

			if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				if (!RemoveFileRecursive(fullName, false))
				{
					ret = false;
				}

				if (0 != rmdir(fullName))
				{
					PBLogOut(PL_LEVEL_ERROR, "remove directory [%s] fail!, errno=%d", fullName, errno);
					ret = false;
				}
			}
			else
			{
				if (0 != remove(fullName))
				{
					PBLogOut(PL_LEVEL_ERROR, "remove file [%s] fail!, errno=%d", fullName, errno);
					ret = false;
				}
			}

			delete[]fullName;
		}
	} while (FindNextFile(hFind, &ffd)); // Find the next file.

	FindClose(hFind);
	return ret;
}
#else
static bool RemoveFileRecursive(const char *fileName, bool isRoot)
{
	bool ret = true;

	struct stat fileStat;
	if (0 != lstat(fileName, &fileStat))
	{
		PBLogOut(PL_LEVEL_ERROR, "stat file [%s] fail!, errno=%d", fileName, errno);
		return false;
	}

	if (isRoot)
	{
		if (S_ISDIR(fileStat.st_mode))
		{
			if (!RemoveFileRecursive(fileName, false))
			{
				ret = false;
			}

			if (0 != rmdir(fileName))
			{
				PBLogOut(PL_LEVEL_ERROR, "remove directory [%s] fail!, errno=%d", fileName, errno);
				ret = false;
			}
		}
		else
		{
			if (0 != remove(fileName))
			{
				PBLogOut(PL_LEVEL_ERROR, "remove file [%s] fail!, errno=%d", fileName, errno);
				ret = false;
			}
		}
	}
	else
	{
		DIR *dir = opendir(fileName);
		if (!dir)
		{
			PBLogOut(PL_LEVEL_ERROR, "open dir [%s] fail!, errno=%d", fileName, errno);
			return false;
		}

		struct dirent* pDirent = NULL;
		while (NULL != (pDirent = readdir(dir)))
		{
			if ((0 == strcmp(pDirent->d_name, ".")) || (0 == strcmp(pDirent->d_name, "..")))
			{
				continue;
			}

			char *fullName = new char[strlen(fileName) + 1 + strlen(pDirent->d_name) + 1];
			sprintf(fullName, "%s%c%s", fileName, PATH_SPLIT_CHAR, pDirent->d_name);

			if (0 != lstat(fullName, &fileStat))
			{
				PBLogOut(PL_LEVEL_ERROR, "stat file [%s] fail!, errno=%d", fullName, errno);
				ret = false;
				delete[]fullName;
				continue;
			}

			if (S_ISDIR(fileStat.st_mode))
			{
				if (!RemoveFileRecursive(fullName, false))
				{
					ret = false;
				}

				if (0 != rmdir(fullName))
				{
					PBLogOut(PL_LEVEL_ERROR, "remove directory [%s] fail!, errno=%d", fullName, errno);
					ret = false;
				}
			}
			else
			{
				if (0 != remove(fullName))
				{
					PBLogOut(PL_LEVEL_ERROR, "remove file [%s] fail!, errno=%d", fullName, errno);
					ret = false;
				}
			}

			delete[]fullName;
		}

		closedir(dir);
	}

	return ret;
}
#endif

int PFRemoveFile(const char *fileName)
{
	if (!fileName)
	{
		return 0;
	}

	std::string sRootFileName = fileName;
	std::replace(sRootFileName.begin(), sRootFileName.end(), PATH_SPLIT_CHAR_OTHER, PATH_SPLIT_CHAR);

	while (PATH_SPLIT_CHAR == sRootFileName.back())
	{
		sRootFileName.pop_back();
	}

	if (!RemoveFileRecursive(fileName, true))
	{
		return -1;
	}
	return 0;
}