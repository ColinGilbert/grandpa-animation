#include "Precompiled.h"
#include "DefaultResourceManager.h"
#include "Resource.h"

namespace grp
{

///////////////////////////////////////////////////////////////////////////////////////////////////
DefaultResourceManager::DefaultResourceManager(ResourceFactory* factory)
	: m_factory(factory)
{
	WRITE_LOG(INFO, GT("Resource manager constructed."));
#ifdef _GRP_WIN32_THREAD_SAFE
	::InitializeCriticalSection(&m_cs);
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////
DefaultResourceManager::~DefaultResourceManager()
{
	reportLeak();
#ifdef _GRP_WIN32_THREAD_SAFE
	::DeleteCriticalSection(&m_cs);
#endif
	WRITE_LOG(INFO, GT("Resource manager destructed."));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
IResource* DefaultResourceManager::getResource(const Char* url, ResourceType type, void* param0, void* param1)
{
	{
		SCOPE_LOCK;

		HASH_MAP(STRING, IResource*)::const_iterator found = m_resourceMap.find(url);
		if (found != m_resourceMap.end())
		{
			return found->second;
		}
	}
	if (m_factory != NULL)
	{
		IResource* resource = m_factory->createResource(url, type, param0, param1);
		if (resource != NULL)
		{
			addResourceToMap(resource);
		}
		return resource;
	}
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void DefaultResourceManager::freeResource(IResource* resource)
{
	const STRING& url = static_cast<const Resource*>(resource)->getResourceUrlStr();
	{
		SCOPE_LOCK;

		HASH_MAP(STRING, IResource*)::iterator found = m_resourceMap.find(url);
		if (found != m_resourceMap.end())
		{
			m_resourceMap.erase(found);
		}
	}
	if (m_factory != NULL)
	{
		m_factory->destroyResource(resource);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void DefaultResourceManager::addResourceToMap(IResource* resource)
{
	const STRING& url = static_cast<Resource*>(resource)->getResourceUrlStr();

	SCOPE_LOCK;

	m_resourceMap[url] = resource;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void DefaultResourceManager::reportLeak()
{
	for (HASH_MAP(STRING, IResource*)::iterator iter = m_resourceMap.begin();
		 iter != m_resourceMap.end();
		 ++iter)
	{
		WRITE_LOG_HINT(WARNING, GT("Resource leak detected:"), iter->first.c_str());
	}
}

}
