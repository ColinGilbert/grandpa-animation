#ifndef __DEMO_MULTITHREAD_RESOURCE_MANAGER_H__
#define __DEMO_MULTITHREAD_RESOURCE_MANAGER_H__

#include "IResourceManager.h"
#include "windows.h"
#include <hash_map>
#include <map>
#include <string>

namespace grp
{
class IFileLoader;
}
class DemoTexture;

class MultithreadResManager : public grp::IResourceManager
{
public:
	MultithreadResManager(grp::IFileLoader* fileLoader);
	virtual ~MultithreadResManager();

	//实现IResourceManager要求的接口函数
	virtual grp::IResource* getResource(const wchar_t* url, grp::ResourceType type, void* param0, void* param1);
	virtual void freeResource(grp::IResource* resource);

	void update(float elapsedTime);

	DemoTexture* grabTexture(const wchar_t* url);

	//返回是否已被释放
	bool dropTexture(DemoTexture* texture);

	void setEnableMap(bool enable);

private:
	void addResourceToMap(grp::IResource* resource);

	grp::IResource* createResource(const wchar_t* url, grp::ResourceType type, void* param0, void* param1);
	void destroyResource(grp::IResource* resource);

	void reportLeak();
	
	void clearRecycleMap();

private:
	grp::IFileLoader*	m_fileLoader;

	stdext::hash_map<std::wstring, grp::IResource*>	m_resourceMap;
	std::map<grp::IResource*, float>				m_recycleMap;	//resource->time remain
	CRITICAL_SECTION	m_criticalSection;
	bool				m_enableMap;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void MultithreadResManager::setEnableMap(bool enable)
{
	m_enableMap = enable;
}

#endif
