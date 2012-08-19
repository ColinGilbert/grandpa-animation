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
		//�ѻ���
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
	//��ʱ30�����٣������Ȼ��resource map�С���Ҫ�Ļ�����Ϊ��ͬ���͵���Դ���ò�ͬ����ʱ
	m_recycleMap[resource] = 30.0f;
	::LeaveCriticalSection(&m_criticalSection);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void MultithreadResManager::update(float elapsedTime)
{
	//������ԴӦ�úܿ죬��ֵ��ÿ��ѭ������һ�ιؼ���
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
		//grandpa��Դ����grandpa����
		return grp::createResource(url, type, param0, param1);
	}

	if (type == RES_TYPE_TEXTURE)
	{
		//��grandpa����Դ����Ҫ�Լ�����
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
		//��ñ�֤file loader��res manager֮ǰ�������������������ȷ���������첽�����е���Դ
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
