#ifndef __GRP_I_RESOURCE_MANAGER_H__
#define __GRP_I_RESOURCE_MANAGER_H__

#include "IResource.h"

namespace grp
{

class IResource;

class IResourceManager
{
public:
	virtual ~IResourceManager(){}

	virtual IResource* getResource(const Char* url, ResourceType type, void* param0, void* param1) = 0;

	virtual void freeResource(IResource* resource) = 0;
};

}

#endif
