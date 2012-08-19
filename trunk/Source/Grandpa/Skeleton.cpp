#include "Precompiled.h"
#include "Skeleton.h"
#include "SkeletonFile.h"
#include "ContentResource.h"
#include "IkSolver.h"
#include "Performance.h"

namespace grp
{

///////////////////////////////////////////////////////////////////////////////////////////////////
const Char* Bone::getName() const
{
	return m_core->name.c_str();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
const Char* Bone::getProperty() const
{
	return m_core->property.c_str();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void Bone::lockTransform()
{
	if (m_totalWeightTemp > 1.0f - m_totalWeight)
	{
		m_totalWeightTemp = 1.0f - m_totalWeight;
	}
	if (m_totalWeightTemp <= 0.0f)
	{
		return;
	}
	if (m_totalWeight == 0.0f)
	{
		m_position = m_positionTemp;
		m_rotation = m_rotationTemp;
		if (BONE_TYPE_SCALE == m_type)
		{
			m_scale = m_scaleTemp;
		}
		m_totalWeight = m_totalWeightTemp;
	}
	else
	{
		float factor = m_totalWeightTemp / (m_totalWeightTemp + m_totalWeight);
		m_position.lerp(m_positionTemp, factor);
	#ifdef QUATERNION_SLERP
		m_rotation.slerp(m_rotationTemp, factor);
	#else
		m_rotation.nlerp(m_rotationTemp, factor);
	#endif
		if (BONE_TYPE_SCALE == m_type)
		{
			m_scale.lerp(m_scaleTemp, factor);
		}
		m_totalWeight += m_totalWeightTemp;
	}
    m_totalWeightTemp = 0.0f;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void Bone::update()
{
	Matrix transform;
	if (BONE_TYPE_SCALE == m_type)
	{
		transform.setTransform(m_position, m_rotation, m_scale);
	}
	else
	{
		transform.setTranslationRotation(m_position, m_rotation);
	}

	assert(m_core != NULL);
	int parentId = m_core->parentId;
	if (parentId < 0)
	{
		assert(m_skeleton != NULL);
		transform.multiply_optimized(m_skeleton->getTransform(), m_transform);
	}
	else
	{
		Bone *parent = m_skeleton->getBone(parentId);
		assert(parent != NULL);
		transform.multiply_optimized(parent->getAbsoluteTransform(), m_transform);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void Bone::updateChildren()
{
	assert(m_core != NULL);
	assert(m_skeleton != NULL);
	for (unsigned long i = 0; i < m_core->childrenId.size(); ++i)
	{
		Bone* child = m_skeleton->getBone(m_core->childrenId[i]);
		child->updateTree();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
IBone* Bone::getParent()
{
	assert(m_core != NULL);
	if (m_core->parentId < 0)
	{
		return NULL;
	}
	assert(m_skeleton != NULL);
	return m_skeleton->getBone(m_core->parentId);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
size_t Bone::getChildCount()
{
	assert(m_core != NULL);
	return m_core->childrenId.size();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
IBone* Bone::getChildById(size_t id)
{
	assert(m_core != NULL);
	assert(m_skeleton != NULL);
	return m_skeleton->getBone(m_core->childrenId[id]);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void Bone::updateLodError(float error)
{
	if (error > m_lodError)
	{
		m_lodError = error;
	}
	else if (error > 0.0f)
	{
		return;
	}
	Bone* parent = static_cast<Bone*>(getParent());
	if (parent != NULL)
	{
		parent->updateLodError(error + m_position.length());
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//Skeleton
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
Skeleton::Skeleton(const SkeletonResource* resource)
	: m_resource(resource)
	, m_callback(NULL)
{
	assert(resource != NULL);
	resource->grab();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
Skeleton::~Skeleton()
{
	assert(m_resource != NULL);
	m_resource->drop();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
IIkSolver* Skeleton::addIkSolver(IBone* sourceBone,
								 const IkBoneData* data,
								 size_t boneCount,
								 float threshold,
								 bool keepSourceRotation)
{
	m_ikSolvers.push_back(IkSolver(sourceBone, data, boneCount, threshold, keepSourceRotation));
	return &(m_ikSolvers.back());
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void Skeleton::reset()
{
	PERF_NODE_FUNC();

	for (VECTOR(Bone)::iterator iter = m_bones.begin();
		  iter != m_bones.end();
		  ++iter)
	{
		Bone& bone = *iter;
		bone.reset();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void Skeleton::resetLodError()
{
	for (VECTOR(Bone)::iterator iter = m_bones.begin();
		  iter != m_bones.end();
		  ++iter)
	{
		Bone& bone = *iter;
		bone.m_lodError = 0.0f;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void Skeleton::lockTransform()
{
	PERF_NODE_FUNC();

	for (VECTOR(Bone)::iterator iter = m_bones.begin();
		  iter != m_bones.end();
		  ++iter)
	{
		Bone& bone = *iter;
		bone.lockTransform();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void Skeleton::update()
{
	PERF_NODE_FUNC();

	assert(isBuilt());

	if (m_callback != NULL)
	{
		m_callback->onPreUpdate(this);
	}

	//update bones
	for (VECTOR(Bone)::iterator iter = m_bones.begin();
		  iter != m_bones.end();
		  ++iter)
	{
		(*iter).update();
	}

	if (m_callback != NULL)
	{
		m_callback->onPostUpdate(this);
	}

	ikUpdate();

	if (m_callback != NULL)
	{
		m_callback->onPostIk(this);
	}
}

void Skeleton::build()
{
	assert(m_resource != NULL);
	const VECTOR(CoreBone)& coreBones = m_resource->getCoreBones();
	m_bones.resize(coreBones.size());
	for (size_t i = 0; i < coreBones.size(); ++i)
	{
		Bone& bone = m_bones[i];
		bone.m_core = &(coreBones[i]);
		bone.m_skeleton = this;
		bone.m_position = bone.m_core->position;
		bone.m_rotation = bone.m_core->rotation;
		bone.m_scale = bone.m_core->scale;
		if (bone.m_scale != Vector3(1.0f, 1.0f, 1.0f))
		{
			bone.m_fileType = Bone::BONE_TYPE_SCALE;
		}
	}
	setBuilt();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int Skeleton::getBoneId(const STRING& name)
{
	assert(m_resource != NULL);
	return m_resource->getBoneId(name);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void Skeleton::ikUpdate()
{
	for (LIST(IkSolver)::iterator iter = m_ikSolvers.begin();
		iter != m_ikSolvers.end();
		++iter)
	{
		(*iter).update();
	}
}

}
