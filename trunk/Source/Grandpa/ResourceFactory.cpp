#include "Precompiled.h"
#include "ResourceFactory.h"
#include "Resource.h"
#include "IFileLoader.h"
#include "ContentResource.h"
#include "ModelResource.h"
#include "PartResource.h"
#include "MaterialResource.h"
#include <cassert>
#include <string>

namespace grp
{

///////////////////////////////////////////////////////////////////////////////////////////////////
ResourceFactory::ResourceFactory(IFileLoader* fileLoader)
	: m_fileLoader(fileLoader)
{
	WRITE_LOG(INFO, GT("Resource factory constructed."));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
ResourceFactory::~ResourceFactory()
{
	WRITE_LOG(INFO, GT("Resource factory destructed."));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
IResource* ResourceFactory::createResource(const Char* url, ResourceType type, void* param0,
										   void* param1, bool loadFromFile)
{
	Resource* resource = NULL;
	switch (type)
	{
	case RES_TYPE_MODEL:
		resource = GRP_NEW ModelResource(loadFromFile);
		break;
	case RES_TYPE_PART:
		resource = GRP_NEW PartResource(loadFromFile);
		break;
	case RES_TYPE_SKELETON:
		resource = GRP_NEW SkeletonResource(loadFromFile);
		break;
	case RES_TYPE_ANIMATION:
		resource = GRP_NEW AnimationResource(loadFromFile);
		break;
	case RES_TYPE_SKINNED_MESH:
		resource = GRP_NEW SkinnedMeshResource(loadFromFile);
		break;
	case RES_TYPE_RIGID_MESH:
		resource = GRP_NEW RigidMeshResource(loadFromFile);
		break;
	case RES_TYPE_MATERIAL:
		resource = GRP_NEW MaterialResource(loadFromFile);
		break;
	default:
		WRITE_LOG(ERROR, GT("Unknown resource type."));
		return NULL;
	}

	resource->setResourceUrl(url);
	resource->setResourceState(RES_STATE_LOADING);

	if (loadFromFile)
	{
		assert(m_fileLoader != NULL);
		m_fileLoader->loadFile(resource, resource, param0, param1);
	}
	return resource;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ResourceFactory::destroyResource(IResource* resource)
{
	assert(resource != NULL);
	//file loader may be deleted before ResourceFactory
	//m_fileLoader->unloadFile(resource);
	GRP_DELETE(static_cast<Resource*>(resource));
}

}
