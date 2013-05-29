#ifndef __GRP_MODEL_RESOURCE_H__
#define __GRP_MODEL_RESOURCE_H__

#include "Resource.h"
#include "Property.h"
#include <vector>
#include <map>

namespace slim
{
class XmlNode;
}

namespace grp
{
class PartResource;
class SkeletonFile;
class AnimationFile;
template<class T, ResourceType resType> class ContentResource;
typedef ContentResource<SkeletonFile, RES_TYPE_SKELETON> SkeletonResource;
typedef ContentResource<AnimationFile, RES_TYPE_ANIMATION> AnimationResource;

struct PartInfo
{
	STRING				slot;
	const PartResource*	resource;
};

enum AnimationEventType
{
	ANIMATION_EVENT_START = 0,
	ANIMATION_EVENT_END,
	ANIMATION_EVENT_TIME
};

struct AnimationEvent
{
	AnimationEventType	type;
	STRING				name;
	float				time;	//only available when typye == ANIMATION_EVENT_TIME
	Property			property;
};

struct AnimationInfo
{
	STRING						filename;	//for async loading
	STRING						slot;
	const AnimationResource*	resource;
	VECTOR(AnimationEvent)		events;
	float						startTime;
	float						endTime;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
class ModelResource : public Resource
{
public:
	ModelResource(bool managed);
	virtual ~ModelResource();

	virtual ResourceType getResourceType() const;

	virtual bool allComplete() const;

	virtual void setPriority(float priority) const;

	virtual bool importXml(const void* buffer, size_t size, void* param0, void* param1);
	void importXmlNode(slim::XmlNode* node, void* param0, void* param1);

	const SkeletonResource* getSkeletonResource() const;

	const VECTOR(PartInfo)& getPartInfoVector() const;

	const AnimationInfo* getAnimationInfo(const STRING& slot) const;

	bool hasAnimation(const STRING& slot) const;

	IProperty* getProperty() const;

	const MAP(STRING, AnimationInfo)& getAnimationInfoMap() const;

private:
	void readPartInfo(slim::XmlNode* node);
	void readAnimationInfo(slim::XmlNode* node);
	void readAnimationEvent(slim::XmlNode* node, AnimationInfo& animationInfo);

	bool updateCompleteState() const;

private:
	Property					m_userProperty;
	const SkeletonResource*		m_skeletonResource;
	VECTOR(PartInfo)			m_partInfo;
	MAP(STRING, AnimationInfo)	m_animationInfo;
	void*						m_userParam0;
	void*						m_userParam1;
	mutable bool				m_allComplete;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
inline ResourceType ModelResource::getResourceType() const
{
	return RES_TYPE_MODEL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline bool ModelResource::allComplete() const
{
	if (m_allComplete)
	{
		return true;
	}
	return (m_allComplete = updateCompleteState());
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline const SkeletonResource* ModelResource::getSkeletonResource() const
{
	return m_skeletonResource;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline const VECTOR(PartInfo)& ModelResource::getPartInfoVector() const
{
	return m_partInfo;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline bool ModelResource::hasAnimation(const STRING& slot) const
{
	return (m_animationInfo.find(slot) != m_animationInfo.end());
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline IProperty* ModelResource::getProperty() const
{
	return const_cast<Property*>(&m_userProperty);
}

}

#endif
