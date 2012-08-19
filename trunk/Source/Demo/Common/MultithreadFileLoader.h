#ifndef __DEMO_MULTITHREAD_FILE_LOADER_H__
#define __DEMO_MULTITHREAD_FILE_LOADER_H__

#include "IFileLoader.h"
#include "IResource.h"
#include "windows.h"
#include <list>
#include <string>

namespace grp
{
class IResource;
}
struct IDirect3DDevice9;

const int ResourcePriority[grp::RES_TYPE_COUNT + 1] = 
{
	0,	//RES_TYPE_MODEL
	1,	//RES_TYPE_PART
	2,	//RES_TYPE_SKELETON
	4,	//RES_TYPE_ANIMATION
	2,	//RES_TYPE_SKINNED_MESH
	2,	//RES_TYPE_RIGID_MESH
	2,	//RES_TYPE_MATERIAL
	3	//RES_TYPE_TEXTURE
};

struct FileRequest
{
	std::wstring		url;
	grp::IResource*		resource;
	grp::IFileCallback*	callback;
	void*				param0;
	void*				param1;

	bool operator < (const FileRequest& other)
	{
		return (ResourcePriority[resource->getResourceType()]
				< ResourcePriority[other.resource->getResourceType()]);
	}
};

class MultithreadFileLoader : public grp::IFileLoader
{
public:
	MultithreadFileLoader(IDirect3DDevice9* device);
	virtual ~MultithreadFileLoader();

	virtual void loadFile(grp::IResource* resource, grp::IFileCallback* callback, void* param0, void* param1);

	virtual void unloadFile(grp::IResource*){}

	void enableMultithread(bool enable);

private:
	static DWORD WINAPI threadFunc(LPVOID pointer);

	void buildResource(grp::IResource* resource, grp::IFileCallback* callback, const wchar_t* url, void* param0, void* param1);

private:
	std::list<FileRequest>	m_tasks;

	CRITICAL_SECTION		m_criticalSection;
	HANDLE					m_thread;
	HANDLE					m_event;
	IDirect3DDevice9*		m_d3dDevice;

	bool					m_multithread;
	bool					m_endThread;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void MultithreadFileLoader::enableMultithread(bool enable)
{
	m_multithread = enable;
}

#endif
