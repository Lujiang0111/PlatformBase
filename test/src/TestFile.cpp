#include <string>
#include <iostream>
#include "PlatformFile.h"
#include "TestFile.h"

static void PresentFileInfo(SPlatformFileInfo *fileInfo)
{
	while (fileInfo)
	{
		printf("name:%s size:%llu modify time:%lld\n", fileInfo->fullName, fileInfo->size, fileInfo->lastModifyTime);
		if (fileInfo->child)
		{
			PresentFileInfo(fileInfo->child);
		}

		fileInfo = fileInfo->next;
	}
}

void TestFileSort(int argc, char *argv[])
{
	std::string filePath;

	if (argc >= 3)
	{
		filePath = argv[2];
	}
	else
	{
		std::cout << "Input FilePath:";
		std::cin >> filePath;
	}

	SPlatformFileInfo *fileInfo = GetFileInfo(filePath.c_str(), PF_SORT_MODE_MODIFY_TIME);
	PresentFileInfo(fileInfo);
	FreeFileInfo(&fileInfo);
}