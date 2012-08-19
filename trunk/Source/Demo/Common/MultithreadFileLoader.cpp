#include "Grandpa.h"
#include "MultithreadFileLoader.h"
#include "DemoTexture.h"
#include <fstream>
#include <sstream>

///////////////////////////////////////////////////////////////////////////////////////////////////
MultithreadFileLoader::MultithreadFileLoader(IDirect3DDevice9* device)
	:	m_d3dDevice(device)
	,	m_event(NULL)
	,	m_thread(NULL)
	,	m_multithread(true)
	,	m_endThread(false)
{
	::InitializeCriticalSection(&m_criticalSection);
	m_event = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	DWORD dwThreadID = 0;
	m_thread = ::CreateThread(NULL,
								0,
								(LPTHREAD_START_ROUTINE)(MultithreadFileLoader::threadFunc),
								this,
								0,
								&dwThreadID);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
MultithreadFileLoader::~MultithreadFileLoader()
{
	m_endThread = true;
	::SetEvent(m_event);
	::WaitForSingleObject(m_thread, INFINITE);

	::CloseHandle(m_thread);
	::CloseHandle(m_event);
	::DeleteCriticalSection(&m_criticalSection);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void MultithreadFileLoader::loadFile(grp::IResource* resource, grp::IFileCallback* callback,
									 void* param0, void* param1)
{
	if (!m_multithread)
	{
		buildResource(resource, callback, resource->getResourceUrl(), param0, param1);
	}
	else
	{
		FileRequest request;
		request.resource = resource;
		request.callback = callback;
		request.url = resource->getResourceUrl();
		request.param0 = param0;
		request.param1 = param1;
		::EnterCriticalSection(&m_criticalSection);
		//加载策略：按资源类型优先级插入任务队列
		std::list<FileRequest>::iterator insertPos = m_tasks.end();
		for (std::list<FileRequest>::iterator iter = m_tasks.begin();
			iter != m_tasks.end();
			++iter)
		{
			if (request < *iter)
			{
				insertPos = iter;
				break;
			}
		}
		m_tasks.insert(insertPos, request);
		::LeaveCriticalSection(&m_criticalSection);
		::SetEvent(m_event);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
DWORD WINAPI MultithreadFileLoader::threadFunc(LPVOID pointer)
{
	MultithreadFileLoader* loader = static_cast<MultithreadFileLoader*>(pointer);
	while (!loader->m_endThread)
	{
		::EnterCriticalSection(&(loader->m_criticalSection));
		if (loader->m_tasks.empty())
		{
			::LeaveCriticalSection(&(loader->m_criticalSection));
			::WaitForSingleObject(loader->m_event, INFINITE);
			continue;
		}
		FileRequest request = loader->m_tasks.front();
		loader->m_tasks.pop_front();
		::LeaveCriticalSection(&(loader->m_criticalSection));

		loader->buildResource(request.resource, request.callback, request.url.c_str(), request.param0, request.param1);
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void MultithreadFileLoader::buildResource(grp::IResource* resource, grp::IFileCallback* callback,
										   const wchar_t* url, void* param0, void* param1)
{
	//如果想看清加载过程，可以打开这句
	//::Sleep(100);
	if (resource->getResourceType() == RES_TYPE_TEXTURE)
	{
		DemoTexture* texture = static_cast<DemoTexture*>(resource);
		texture->build(m_d3dDevice);
		return;
	}
	if (callback == NULL)
	{
		return;
	}
	std::fstream file;
	file.open(url, std::ios_base::in | std::ios_base::binary);
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
