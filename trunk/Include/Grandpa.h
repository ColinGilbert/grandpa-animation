#ifndef __GRP_GRANDPA_H__
#define __GRP_GRANDPA_H__

#include "Define.h"

#include "math.h"

#include "Vector.h"
#include "Line.h"
#include "AaBox.h"
#include "Quaternion.h"
#include "Matrix.h"
#include "Triangle.h"
#include "Frustum.h"

#include "IResourceManager.h"
#include "IFileLoader.h"
#include "IAllocator.h"
#include "ILogger.h"

#include "IResource.h"
#include "IModel.h"
#include "IEventHandler.h"
#include "IPart.h"
#include "IAnimation.h"
#include "ISkeleton.h"
#include "IMesh.h"
#include "ISkin.h"
#include "IMaterial.h"
#include "IProperty.h"
#include "ISpline.h"

class PerfManager;

namespace grp
{

GRANDPA_API bool initialize(ILogger* logger = NULL, IFileLoader* fileLoader = NULL,
							 IAllocator* allocator = NULL, IResourceManager* resourceManager = NULL,
							 AnimationSampleType sampleType = SAMPLE_LINEAR);
GRANDPA_API void destroy();

GRANDPA_API IResource* grabResource(const Char* url, ResourceType type, void* param0 = NULL, void* param1 = NULL);
GRANDPA_API void dropResource(IResource* resource);

GRANDPA_API IResource* createResource(const Char* url, ResourceType type, void* param0 = NULL, void* param1 = NULL);
GRANDPA_API void destroyResource(IResource* resource);

GRANDPA_API IModel* createModel(IResource* resource, IEventHandler* eventHandler = NULL);
GRANDPA_API IModel* createModel(const Char* url, IEventHandler* eventHandler = NULL, void* param0 = NULL, void* param1 = NULL);
GRANDPA_API void destroyModel(IModel* entity);

GRANDPA_API IMesh* createMesh(IResource* resource);
GRANDPA_API IMesh* createMesh(const Char* url, void* param0 = NULL, void* param1 = NULL);
GRANDPA_API void destroyMesh(IMesh* mesh);

GRANDPA_API IMaterial* createMaterial(IResource* resource);
GRANDPA_API IMaterial* createMaterial(const Char* url, void* param0 = NULL, void* param1 = NULL);
GRANDPA_API void destroyMaterial(IMaterial* material);

//support spline of float, Vector2, Vector3, Vector4, Quaternino, Euler
template<typename T>
GRANDPA_API ISpline<T>* createSpline();
template<typename T>
GRANDPA_API void destroySpline(ISpline<T>* spline);

GRANDPA_API void setProfiler(PerfManager* perfManager);

}

#endif
