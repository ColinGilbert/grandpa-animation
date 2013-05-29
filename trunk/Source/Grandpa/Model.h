#ifndef __GRP_MODEL_H__
#define __GRP_MODEL_H__

#include "IModel.h"
#include "ResourceInstance.h"
#include <map>
#include <list>

namespace grp
{
class ModelResource;
class Skeleton;
class Part;
class Animation;
class IBone;

class AnimationFile;
template<class T, ResourceType resType> class ContentResource;
typedef ContentResource<AnimationFile, RES_TYPE_ANIMATION> AnimationResource;

template<typename T> void GRP_DELETE(const T* p);

///////////////////////////////////////////////////////////////////////////////////////////////////
class Model : public IModel, public ResourceInstance
{
	friend void GRP_DELETE<Model>(const Model* p);

public:
	Model(const ModelResource* resource, IEventHandler* eventHandler);

	virtual IResource* getResource() const;

	virtual void setTransform(const Matrix& transform);
	virtual const Matrix& getTransform() const;

	virtual void setFixedBoundingBox(const AaBox& box);
	virtual void unsetFixedBoundingBox();
	virtual const AaBox& getBoundingBox() const;

	virtual ISkeleton* getSkeleton();

	virtual IPart* setPart(const Char* slot, const IResource* resource);
	virtual IPart* setPart(const Char* slot, const Char* url, void* param = NULL);
	virtual IPart* findPart(const Char* slot) const;
	virtual void removePart(const Char* slot);
	virtual void removeAllParts();
	virtual IPart* getFirstPart(const Char** slot = NULL);
	virtual IPart* getNextPart(const Char** slot = NULL);

	virtual IAnimation* playAnimation(const Char* slot,
										AnimationMode mode,
										int priority = 1,
										int syncGroup = -1,
										float fadeinTime = 0.3f,
										float fadeoutTime = 0.3f);
	virtual IAnimation* findAnimation(const Char* slot) const;
	virtual bool stopAnimation(const Char* slot, float fadeoutTime = -1.0f);
	virtual void stopAllAnimations(float fadeoutTime = 0.3f);
	virtual bool isAnimationPlaying(const Char* slot) const;
	virtual bool hasAnimation(const Char* slot) const;

	virtual const Char* getFirstAnimationSlot() const;
	virtual const Char* getNextAnimationSlot(const Char* slot) const;

	virtual void setGlobalSkinning(bool enable);
	virtual bool isGlobalSkinning() const;

	virtual void setLodTolerance(float tolerance);
	virtual float getLodTolerance() const;

	virtual void enableMeshLod(bool enable);
	virtual bool isMeshLodEnabled() const;

	virtual void enableWeightLod(bool enable);
	virtual bool isWeightLodEnabled() const;

	virtual void enableSkeletonLod(bool enable);
	virtual bool isSkeletonLodEnabled() const;

	virtual IProperty* getProperty() const;

	virtual void setUserData(void* data);
	virtual void* getUserData() const;

	virtual void update(double time, float elapsedTime, unsigned long flag = 0);

	virtual void setEventHandler(IEventHandler* handler);

	virtual void attach(IModel* otherModel, const Char* boneName, AttachType type = ATTACH_NORMAL, bool syncAnimation = false);
	virtual void detach(IModel* otherModel);
	virtual IModel* getAttachedTo() const;

private:
	Animation* findAnimationBySlot(const Char* slot) const;
	Animation* findAnimationBySlot(const Char* slot, LIST(Animation*)::iterator* found = NULL);

	void insertAnimationByPriority(Animation* animation);

	void updateInternal(double time, float elapsedTime, unsigned long flag);
	void updateAnimations(float elapsedTime);
	void updateSkeleton();
	void updateParts();
	void updateBoundingBox();
	void updateBoundingBoxBySkeleton();
	void updateAttachments(double time, float elapsedTime, unsigned long flag);
	void updateSkeletonError();
	
	void blendAnimation(Animation* animation);
	void blendSplineAnimation(Animation* animation);

	void addAnimationToSyncGroup(Animation* animation, int group);
	void removeAnimationFromSyncGroup(Animation* animation);
	void updateSyncAnimations(float elapsedTime);

	void setAllMeshLodTolerance(float tolerance);

	IAnimation* playDummyAnimation(const Char* slot,
									AnimationMode mode,
									int priority,
									int syncGroup,
									float fadeinTime,
									float fadeoutTime);

	IAnimation* playSelfAnimation(const Char* slot,
									AnimationMode mode,
									int priority,
									int syncGroup,
									float fadeinTime,
									float fadeoutTime);

	struct DummyAnimation
	{
		STRING		slot;
		Animation*	animation;
		int			syncGroup;
	};

	Animation* findDummyAnimationBySlot(const Char* slot) const;
	Animation* findDummyAnimationBySlot(const Char* slot, LIST(DummyAnimation)::iterator* returnIter);
	void insertDummyAnimation(const DummyAnimation& dummy);

	void buildPart(Part* part);

	void buildAnimation(Animation* animation);

	void buildSkeleton();

	void buildPartMesh(const Char* slot, Part* part);

	void animationStartCallback(const Char* slot, const VECTOR(AnimationEvent)& events);
	void animationEndCallback(const Animation* animation);
	void animationTimeCallback(const Animation* animation, float prevTime);

protected:
	virtual ~Model();

	void build();

private:
	struct SyncGroup
	{
		float				time;	//0~1
		float				duration;
		VECTOR(Animation*)	animations;
	};
	struct Attachment
	{
		IModel*		model;
		STRING		boneName;
		IBone*		bone;
		AttachType	type;
		bool		syncAnimation;
	};

private:
	const ModelResource*	m_resource;
	Skeleton*				m_skeleton;

	MAP(STRING, Part*)		m_parts;
	LIST(Animation*)		m_animations;

	AaBox					m_boundingBox;
	Matrix					m_transform;
	Matrix					m_attachedTransform;

	//temporary state for getFirstPart and getNextPart
	MAP(STRING, Part*)::const_iterator m_partIterator;

	LIST(DummyAnimation)	m_dummyAnimations;
	MAP(int, SyncGroup)		m_syncGroups;

	LIST(Attachment)		m_attachments;
	IModel*					m_attachedTo;

	IEventHandler*			m_eventHandler;

	void*					m_userData;

	float					m_lodTolerance;
	bool					m_meshLodEnabled;
	bool					m_weightLodEnabled;
	bool					m_skeletonLodEnabled;

	bool					m_globalSkinning;

	bool					m_skeletonErrorDirty;

	bool					m_useFixedBoundingBox;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
inline IResource* Model::getResource() const
{
	return const_cast<ModelResource*>(m_resource);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void Model::setTransform(const Matrix& transform)
{
	m_transform = transform;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline const Matrix& Model::getTransform() const
{
	return m_attachedTo == NULL ? m_transform : m_attachedTransform;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void Model::setFixedBoundingBox(const AaBox& box)
{
	m_useFixedBoundingBox = true;
	m_boundingBox = box;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void Model::unsetFixedBoundingBox()
{
	m_useFixedBoundingBox = false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline const AaBox& Model::getBoundingBox() const
{
	return m_boundingBox;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline ISkeleton* Model::getSkeleton()
{
	return m_skeleton;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void Model::setGlobalSkinning(bool enable)
{
	m_globalSkinning = enable;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline bool Model::isGlobalSkinning() const
{
	return m_globalSkinning;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void Model::setLodTolerance(float tolerance)
{
	if (tolerance == m_lodTolerance)
	{
		return;
	}
	m_lodTolerance = tolerance;
	if (m_meshLodEnabled)
	{
		setAllMeshLodTolerance(tolerance);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline float Model::getLodTolerance() const
{
	return m_lodTolerance;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void Model::enableMeshLod(bool enable)
{
	m_meshLodEnabled = enable;
	setAllMeshLodTolerance(enable ? m_lodTolerance : 0.0f);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline bool Model::isWeightLodEnabled() const
{
	return m_weightLodEnabled;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline bool Model::isMeshLodEnabled() const
{
	return m_meshLodEnabled;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void Model::enableSkeletonLod(bool enable)
{
	m_skeletonLodEnabled = enable;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline bool Model::isSkeletonLodEnabled() const
{
	return m_skeletonLodEnabled;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void Model::setUserData(void* data)
{
	m_userData = data;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void* Model::getUserData() const
{
	return m_userData;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void Model::setEventHandler(IEventHandler* handler)
{
	m_eventHandler = handler;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline IModel* Model::getAttachedTo() const
{
	return m_attachedTo;
}

}

#endif
