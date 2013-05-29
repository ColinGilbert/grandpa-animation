#include "Precompiled.h"
#include "Grandpa.h"
#include "Model.h"
#include "Part.h"
#include "ModelResource.h"
#include "PartResource.h"
#include "ContentResource.h"
#include "Skeleton.h"
#include "Animation.h"
#include "SkinnedMesh.h"
#include "StepSampler.h"
#include "LinearSampler.h"
#include "SplineSampler.h"
#include "IEventHandler.h"
#include "Performance.h"

namespace grp
{

///////////////////////////////////////////////////////////////////////////////////////////////////
Model::Model(const ModelResource* resource, IEventHandler* eventHandler)
	: m_resource(resource)
	, m_skeleton(NULL)
	, m_eventHandler(eventHandler)
	, m_lodTolerance(0.0f)
	, m_meshLodEnabled(false)
	, m_weightLodEnabled(false)
	, m_skeletonLodEnabled(false)
	, m_globalSkinning(false)
	, m_skeletonErrorDirty(false)
	, m_useFixedBoundingBox(false)
	, m_userData(0)
	, m_boundingBox(AaBox::EMPTY)
	, m_transform(Matrix::IDENTITY)
	, m_attachedTransform(Matrix::IDENTITY)
	, m_attachedTo(NULL)
{
	assert(resource != NULL);
	resource->grab();
	//build model if resource is ready
	updateInternal(0.0, 0.0f, 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
Model::~Model()
{
	stopAllAnimations(0.0f);
	removeAllParts();
	if (m_skeleton != NULL)
	{
		GRP_DELETE(m_skeleton);
	}
	assert(m_resource != NULL);
	SAFE_DROP(m_resource);
	if (m_attachedTo != NULL)
	{
		m_attachedTo->detach(this);
	}
	for (LIST(Attachment)::iterator iter = m_attachments.begin();
		iter != m_attachments.end();
		++iter)
	{
		static_cast<Model*>(iter->model)->m_attachedTo = NULL;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void Model::enableWeightLod(bool enable)
{
	if (enable == m_weightLodEnabled)
	{
		return;
	}
	m_weightLodEnabled = enable;
	for (MAP(STRING, Part*)::iterator iter = m_parts.begin();
		iter != m_parts.end();
		++iter)
	{
		Part* part = iter->second;
		if (part->isBuilt() && part->isMeshBuilt()	&& part->isSkinnedPart())
		{
			SkinnedMesh* skinnedMesh = static_cast<SkinnedMesh*>(iter->second->getMesh());
			assert(skinnedMesh != NULL);
			skinnedMesh->enableWeightLod(enable);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void Model::update(double time, float elapsedTime, unsigned long flag)
{
	//attachments will be updated by host
	if (m_attachedTo == NULL)
	{
		updateInternal(time, elapsedTime, flag);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void Model::updateInternal(double time, float elapsedTime, unsigned long flag)
{
	PERF_NODE_FUNC();

	if (m_resource->getResourceState() != RES_STATE_COMPLETE)
	{
		return;
	}
	if (!isBuilt())
	{
		build();
	}

	updateAnimations(elapsedTime);

	updateSyncAnimations(elapsedTime);

	if ((flag & UPDATE_INVISIBLE) == 0)
	{
		if (m_skeletonLodEnabled && m_skeletonErrorDirty)
		{
			updateSkeletonError();
		}

		updateSkeleton();

		updateParts();

		if ((flag & UPDATE_NO_BOUNDING_BOX) == 0 && !m_useFixedBoundingBox)
		{
			updateBoundingBox();
		}
	}

	updateAttachments(time, elapsedTime, flag);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void Model::attach(IModel* otherModel, const Char* boneName, AttachType type, bool syncAnimation)
{
	Attachment attachment;
	attachment.model = otherModel;
	attachment.boneName = boneName;
	attachment.bone = NULL;
	attachment.type = type;
	attachment.syncAnimation = syncAnimation;
	m_attachments.push_back(attachment);
	static_cast<Model*>(otherModel)->m_attachedTo = this;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void Model::detach(IModel* otherModel)
{
	for (LIST(Attachment)::iterator iter = m_attachments.begin();
		iter != m_attachments.end();
		++iter)
	{
		if (iter->model == otherModel)
		{
			static_cast<Model*>(iter->model)->m_attachedTo = NULL;
			m_attachments.erase(iter);
			return;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
IPart* Model::setPart(const Char* slot, const IResource* resource)
{
	removePart(slot);

	if (resource == NULL || resource->getResourceState() == RES_STATE_BROKEN)
	{
		return NULL;
	}
	const PartResource* partResource = static_cast<const PartResource*>(resource);
	Part* part = GRP_NEW Part(partResource);
	m_parts[slot] = part;

	if (m_eventHandler != NULL)
    {
        m_eventHandler->onPartSet(this, slot, part);
    }

	if (partResource->getResourceState() == RES_STATE_COMPLETE)
	{
		buildPart(part);
	}
	if (isBuilt()
		&& (m_skeleton == NULL || m_skeleton->isBuilt())
		&& part->isBuilt()
		&& partResource->getMeshResource()->getResourceState() == RES_STATE_COMPLETE)
	{
		buildPartMesh(slot, part);
	}
	return part;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
IPart* Model::setPart(const Char* slot, const Char* url, void* param)
{
	IResource* partResource = grp::grabResource(url, RES_TYPE_PART, param);
	if (partResource == NULL)
	{
		return NULL;
	}
	IPart* part = setPart(slot, partResource);
	grp::dropResource(partResource);
	return part;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
IPart* Model::findPart(const Char* slot) const
{
	MAP(STRING, Part*)::const_iterator found = m_parts.find(slot);
	if (found != m_parts.end())
	{
		return found->second;
	}
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void Model::removePart(const Char* slot)
{
	MAP(STRING, Part*)::iterator found = m_parts.find(slot);
	if (found != m_parts.end())
	{
		if (m_eventHandler != NULL)
		{
			m_eventHandler->onPartDestroy(this, slot, found->second);
		}
		GRP_DELETE(found->second);
        m_parts.erase(found);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void Model::removeAllParts()
{
	for (MAP(STRING, Part*)::iterator iter = m_parts.begin();
		iter != m_parts.end();
		++iter)
	{
		if (m_eventHandler != NULL)
		{
			m_eventHandler->onPartDestroy(this, iter->first.c_str(), iter->second);
		}
		GRP_DELETE(iter->second);
	}
	m_parts.clear();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
IPart* Model::getFirstPart(const Char** slot)
{
	m_partIterator = m_parts.begin();
	if (m_parts.empty())
	{
		if (slot != NULL)
		{
			*slot = GT("");
		}
		return NULL;
	}
	if (slot != NULL)
	{
		*slot = m_partIterator->first.c_str();
	}
	return m_partIterator->second;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
IPart* Model::getNextPart(const Char** slot)
{
	if (m_partIterator == m_parts.end() || ++m_partIterator == m_parts.end())
	{
		if (slot != NULL)
		{
			*slot = GT("");
		}
		return NULL;
	}
	if (slot != NULL)
	{
		*slot = m_partIterator->first.c_str();
	}
	return m_partIterator->second;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
IAnimation* Model::playAnimation(const Char* slot,
									AnimationMode mode,
									int priority,
									int syncGroup,
									float fadeinTime,
									float fadeoutTime)
{
	IAnimation* animation = playSelfAnimation(slot, mode, priority, syncGroup, fadeinTime, fadeoutTime);

	//play animation on attachments
	for (LIST(Attachment)::iterator iter = m_attachments.begin();
		iter != m_attachments.end();
		++iter)
	{
		assert(iter->model != NULL);
		if (iter->syncAnimation)
		{
			iter->model->playAnimation(slot, mode, priority, syncGroup, fadeinTime, fadeoutTime);
		}
	}
	return animation;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
IAnimation* Model::findAnimation(const Char* slot) const
{
	if (isBuilt())
	{
		return findAnimationBySlot(slot);
	}
	else
	{
		return findDummyAnimationBySlot(slot);
	}
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool Model::stopAnimation(const Char* slot, float fadeoutTime)
{
	Animation* animation = static_cast<Animation*>(findAnimation(slot));
	if (animation != NULL)
	{
		animation->stop(fadeoutTime);
		return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void Model::stopAllAnimations(float fadeoutTime)
{
	if (fadeoutTime > 0.0f)
	{
		for (LIST(Animation*)::iterator iter = m_animations.begin();
			iter != m_animations.end();
			++iter)
		{
			(*iter)->stop(fadeoutTime);
		}
		for (LIST(DummyAnimation)::iterator iter = m_dummyAnimations.begin();
			iter != m_dummyAnimations.end();
			++iter)
		{
			(*iter).animation->stop(fadeoutTime);
		}
	}
	else
	{
		for (LIST(Animation*)::iterator iter = m_animations.begin();
			iter != m_animations.end();
			++iter)
		{
			GRP_DELETE(*iter);
		}
		m_animations.clear();
		m_syncGroups.clear();
		for (LIST(DummyAnimation)::iterator iter = m_dummyAnimations.begin();
			iter != m_dummyAnimations.end();
			++iter)
		{
			GRP_DELETE((*iter).animation);
		}
		m_dummyAnimations.clear();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool Model::isAnimationPlaying(const Char* slot) const
{
	const Animation* animation = static_cast<Animation*>(findAnimation(slot));
	return (animation != NULL && !animation->isEnding());
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool Model::hasAnimation(const Char* slot) const
{
	if (!isBuilt())
	{
		return false;
	}
	assert(m_resource != NULL);
	return m_resource->hasAnimation(slot);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
const Char* Model::getFirstAnimationSlot() const
{
    if(!isBuilt())
    {
        return NULL;
    }
    assert(m_resource);
    const MAP(STRING, AnimationInfo)& animationInfo = m_resource->getAnimationInfoMap();
    MAP(STRING, AnimationInfo)::const_iterator it = animationInfo.begin();
    if( it == animationInfo.end())
    {
        return NULL;
    }
    return it->first.c_str();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
const Char* Model::getNextAnimationSlot( const Char* slot ) const
{
    if(!isBuilt())
    {
        return NULL;
    }
    if( !slot )
    {
        return NULL;
    }
    assert(m_resource);
    const MAP(STRING, AnimationInfo)& animationInfo = m_resource->getAnimationInfoMap();

    MAP(STRING, AnimationInfo)::const_iterator it = animationInfo.find( slot );
    ++it;
    if( it == animationInfo.end())
    {
        return NULL;
    }
    return it->first.c_str();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
IProperty* Model::getProperty() const
{
	if (m_resource == NULL)
	{
		return NULL;
	}
	return m_resource->getProperty();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void Model::build()
{
	PERF_NODE_FUNC();

	setBuilt();
	if (m_eventHandler != NULL)
    {
        m_eventHandler->onModelBuilt(this);
    }

	assert(m_resource != NULL && m_resource->getResourceState() == RES_STATE_COMPLETE);
	const SkeletonResource* skeletonRes = m_resource->getSkeletonResource();
	if (skeletonRes != NULL && skeletonRes->getResourceState() != RES_STATE_BROKEN)
	{
		m_skeleton = GRP_NEW Skeleton(skeletonRes);
		if (skeletonRes->getResourceState() == RES_STATE_COMPLETE)
		{
			buildSkeleton();
		}
	}
	const VECTOR(PartInfo)& parts = m_resource->getPartInfoVector();
	for (VECTOR(PartInfo)::const_iterator iter = parts.begin();
		iter != parts.end();
		++iter)
	{
		const PartInfo& partInfo = *iter;
		if (findPart(partInfo.slot.c_str()) == NULL)	//slot may have been taken
		{
			setPart(partInfo.slot.c_str(), partInfo.resource);
		}
	}
	//make dummy animations real
	for (LIST(DummyAnimation)::iterator iter = m_dummyAnimations.begin();
		iter != m_dummyAnimations.end();
		++iter)
	{
		DummyAnimation& dummy = *iter;
		const AnimationInfo* animationInfo = m_resource->getAnimationInfo(dummy.slot);
		if (animationInfo == NULL ||
			animationInfo->resource == NULL ||
			animationInfo->resource->getResourceState() == RES_STATE_BROKEN)
		{
			GRP_DELETE(dummy.animation);
			continue;
		}
		dummy.animation->setInfo(animationInfo);
		dummy.animation->setAnimationResource(animationInfo->resource);
		dummy.animation->setClip(animationInfo->startTime, animationInfo->endTime);
		insertAnimationByPriority(dummy.animation);
		if (dummy.syncGroup >= 0)
		{
			addAnimationToSyncGroup(dummy.animation, dummy.syncGroup);
		}
	}
	m_dummyAnimations.clear();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
Animation* Model::findAnimationBySlot(const Char* slot) const
{
	for (LIST(Animation*)::const_iterator iter = m_animations.begin();
		iter != m_animations.end();
		++iter)
	{
		const AnimationInfo* animationInfo = static_cast<const AnimationInfo*>((*iter)->getInfo());
		assert(animationInfo != NULL);
		if (Strcmp(animationInfo->slot.c_str(), slot) == 0)
		{
			return *iter;
		}
	}
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
Animation* Model::findAnimationBySlot(const Char* slot, LIST(Animation*)::iterator* found)
{
	for (LIST(Animation*)::iterator iter = m_animations.begin();
		iter != m_animations.end();
		++iter)
	{
		const AnimationInfo* animationInfo = static_cast<const AnimationInfo*>((*iter)->getInfo());
		assert(animationInfo != NULL);
		if (Strcmp(animationInfo->slot.c_str(), slot) == 0)
		{
			if (found != NULL)
			{
				*found = iter;
			}
			return *iter;
		}
	}
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void Model::insertAnimationByPriority(Animation* animation)
{
	LIST(Animation*)::iterator iter;
	for (iter = m_animations.begin(); iter != m_animations.end(); ++iter)
	{
		if ((*iter)->getPriority() <= animation->getPriority())
		{
			break;
		}
	}
	m_animations.insert(iter, animation);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void Model::updateAnimations(float elapsedTime)
{
	PERF_NODE_FUNC();

	if (m_skeleton == NULL)
	{
		return;
	}
	for (LIST(Animation*)::iterator iter = m_animations.begin();
		iter != m_animations.end();)
	{
		Animation* animation = *iter;
		assert(animation->getAnimationResource() != NULL);
		if (!animation->isBuilt()
			&& animation->getAnimationResource()->getResourceState() == RES_STATE_COMPLETE
			&& m_skeleton->isBuilt())
		{
			buildAnimation(animation);
		}
		
		float prevTime;
		bool prevEnding;
		if (m_eventHandler != NULL)
		{
			prevTime = animation->getTime();
			prevEnding = animation->isEnding();
		}
		
		bool deleteMe = !animation->update(elapsedTime);

		if (m_eventHandler != NULL)
		{
			animationTimeCallback(animation, prevTime);
			if (!prevEnding && animation->isEnding())
			{
				animationEndCallback(animation);
			}
		}
		if (deleteMe)
		{
			iter = m_animations.erase(iter);
			removeAnimationFromSyncGroup(animation);
			GRP_DELETE(animation);
		}
		else
		{
			++iter;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void Model::updateSkeleton()
{
	PERF_NODE_FUNC();

	if (m_skeleton == NULL)
	{
		return;
	}
	assert(m_skeleton->getSkeletonResource() != NULL);
	if (m_skeleton->getSkeletonResource()->getResourceState() != RES_STATE_COMPLETE)
	{
		return;
	}
	if (!m_skeleton->isBuilt())
	{
		buildSkeleton();
	}
	if (m_globalSkinning)
	{
		m_skeleton->setTransform(getTransform());
	}
	else
	{
		m_skeleton->setTransform(Matrix::IDENTITY);
	}

	if (!m_animations.empty())
	{
		m_skeleton->reset();

		int lastPriority = m_animations.front()->getPriority();

		for (LIST(Animation*)::iterator iter = m_animations.begin();
			iter != m_animations.end();
			++iter)
		{
			Animation* animation = *iter;
			if (!animation->isBuilt() || animation->getWeight() <= 0.0f)
			{
				continue;
			}
			assert(animation->getPriority() <= lastPriority);
			if (animation->getPriority() < lastPriority)
			{
				//lock transform when priority change
				lastPriority = animation->getPriority();
				m_skeleton->lockTransform();
			}
			if (animation->getAnimationResource()->getSampleType() == SAMPLE_SPLINE)
			{
				blendSplineAnimation(animation);
			}
			else
			{
				blendAnimation(animation);
			}
		}
		m_skeleton->lockTransform();
	}

	m_skeleton->update();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void Model::updateParts()
{
	PERF_NODE_FUNC();

	for (MAP(STRING, Part*)::iterator iter = m_parts.begin();
		iter != m_parts.end();
		++iter)
	{
		Part* part= iter->second;
		const PartResource* partResource = part->getPartResource();
		assert(partResource != NULL);
		if (partResource->getResourceState() != RES_STATE_COMPLETE)
		{
			continue;
		}
		if (!part->isBuilt())
		{
			buildPart(part);
		}
		if (!part->isMeshBuilt()
			&& isBuilt()
			&& (m_skeleton == NULL || m_skeleton->isBuilt())
			&& partResource->getMeshResource()->getResourceState() == RES_STATE_COMPLETE)
		{
			buildPartMesh(iter->first.c_str(), part);
		}
		part->update(this, m_eventHandler);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void Model::updateBoundingBox()
{
	PERF_NODE_FUNC();

	bool first = true;
	for (MAP(STRING, Part*)::iterator iter = m_parts.begin();
		iter != m_parts.end();
		++iter)
	{
		Part* part = iter->second;
		Mesh* mesh = static_cast<Mesh*>(part->getMesh());
		if (mesh == NULL || !mesh->isBuilt())
		{
			continue;
		}
		if (mesh->getSkin() != NULL && !mesh->getSkin()->isGpuSkinning())
		{
			mesh->calculateBoundingBox();
		}
		if (first)
		{
			m_boundingBox = mesh->getBoundingBox();
			first = false;
		}
		else
		{
			m_boundingBox.addInternalBox(mesh->getBoundingBox());
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void Model::updateBoundingBoxBySkeleton()
{
	if (m_skeleton == NULL)
	{
		m_boundingBox.MaxEdge = m_boundingBox.MaxEdge = Vector3::ZERO;
		return;
	}
	bool first = true;
	size_t boneCount = m_skeleton->getBoneCount();
	for (size_t i = 0; i < boneCount; ++i)
	{
		IBone* bone = m_skeleton->getBoneById(i);
		if (bone != NULL)
		{
			const Vector3& translation = bone->getAbsoluteTransform().getTranslation();
			if (first)
			{
				first = false;
				m_boundingBox.reset(translation);
			}
			else
			{
				m_boundingBox.addInternalPoint(translation);
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void Model::updateAttachments(double time, float elapsedTime, unsigned long flag)
{
	if (m_skeleton == NULL)
	{
		return;
	}
	for (LIST(Attachment)::iterator iter = m_attachments.begin();
		iter != m_attachments.end();
		++iter)
	{
		IBone* &attachedBone = iter->bone;
		if (attachedBone == NULL)
		{
			attachedBone = m_skeleton->getBoneByName(iter->boneName.c_str());
		}
		if (attachedBone == NULL)
		{
			continue;
		}
		Model* attachedModel = static_cast<Model*>(iter->model);
		assert(attachedModel != NULL);
		assert(attachedModel->m_attachedTo == this);

		Matrix boneTransform = attachedBone->getAbsoluteTransform();
		if (!m_globalSkinning)
		{
			boneTransform = boneTransform * m_transform;
		}
		if (iter->type == ATTACH_NO_SCALE)
		{
			boneTransform.removeScale();
		}
		else if (iter->type == ATTACH_NO_ROTATION)
		{
			boneTransform.removeRotation();
		}
		attachedModel->m_attachedTransform = attachedModel->m_transform * boneTransform;
		attachedModel->updateInternal(time, elapsedTime, flag);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void Model::updateSkeletonError()
{
	PERF_NODE_FUNC();

	if (m_skeleton == NULL)
	{
		m_skeletonErrorDirty = false;
		return;
	}
	assert(m_skeleton->isBuilt());
	m_skeleton->resetLodError();

	for (MAP(STRING, Part*)::iterator iter = m_parts.begin();
		iter != m_parts.end();
		++iter)
	{
		Part* part= iter->second;
		if (!part->isSkinnedPart())
		{
			continue;
		}
		const SkinnedMesh* skinnedMesh = static_cast<const SkinnedMesh*>(part->getMesh());
		if (!skinnedMesh->isBuilt())
		{
			return;
		}
		const SkinnedMeshResource* meshResource = static_cast<const SkinnedMeshResource*>(skinnedMesh->getMeshResource());
		assert(meshResource != NULL);

		const VECTOR(float)& distances = meshResource->getBoneMaxDistances();
		const VECTOR(int)& boneIds = skinnedMesh->getBoneIds();
		assert(distances.size() == boneIds.size());
		//do it backward makes a little faster
		for (int i = (int)distances.size() - 1; i >= 0; --i)
		{
			Bone* bone = static_cast<Bone*>(m_skeleton->getBoneById(boneIds[i]));
			bone->updateLodError(distances[i]);
		}
	}
	m_skeletonErrorDirty = false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void Model::blendAnimation(Animation* animation)
{
	PERF_NODE_FUNC();

	assert(m_skeleton != NULL);
	assert(animation != NULL);
	assert(animation->isBuilt());

	float sampleTime = animation->getSampleTime();
	if (sampleTime != sampleTime)
	{	//invalid float
		return;
	}
	const AnimationResource* animationResource = animation->getAnimationResource();
	float fps = animationResource->getFps();
	const VECTOR(BoneTrack)& boneTracks = animationResource->getBoneTracks();
	for (size_t i = 0; i < boneTracks.size(); ++i)
	{
		int boneId = animation->getBoneId(i);
		Bone* bone = (boneId < 0) ? NULL : m_skeleton->getBone(boneId);
		if (bone == NULL || !bone->hasWeightLeft())	//all weight has been taken, no need to blend any more
		{
			continue;
		}
		if (m_skeletonLodEnabled && m_lodTolerance > bone->getLodError())
		{
			continue;
		}
		const BoneTrack& track = boneTracks[i];
		if (track.positionKeys.empty() || track.rotationKeys.empty())
		{
			continue;
		}
		Vector3 position;
		Quaternion rotation;
		if (animationResource->getSampleType() == SAMPLE_STEP)
		{
			StepSampler::sample(track.positionKeys, sampleTime, fps, position);
			StepSampler::sample(track.rotationKeys, sampleTime, fps, rotation);
		}
		else
		{
			LinearSampler::sample(track.positionKeys, sampleTime, fps, position);
			LinearSampler::sample(track.rotationKeys, sampleTime, fps, rotation);
		}
		if (track.scaleKeys.empty())
		{
			bone->blendTransform(animation->getWeight(), position, rotation);
		}
		else
		{
			Vector3 scale;
			if (animationResource->getSampleType() == SAMPLE_STEP)
			{
				StepSampler::sample(track.scaleKeys, sampleTime, fps, scale);
			}
			else
			{
				LinearSampler::sample(track.scaleKeys, sampleTime, fps, scale);
			}
			bone->blendTransform(animation->getWeight(), position, rotation, scale); 
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void Model::blendSplineAnimation(Animation* animation)
{
	PERF_NODE_FUNC();

	assert(m_skeleton != NULL);
	assert(animation != NULL);
	assert(animation->isBuilt());

	float sampleTime = animation->getSampleTime();
	if (sampleTime != sampleTime)
	{	//invalid float
		return;
	}
	const VECTOR(BoneTrack)& boneTracks = animation->getAnimationResource()->getBoneTracks();
	for (size_t i = 0; i < boneTracks.size(); ++i)
	{
		const BoneTrack& track = boneTracks[i];
		int boneId = animation->getBoneId(i);
		Bone* bone = (boneId < 0) ? NULL : m_skeleton->getBone(boneId);
		if (bone == NULL || !bone->hasWeightLeft())	//all weight has been taken, no need to blend any more
		{
			continue;
		}
		if (m_skeletonLodEnabled && m_lodTolerance > bone->getLodError())
		{
			continue;
		}
		if (track.positionKeys.empty() || track.rotationKeys.empty())
		{
			continue;
		}
		Vector3 position;
		Quaternion rotation;
		SplineSampler::sample(track.positionKeys, track.positionKnots, sampleTime, position);
		SplineSampler::sample(track.rotationKeys, track.rotationKnots, sampleTime, rotation);
		rotation.normalize();
		if (track.scaleKeys.empty())
		{
			bone->blendTransform(animation->getWeight(), position, rotation);
		}
		else
		{
			Vector3 scale;
			SplineSampler::sample(track.scaleKeys, track.scaleKnots, sampleTime, scale);
			bone->blendTransform(animation->getWeight(), position, rotation, scale); 
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void Model::addAnimationToSyncGroup(Animation* animation, int group)
{
	assert(group >= 0);
	MAP(int, SyncGroup)::iterator found = m_syncGroups.find(group);
	if (found == m_syncGroups.end())
	{
		SyncGroup& animationGroup = m_syncGroups[group];
		animationGroup.time = 0.0f;
		if (animation->isBuilt())
		{
			animationGroup.duration = animation->getDuration() / animation->getTimeScale();
		}
		else
		{
			animationGroup.duration = 1.0f;
		}
		animationGroup.animations.push_back(animation);
	}
	else
	{
		found->second.animations.push_back(animation);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void Model::removeAnimationFromSyncGroup(Animation* animation)
{
	for (MAP(int, SyncGroup)::iterator groupIter = m_syncGroups.begin();
		groupIter != m_syncGroups.end();
		++groupIter)
	{
		VECTOR(Animation*)& animations = groupIter->second.animations;
		for (VECTOR(Animation*)::iterator animationIter = animations.begin();
			animationIter != animations.end();
			++animationIter)
		{
			if (*animationIter == animation)
			{
				animations.erase(animationIter);
				if (animations.empty())
				{
					//m_syncGroups.erase(groupIter);
					groupIter->second.time = 0;
				}
				return;
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void Model::updateSyncAnimations(float elapsedTime)
{
	PERF_NODE_FUNC();

	for (MAP(int, SyncGroup)::iterator groupIter = m_syncGroups.begin();
		groupIter != m_syncGroups.end();
		++groupIter)
	{
		SyncGroup& group = groupIter->second;
		//if (group.animations.size() <= 1)
		//{
		//	continue;
		//}
		group.time += (elapsedTime / group.duration);
		if (group.time > 1.0f)
		{
			group.time = fmod(group.time, 1.0f);
		}
		float totalWeight = 0.0f;
		float totalDuration = 0.0f;
		for (VECTOR(Animation*)::iterator animationIter = group.animations.begin();
			animationIter != group.animations.end();
			++animationIter)
		{
			Animation* animation = *animationIter;
			if (animation->isBuilt())
			{
				animation->setTime(animation->getDuration() * group.time);
				totalWeight += animation->getWeight();
				totalDuration += (animation->getDuration() * animation->getWeight() / animation->getTimeScale());
			}
		}
		if (totalWeight > 0.0f)
		{
			group.duration = totalDuration / totalWeight;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void Model::setAllMeshLodTolerance(float tolerance)
{
	for (MAP(STRING, Part*)::iterator iter = m_parts.begin();
		iter != m_parts.end();
		++iter)
	{
		Part* part = iter->second;
		if (part->isBuilt() && static_cast<Mesh*>(part->getMesh())->isBuilt())
		{
			static_cast<Mesh*>(part->getMesh())->setLodTolerance(tolerance, m_eventHandler);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
IAnimation* Model::playDummyAnimation(const Char* slot,
										AnimationMode mode,
										int priority,
										int syncGroup,
										float fadeinTime,
										float fadeoutTime)
{
	LIST(DummyAnimation)::iterator iterOld;
	Animation* animation = findDummyAnimationBySlot(slot, &iterOld);
	if (animation != NULL)
	{
		animation->stop();
	}
	animation = GRP_NEW Animation;
	animation->play(priority, fadeinTime, fadeoutTime, mode);
	DummyAnimation dummy;
	dummy.slot = slot;
	dummy.animation = animation;
	dummy.syncGroup = syncGroup;
	insertDummyAnimation(dummy);
	return animation;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
IAnimation* Model::playSelfAnimation(const Char* slot,
									AnimationMode mode,
									int priority,
									int syncGroup,
									float fadeinTime,
									float fadeoutTime)
{
	if (!isBuilt())
	{
		return playDummyAnimation(slot, mode, priority, syncGroup, fadeinTime, fadeoutTime);
	}
	if (m_skeleton == NULL)
	{
		return NULL;
	}
	assert(m_resource != NULL && m_resource->getResourceState() == RES_STATE_COMPLETE);
	const AnimationInfo* animationInfo = m_resource->getAnimationInfo(slot);
	if (animationInfo == NULL)
	{
		return NULL;
	}
	const AnimationResource* animationResource = animationInfo->resource;
	assert(animationResource != NULL);
	if (animationResource->getResourceState() == RES_STATE_BROKEN)
	{
		return NULL;
	}
	LIST(Animation*)::iterator animationIter;
	Animation* animation = findAnimationBySlot(slot, &animationIter);
	if (animation != NULL)
	{
		animation->stop();
	}
	
	animation = GRP_NEW Animation;
	animation->setInfo(animationInfo);
	animation->setAnimationResource(animationResource);
	animation->setClip(animationInfo->startTime, animationInfo->endTime);
	if (animationResource->getResourceState() == RES_STATE_COMPLETE
		&& m_skeleton->isBuilt())
	{
		buildAnimation(animation);
	}
	animation->play(priority, fadeinTime, fadeoutTime, mode);
	insertAnimationByPriority(animation);
	if (syncGroup >= 0)
	{
		addAnimationToSyncGroup(animation, syncGroup);
	}
	//animation start callback
	if (m_eventHandler != NULL)
	{
		animationStartCallback(slot, animationInfo->events);
	}
	return animation;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
Animation* Model::findDummyAnimationBySlot(const Char* slot, LIST(DummyAnimation)::iterator* returnIter)
{
	for (LIST(DummyAnimation)::iterator iter = m_dummyAnimations.begin();
		iter != m_dummyAnimations.end();
		++iter)
	{
		if (Strcmp(iter->slot.c_str(), slot) == 0)
		{
			if (returnIter != NULL)
			{
				*returnIter = iter;
			}
			return iter->animation;
		}
	}
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
Animation* Model::findDummyAnimationBySlot(const Char* slot) const
{
	for (LIST(DummyAnimation)::const_iterator iter = m_dummyAnimations.begin();
		iter != m_dummyAnimations.end();
		++iter)
	{
		if (Strcmp(iter->slot.c_str(), slot) == 0)
		{
			return iter->animation;
		}
	}
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void Model::insertDummyAnimation(const DummyAnimation& dummy)
{
	LIST(DummyAnimation)::iterator iter;
	for (iter = m_dummyAnimations.begin(); iter != m_dummyAnimations.end(); ++iter)
	{
		if (iter->animation->getPriority() <= dummy.animation->getPriority())
		{
			break;
		}
	}
	m_dummyAnimations.insert(iter, dummy);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void Model::buildPart(Part* part)
{
	assert(part != NULL);
	part->build(this, m_eventHandler);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void Model::buildAnimation(Animation* animation)
{
	PERF_NODE_FUNC();

	const AnimationResource* animationResource = animation->getAnimationResource();
	assert(animationResource != NULL);
	const VECTOR(BoneTrack)& boneTracks = animationResource->getBoneTracks();
	VECTOR(int) boneIds(boneTracks.size());
	const SkeletonResource* skeletonResource = m_skeleton->getSkeletonResource();
	assert(skeletonResource != NULL);
	for (size_t i = 0; i < boneTracks.size(); ++i)
	{
		boneIds[i] = skeletonResource->getBoneId(boneTracks[i].boneName);
	}
	animation->build(boneIds);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void Model::buildSkeleton()
{
	assert(m_skeleton != NULL);
	m_skeleton->build();
	if (m_eventHandler != NULL)
	{
		m_eventHandler->onSkeletonBuilt(this, m_skeleton);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void Model::buildPartMesh(const Char* slot, Part* part)
{
	PERF_NODE_FUNC();

	assert(isBuilt());
	assert(part != NULL);
	part->buildMesh(m_meshLodEnabled ? m_lodTolerance : 0.0f, m_eventHandler);

	if (m_skeleton != NULL	)
	{
		assert(m_skeleton->isBuilt());
		assert(part->isBuilt());
		
		const PartResource* partResource = part->getPartResource();
		assert(partResource != NULL);
		if (part->isSkinnedPart())
		{
			const SkinnedMeshResource* meshResource = static_cast<const SkinnedMeshResource*>(partResource->getMeshResource());
			assert(meshResource != NULL);
			const VECTOR(STRING)& boneNames = meshResource->getBoneNames();
			SkinnedMesh* skinnedMesh = static_cast<SkinnedMesh*>(part->getMesh());
			if (skinnedMesh == NULL)
			{
				return;
			}
			for (size_t i = 0; i < boneNames.size(); ++i)
			{
				int boneId = m_skeleton->getSkeletonResource()->getBoneId(boneNames[i]);
				if (boneId >= 0)
				{
					const Bone* bone = m_skeleton->getBone(boneId);
					if (bone != NULL)
					{
						skinnedMesh->setBoneMatrix(i, boneId, &(bone->getAbsoluteTransform()));
					}
				}
			}
			skinnedMesh->enableWeightLod(m_weightLodEnabled);
		}
		else
		{
			const RigidMeshResource* meshResource = static_cast<const RigidMeshResource*>(partResource->getMeshResource());
			assert(meshResource != NULL);
			RigidMesh* rigidMesh = static_cast<RigidMesh*>(part->getMesh());
			if (rigidMesh == NULL)
			{
				return;
			}
			if (!meshResource->getAttachedBoneName().empty())
			{
				int boneId = m_skeleton->getSkeletonResource()->getBoneId(meshResource->getAttachedBoneName());
				if (boneId >= 0)
				{
					const Bone* bone = m_skeleton->getBone(boneId);
					if (bone != NULL)
					{
						rigidMesh->setAttachedBoneMatrix(bone->getAbsoluteTransform());
					}
				}
			}
		}
	}
	m_skeletonErrorDirty = true;

	if (m_eventHandler != NULL)
	{
		m_eventHandler->onPartBuilt(this, slot, part);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void Model::animationStartCallback(const Char* slot, const VECTOR(AnimationEvent)& events)
{
	for (size_t i = 0; i < events.size(); ++i)
	{
		if (events[i].type == ANIMATION_EVENT_START)
		{
			assert(m_eventHandler != NULL);
			m_eventHandler->onAnimationStart(this, slot, events[i].name.c_str(),
												const_cast<Property*>(&events[i].property));
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void Model::animationEndCallback(const Animation* animation)
{
	const AnimationInfo* animationInfo = static_cast<const AnimationInfo*>(animation->getInfo());
	if (animationInfo == NULL)
	{
		return;
	}
	const VECTOR(AnimationEvent)& events = animationInfo->events;
	for (size_t i = 0; i < events.size(); ++i)
	{
		if (events[i].type == ANIMATION_EVENT_END)
		{
			assert(m_eventHandler != NULL);
			m_eventHandler->onAnimationEnd(this, animationInfo->slot.c_str(), events[i].name.c_str(),
											const_cast<Property*>(&events[i].property));
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void Model::animationTimeCallback(const Animation* animation, float prevTime)
{
	const AnimationInfo* animationInfo = static_cast<const AnimationInfo*>(animation->getInfo());
	if (animationInfo == NULL)
	{
		return;
	}
	if (animation->getTimeScale() <= 0.0f)
	{
		return;
	}
	if (animation->getPlayMode() != ANIMATION_SINGLE && animation->getPlayMode() != ANIMATION_LOOP)
	{
		return;
	}
	const VECTOR(AnimationEvent)& events = animationInfo->events;
	for (size_t i = 0; i < events.size(); ++i)
	{
		const AnimationEvent& event = events[i];
		if (event.type != ANIMATION_EVENT_TIME)
		{
			continue;
		}
		if ((event.time > prevTime && event.time <= animation->getTime())
			|| (animation->getPlayMode() == ANIMATION_LOOP && prevTime > animation->getTime()
				&& (event.time > prevTime || event.time <= animation->getTime())))
		{
			assert(m_eventHandler != NULL);
			const AnimationInfo* animationInfo = static_cast<const AnimationInfo*>(animation->getInfo());
			assert(animationInfo != NULL);
			m_eventHandler->onAnimationTime(this, animationInfo->slot.c_str(), events[i].name.c_str(),
											const_cast<Property*>(&events[i].property));
		}
	}
}

}
