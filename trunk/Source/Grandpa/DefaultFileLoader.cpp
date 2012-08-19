#include "Precompiled.h"
#include "DefaultFileLoader.h"
#include <fstream>
#include <sstream>

namespace grp
{

///////////////////////////////////////////////////////////////////////////////////////////////////
void DefaultFileLoader::loadFile(IResource* resource, IFileCallback* callback, void* param0, void* param1)
{
	if (callback == NULL)
	{
		return;
	}
	std::fstream file;
	file.open(resource->getFilePath(), std::ios_base::in | std::ios_base::binary);
	if (file.is_open())
	{
		file.seekg(0, std::ios::end);
		unsigned long fileSize = (unsigned long)file.tellg();
		file.seekg(0, std::ios::beg);
		char* buffer = new char[fileSize];
		file.read(buffer, fileSize);
		callback->onFileComplete(buffer, fileSize, param0, param1);
		delete[] buffer;
	}
	else
	{
		callback->onFileNotFound(param0, param1);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void DefaultFileLoader::unloadFile(IResource* resource)
{
}

}
