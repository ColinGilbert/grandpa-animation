#ifndef __GRP_CONTENT_RESOURCE_H__
#define __GRP_CONTENT_RESOURCE_H__

#include "Resource.h"
#include "SkeletonFile.h"
#include "AnimationFile.h"
#include "SkinnedMeshFile.h"
#include "RigidMeshFile.h"

namespace grp
{

class IResourceManager;

template<class T, ResourceType resType>
class ContentResource : public Resource, public T
{
public:
	ContentResource(bool managed);

	virtual ResourceType getResourceType() const;

	virtual bool importBinary(std::istream& input, void* param0, void* param1);
};

///////////////////////////////////////////////////////////////////////////////////////////////////
template<class T, ResourceType resType>
inline ContentResource<T, resType>::ContentResource(bool managed)
	: Resource(managed)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
template<class T, ResourceType resType>
inline ResourceType ContentResource<T, resType>::getResourceType() const
{
	return resType;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
template<class T, ResourceType resType>
inline bool ContentResource<T, resType>::importBinary(std::istream& input, void* param0, void* param1)
{
	//content file won't need param
	return T::importFrom(input);
}

typedef ContentResource<SkeletonFile, RES_TYPE_SKELETON> SkeletonResource;
typedef ContentResource<AnimationFile, RES_TYPE_ANIMATION> AnimationResource;		
typedef ContentResource<RigidMeshFile, RES_TYPE_RIGID_MESH> RigidMeshResource;
typedef ContentResource<SkinnedMeshFile, RES_TYPE_SKINNED_MESH> SkinnedMeshResource;

}

#endif
