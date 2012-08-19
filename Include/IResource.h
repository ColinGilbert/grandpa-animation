#ifndef __GRP_I_RESOURCE_H__
#define __GRP_I_RESOURCE_H__

namespace grp
{

enum ResourceType
{
	RES_TYPE_MODEL = 0,
	RES_TYPE_PART,
	RES_TYPE_SKELETON,
	RES_TYPE_ANIMATION,
	RES_TYPE_SKINNED_MESH,
	RES_TYPE_RIGID_MESH,
	RES_TYPE_MATERIAL,
	RES_TYPE_COUNT,
	RES_TYPE_USER0 = RES_TYPE_COUNT,	//for user resource
	RES_TYPE_USER1,
	RES_TYPE_USER2,
	RES_TYPE_USER3,
	RES_TYPE_USER4,
	RES_TYPE_USER5,
	RES_TYPE_USER6,
	RES_TYPE_USER7,
	RES_TYPE_USER8,
	RES_TYPE_USER9,
	RES_TYPE_USER10,
	RES_TYPE_USER11,
	RES_TYPE_USER12,
	RES_TYPE_USER13,
	RES_TYPE_USER14,
	RES_TYPE_USER15,
	RES_TYPE_USER16
};

inline bool isGrandpaResource(ResourceType type)
{
	return (type < RES_TYPE_USER0);
}

enum ResourceState
{
	RES_STATE_LOADING = 0,
	RES_STATE_COMPLETE,
	RES_STATE_BROKEN
};

class IResource
{
public:
	virtual ResourceType getResourceType() const = 0;

	virtual const Char* getResourceUrl() const = 0;

	virtual const Char* getFilePath() const = 0;

	virtual ResourceState getResourceState() const = 0;

	virtual bool allComplete() const = 0;

	virtual void setPriority(float priority) const = 0;
	virtual float getPriority() const = 0;

	virtual void setUserData(const void* data) = 0;
	virtual const void* getUserData() const = 0;

	virtual float getFreeDelay() const = 0;

protected:
	virtual ~IResource(){}
};

}

#endif
