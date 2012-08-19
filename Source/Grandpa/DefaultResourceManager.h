#ifndef __GRP_DEFAULT_RESOURCE_MANAGER_H__
#define __GRP_DEFAULT_RESOURCE_MANAGER_H__

#include "IResourceManager.h"
#include <string>
#if defined (_MSC_VER)
	#include <hash_map>
#elif defined (__GNUC__)
	#include <ext/hash_map>
#endif

namespace grp
{

class ResourceFactory;

class DefaultResourceManager : public IResourceManager
{
public:
	DefaultResourceManager(ResourceFactory* factory);
	virtual ~DefaultResourceManager();

	virtual IResource* getResource(const Char* url, ResourceType type, void* param0, void* param1);

	virtual void freeResource(IResource* resource);

private:
	void addResourceToMap(IResource* resource);

	void reportLeak();

private:
#ifdef _GRP_WIN32_THREAD_SAFE
	class Lock
	{
	public:
		Lock(LPCRITICAL_SECTION cs) : m_cs(cs){::EnterCriticalSection(m_cs);}
		~Lock()	{::LeaveCriticalSection(m_cs);}
		LPCRITICAL_SECTION m_cs;
	};
	CRITICAL_SECTION	m_cs;
#endif
	ResourceFactory*				m_factory;
	HASH_MAP(STRING, IResource*)	m_resourceMap;
};

#ifdef _GRP_WIN32_THREAD_SAFE
	#define SCOPE_LOCK	Lock l(&m_cs)
#else
	#define SCOPE_LOCK
#endif

}

#endif
