#include "Precompiled.h"
#include "Core.h"
#include "Grandpa.h"
#include "DefaultAllocator.h"
#include "DefaultFileLoader.h"
#include "DefaultResourceManager.h"
#include "ResourceFactory.h"
#include "ModelResource.h"
#include "Model.h"
#include "StandaloneRigidMesh.h"
#include "Performance.h"

namespace grp
{

bool g_initialized = false;
ResourceFactory* g_resourceFactory = NULL;

//logger is always external
ILogger* g_logger = NULL;

IAllocator* g_allocator = NULL;
bool g_externalAllocator = false;

IFileLoader* g_fileLoader = NULL;
bool g_externalFileProvider = false;

IResourceManager* g_resourceManager = NULL;
bool g_externalResourceManager = false;

#ifdef GRANDPA_SQRT_TABLE
unsigned char g_sqrtTable[256][256];
void initializeSqrtTable();
#endif

AnimationSampleType g_animationSampleType = SAMPLE_SPLINE;

///////////////////////////////////////////////////////////////////////////////////////////////////
bool initialize(ILogger* logger, IFileLoader* fileLoader,
				IAllocator* allocator, IResourceManager* resourceManager,
				AnimationSampleType sampleType)
{
	PERF_NODE_FUNC();

	if (g_initialized)
	{
		return false;
	}

	g_animationSampleType = sampleType;
	g_logger = logger;
	
	if (allocator != NULL)
	{
		g_externalAllocator = true;
		g_allocator = allocator;
	}
	else
	{
		g_allocator = new DefaultAllocator;
	}

	if (fileLoader != NULL)
	{
		g_externalFileProvider = true;
		g_fileLoader = fileLoader;
	}
	else
	{
		g_fileLoader = GRP_NEW DefaultFileLoader;
	}

	g_resourceFactory = GRP_NEW ResourceFactory(g_fileLoader);

	if (resourceManager != NULL)
	{
		g_externalResourceManager = true;
		g_resourceManager = resourceManager;
	}
	else
	{
		g_resourceManager = GRP_NEW DefaultResourceManager(g_resourceFactory);
	}

#ifdef GRANDPA_SQRT_TABLE
	initializeSqrtTable();
#endif

	g_initialized = true;
	WRITE_LOG(INFO, GT("Grandpa initialized."));
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void destroy()
{
	if (!g_initialized)
	{
		return;
	}
	g_initialized = false;

	if (!g_externalResourceManager)
	{
		GRP_DELETE(g_resourceManager);
	}
	g_resourceManager = NULL;

	GRP_DELETE(g_resourceFactory);
	g_resourceFactory = NULL;

	if (!g_externalFileProvider)
	{
		GRP_DELETE(g_fileLoader);
	}
	g_fileLoader = NULL;

	if (!g_externalAllocator)
	{
		delete g_allocator;
	}
	g_allocator = NULL;
	
	WRITE_LOG(INFO, GT("Grandpa destroyed."));

	g_logger = NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
IResource* grabResource(const Char* url, ResourceType type, void* param0, void* param1)
{
	assert(g_resourceManager != NULL);
	IResource* resource = g_resourceManager->getResource(url, type, param0, param1);
	if (resource != NULL)
	{
		static_cast<Resource*>(resource)->grab();
	}
	return resource;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void dropResource(IResource* resource)
{
	assert(resource != NULL);
	static_cast<Resource*>(resource)->drop();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
IModel* createModel(IResource* resource, IEventHandler* eventHandler)
{
	if (resource->getResourceState() == RES_STATE_BROKEN
		|| resource->getResourceType() != RES_TYPE_MODEL)
	{
		return NULL;
	}
	return GRP_NEW Model(static_cast<const ModelResource*>(resource), eventHandler);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
IModel* createModel(const Char* url, IEventHandler* eventHandler, void* param0, void* param1)
{
	IResource* modelResource = grabResource(url, RES_TYPE_MODEL, param0, param1);
	if (modelResource == NULL)
	{
		return NULL;
	}
	IModel* model = createModel(modelResource, eventHandler);
	dropResource(modelResource);
	return model;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void destroyModel(IModel* model)
{
	GRP_DELETE(static_cast<Model*>(model));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
IMesh* createMesh(IResource* resource)
{
	if (resource->getResourceState() != RES_STATE_COMPLETE
		|| resource->getResourceType() != RES_TYPE_RIGID_MESH)
	{
		return NULL;
	}
	StandaloneRigidMesh* mesh = GRP_NEW StandaloneRigidMesh(static_cast<const RigidMeshResource*>(resource));
	mesh->build();
	mesh->calculateBoundingBox();
	return mesh;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
IMesh* createMesh(const Char* url, void* param0, void* param1)
{
	IResource* meshResource = grabResource(url, RES_TYPE_RIGID_MESH, param0, param1);
	if (meshResource == NULL)
	{
		return NULL;
	}
	IMesh* mesh = createMesh(meshResource);
	dropResource(meshResource);
	return mesh;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void destroyMesh(IMesh* mesh)
{
	if (static_cast<Mesh*>(mesh)->checkType(MESH_STANDALONE))
	{
		GRP_DELETE(static_cast<StandaloneRigidMesh*>(mesh));
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////
GRANDPA_API IMaterial* createMaterial( IResource* resource )
{
    if (resource->getResourceState() == RES_STATE_BROKEN
        || resource->getResourceType() != RES_TYPE_MATERIAL)
    {
        return NULL;
    }
    Material* newMaterial = GRP_NEW Material();
    newMaterial->setResource( static_cast<MaterialResource*>(resource));
    return newMaterial;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
GRANDPA_API IMaterial* createMaterial( const Char* url, void* param0 /*= NULL*/, void* param1 /*= NULL*/ )
{
    IResource* materialResource = grabResource(url, RES_TYPE_MATERIAL, param0, param1);
    if (materialResource == NULL)
    {
        return NULL;
    }

    IMaterial* newMaterial = createMaterial( materialResource );
    dropResource( materialResource );
    return newMaterial;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
GRANDPA_API void destroyMaterial( IMaterial* material )
{
    GRP_DELETE(static_cast<Material*>(material));
}
///////////////////////////////////////////////////////////////////////////////////////////////////
GRANDPA_API IResource* createResource(const Char* url, ResourceType type, void* param0, void* param1)
{
	if (g_resourceFactory != NULL)
	{
		return g_resourceFactory->createResource(url, type, param0, param1);
	}
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
GRANDPA_API void destroyResource(IResource* resource)
{
	if (g_resourceFactory != NULL)
	{
		return g_resourceFactory->destroyResource(resource);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void setProfiler(PerfManager* perfManager)
{
#ifdef _PERFORMANCE
	PerfManager::setTheOne(perfManager);
#endif
}

#ifdef GRANDPA_SQRT_TABLE
///////////////////////////////////////////////////////////////////////////////////////////////////
void initializeSqrtTable()
{
	//initializing sqrt table
	for (size_t i = 0; i < 256; ++i)
	{
		float fi = i / 127.5f - 1.0f;
		for (size_t j = 0; j < 256; ++j)
		{
			float fj = j / 127.5f - 1.0f;
			float sqr = 1.0f - fi * fi - fj * fj;
			if (sqr > 0.0f)
			{
				g_sqrtTable[i][j] = (unsigned char)(sqrtf(sqr) * 255);
			}
			else
			{
				g_sqrtTable[i][j] = 0;
			}
		}
	}
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
template <typename T>
ISpline<T>* createSpline()
{
	return GRP_NEW Spline<T>;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
template <typename T>
void destroySpline(ISpline<T>* spline)
{
	GRP_DELETE(static_cast<Spline<T>*>(spline));
}

template GRANDPA_API ISpline<float>* createSpline();
template GRANDPA_API ISpline<Vector2>* createSpline();
template GRANDPA_API ISpline<Vector3>* createSpline();
template GRANDPA_API ISpline<Vector4>* createSpline();
template GRANDPA_API ISpline<Quaternion>* createSpline();

template GRANDPA_API void destroySpline(ISpline<float>*);
template GRANDPA_API void destroySpline(ISpline<Vector2>*);
template GRANDPA_API void destroySpline(ISpline<Vector3>*);
template GRANDPA_API void destroySpline(ISpline<Vector4>*);
template GRANDPA_API void destroySpline(ISpline<Quaternion>*);

}
