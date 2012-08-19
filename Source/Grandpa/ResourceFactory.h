#ifndef __GRP_RESOURCE_FACTORY_H__
#define __GRP_RESOURCE_FACTORY_H__

#include <istream>

namespace grp
{
class IResource;
class IFileLoader;
enum ResourceType;

class ResourceFactory
{
public:
	ResourceFactory(IFileLoader* fileLoader);
	~ResourceFactory();

	IResource* createResource(const Char* url, ResourceType type, void* param0, void* param1, bool loadFromFile = true);

	void destroyResource(IResource* resource);

private:
	IFileLoader*		m_fileLoader;
};

}

#endif
