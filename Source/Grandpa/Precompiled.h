#ifndef __GRP_PRECOMPILED_H__
#define __GRP_PRECOMPILED_H__

#ifdef _GRP_WIN32_THREAD_SAFE
	#include <windows.h>
	#undef min
	#undef max
#endif

#include <cassert>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <algorithm>
#include <istream>
#include <ostream>
#include <fstream>
//#include <functional>
#include <stdio.h>
#include <new>
#if defined (_MSC_VER)
	#include <hash_map>
#elif defined (__GNUC__)
	#include <ext/hash_map>
#endif

#include "Define.h"

#include "IAllocator.h"
#include "ILogger.h"
#include "IFileLoader.h"
#include "IResourceManager.h"

inline void* operator new(size_t size, grp::IAllocator& allocator)
{
	return allocator.allocateChunk(size);
}

inline void operator delete(void* p, grp::IAllocator& allocator)
{
	allocator.deallocateChunk(p);
}

namespace grp
{

extern IAllocator* g_allocator;
extern ILogger* g_logger;

#ifdef _MSC_VER
#define GRP_NEW new(*grp::g_allocator)
#else
#define GRP_NEW new
#endif

template<class Type>
void GRP_DELETE(const Type* p)
{
	if (p)
	{
		p->~Type();
		g_allocator->deallocateChunk(p);
	}
}

#define WRITE_LOG(type, log)	if (g_logger != NULL) g_logger->write(LOG_##type, log);

#if defined (GRP_USE_WCHAR)
	#define WRITE_LOG_HINT(type, log, hint)	if (g_logger != NULL) { wchar_t out[256]; swprintf(out, sizeof(out), L"%s %s", log, hint); g_logger->write(LOG_##type, out); }
#else
	#define WRITE_LOG_HINT(type, log, hint)	if (g_logger != NULL) { char out[256]; sprintf(out, "%s %s", log, hint); g_logger->write(LOG_##type, out); }
#endif

}

#include "StlAllocator.h"

#include "Core.h"

#include "IResource.h"
#include "IModel.h"
#include "IPart.h"
#include "IMesh.h"
#include "IAnimation.h"
#include "ISkeleton.h"
#include "IMaterial.h"
#include "IProperty.h"
#include "IEventHandler.h"
#include "ISpline.h"

#include "ChunkFileIo.h"
#include "ContentFile.h"
#include "AnimationFile.h"
#include "SkeletonFile.h"
#include "MeshFile.h"
#include "SkinnedMeshFile.h"

#include "Resource.h"
#include "ResourceInstance.h"
#include "ContentResource.h"
#include "MaterialResource.h"
#include "PartResource.h"
#include "ModelResource.h"
#include "ResourceFactory.h"

#include "ReferenceCounted.h"
#include "Animation.h"
#include "Skeleton.h"
#include "IkSolver.h"
#include "Mesh.h"
#include "SkinnedMesh.h"
#include "RigidMesh.h"
#include "Part.h"
#include "Model.h"

#include "AnimationSampler.h"
#include "StepSampler.h"
#include "LinearSampler.h"
#include "SplineSampler.h"

#include "DefaultAllocator.h"
#include "DefaultFileLoader.h"
#include "DefaultResourceManager.h"

#include "Property.h"

#endif
