#ifndef __GRP_DEFAULT_FILE_LOADER_H__
#define __GRP_DEFAULT_FILE_LOADER_H__

#include "IFileLoader.h"

namespace grp
{

class DefaultFileLoader : public IFileLoader
{
public:
	virtual void loadFile(IResource* resource, IFileCallback* callback, void* param0, void* param1);
	virtual void unloadFile(IResource* resource);
};

}

#endif
