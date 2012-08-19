#ifndef __GRP_I_FILE_LOADER_H__
#define __GRP_I_FILE_LOADER_H__

#include <istream>

namespace grp
{

class IResource;
class IFileCallback;

class IFileLoader
{
public:
	virtual ~IFileLoader(){}

	virtual void loadFile(IResource* resource, IFileCallback* callback, void* param0, void* param1) = 0;

	//do not call any callback after this
	virtual void unloadFile(IResource* resource) = 0;
};

class IFileCallback
{
public:
	virtual bool wantBuffer() const = 0;

	virtual void onFileComplete(const Char* path, void* param0, void* param1) = 0;

	virtual void onFileComplete(const void* buffer, unsigned long size, void* param0, void* param1) = 0;

	virtual void onFileNotFound(void* param0, void* param1) = 0;

protected:
	virtual ~IFileCallback(){}
};

}

#endif
