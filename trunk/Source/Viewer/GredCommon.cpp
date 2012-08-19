#include "Precompiled.h"
#include "GredCommon.h"

namespace gred
{

void enumFile(const std::wstring& searchPath, EnumCallback callback, void* param)
{
	WIN32_FIND_DATA fd;
	HANDLE findFile = ::FindFirstFile((searchPath + L"*").c_str(), &fd);

	if (findFile == INVALID_HANDLE_VALUE)
	{
		::FindClose(findFile);
		return;
	}

	while (true)
	{
		if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
		{
			//file
			callback(searchPath + fd.cFileName, param);
		}
		else if (wcscmp(fd.cFileName, L".") != 0 && wcscmp(fd.cFileName, L"..")  != 0)
		{
			//folder
			enumFile(searchPath + fd.cFileName + L"/", callback, param);
		}
		if (!FindNextFile(findFile, &fd))
		{
			::FindClose(findFile);
			return;
		}
	}
}

}