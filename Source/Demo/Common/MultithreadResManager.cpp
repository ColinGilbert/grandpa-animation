#include "Grandpa.h"
#include "MultithreadResManager.h"
#include "DemoTexture.h"
#include "Windows.h"
#include <cassert>

///////////////////////////////////////////////////////////////////////////////////////////////////
MultithreadResManager::MultithreadResManager(grp::IFileLoader* fileLoader)
	: m_fileLoader(fileLoader)
	, m_enableMap(true)
{
	::InitializeCriticalSection(&m_criticalSection);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
MultithreadResManager::~MultithreadResManager()
{
	clearRecycleMap();
	reportLeak();
	::DeleteCriticalSection(&m_criticalSection);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
grp::IResource* MultithreadResManager::getResource(const wchar_t* url, grp::ResourceType type,
												   void* param0, void* param1)
{
	::EnterCriticalSection(&m_criticalSection);
	stdext::hash_map<std::wstring, grp::IResource*>::const_iterator found = m_resourceMap.find(url);
	if (found != m_resourceMap.end())
	{
		//已缓存
		grp::IResource* resource = found->second;
		assert(resource != NULL);
		std::map<grp::IResource*, float>::iterator recycleIter = m_recycleMap.find(resource);
		if (recycleIter != m_recycleMap.end())
		{
			m_recycleMap.erase(recycleIter);
		}
		::LeaveCriticalSection(&m_criticalSection);
		return resource;
	}
	::LeaveCriticalSection(&m_criticalSection);

	grp::IResource* resource = createResource(url, type, param0, param1);
	if (resource != NULL)
	{
		if (m_enableMap)
		{
			addResourceToMap(resource);
		}
	}
	return resource;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void MultithreadResManager::freeResource(grp::IResource* resource)
{
	assert(resource != NULL);
	::EnterCriticalSection(&m_criticalSection);
	//延时30秒销毁，其间仍然在resource map中。需要的话可以为不同类型的资源设置不同的延时
	m_recycleMap[resource] = 30.0f;
	::LeaveCriticalSection(&m_criticalSection);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void MultithreadResManager::update(float elapsedTime)
{
	//销毁资源应该很快，不值得每次循环进出一次关键区
	::EnterCriticalSection(&m_criticalSection);

	for (std::map<grp::IResource*, float>::iterator iter = m_recycleMap.begin();
		iter != m_recycleMap.end();)
	{
		grp::IResource* resource = iter->first;
		assert(resource != NULL);

		float& timeRemain = iter->second;
		timeRemain -= elapsedTime;

		std::map<grp::IResource*, float>::iterator backupIter = iter;
		++iter;

		if (timeRemain > 0.0f)
		{
			continue;
		}
		if (resource->getResourceState() == grp::RES_STATE_LOADING)
		{
			resource->setPriority(FLT_MAX);
			continue;
		}
		const wchar_t* url = resource->getResourceUrl();
		stdext::hash_map<std::wstring, grp::IResource*>::iterator found = m_resourceMap.find(url);
		if (found != m_resourceMap.end())
		{
			m_resourceMap.erase(found);
		}
		destroyResource(resource);

		m_recycleMap.erase(backupIter);
	}
	::LeaveCriticalSection(&m_criticalSection);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
DemoTexture* MultithreadResManager::grabTexture(const wchar_t* url)
{
	grp::IResource* resource = getResource(url, RES_TYPE_TEXTURE, NULL, NULL);
	assert(resource != NULL);
	DemoTexture* texture = static_cast<DemoTexture*>(resource);
	texture->grab();
	return texture;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool MultithreadResManager::dropTexture(DemoTexture* texture)
{
	assert(texture != NULL);
	if (texture->drop())
	{
		freeResource(texture);
		return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void MultithreadResManager::addResourceToMap(grp::IResource* resource)
{
	const std::wstring& url = resource->getResourceUrl();
	::EnterCriticalSection(&m_criticalSection);
	m_resourceMap[url] = resource;
	::LeaveCriticalSection(&m_criticalSection);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
grp::IResource* MultithreadResManager::createResource(const wchar_t* url, grp::ResourceType type, void* param0, void* param1)
{
	if (grp::isGrandpaResource(type))
	{
		//grandpa资源交给grandpa创建
		return grp::createResource(url, type, param0, param1);
	}

	if (type == RES_TYPE_TEXTURE)
	{
		//非grandpa的资源类型要自己处理
		DemoTexture* texture = new DemoTexture;
		texture->setResourceUrl(url);
		if (m_fileLoader != NULL)
		{
			m_fileLoader->loadFile(texture, NULL, NULL, NULL);
		}
		return texture;
	}
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void MultithreadResManager::destroyResource(grp::IResource* resource)
{
	assert(resource != NULL);
	if (grp::isGrandpaResource(resource->getResourceType()))
	{
		grp::destroyResource(resource);
	}
	else if (resource->getResourceType() == RES_TYPE_TEXTURE)
	{
		static_cast<DemoTexture*>(resource)->destroy();
	}	
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void MultithreadResManager::clearRecycleMap()
{
	::EnterCriticalSection(&m_criticalSection);

	std::map<grp::IResource*, float>::iterator iter = m_recycleMap.begin();
	while (iter != m_recycleMap.end())
	{
		grp::IResource* resource = iter->first;
		assert(resource != NULL);

		const wchar_t* url = resource->getResourceUrl();
		stdext::hash_map<std::wstring, grp::IResource*>::iterator found = m_resourceMap.find(url);
		if (found != m_resourceMap.end())
		{
			m_resourceMap.erase(found);
		}
		//最好保证file loader在res manager之前析构，这样到这里就能确保不会有异步加载中的资源
		//if (resource->getResourceState() != grp::RES_STATE_LOADING)
		{
			destroyResource(resource);
		}
		m_recycleMap.erase(iter);
		iter = m_recycleMap.begin();
	}
	::LeaveCriticalSection(&m_criticalSection);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void MultithreadResManager::reportLeak()
{
	::EnterCriticalSection(&m_criticalSection);
	for (stdext::hash_map<std::wstring, grp::IResource*>::iterator iter = m_resourceMap.begin();
		 iter != m_resourceMap.end();
		 ++iter)
	{
		::MessageBoxW(NULL, iter->first.c_str(), L"Resource leak detected!", MB_OK | MB_ICONWARNING);
	}
	::LeaveCriticalSection(&m_criticalSection);
}
