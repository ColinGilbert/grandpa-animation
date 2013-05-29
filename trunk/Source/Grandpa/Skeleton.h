#ifndef __GRP_SKELETON_H__
#define __GRP_SKELETON_H__

#include "ISkeleton.h"
#include "ResourceInstance.h"
#include "IResource.h"
#include <vector>
#include <string>

namespace grp
{

struct CoreBone;
class Skeleton;
class IkSolver;
class SkeletonFile;
template<class T, ResourceType resType> class ContentResource;
typedef ContentResource<SkeletonFile, RES_TYPE_SKELETON> SkeletonResource;

/////////////////////////////////////////////////////////////////////////////////////////////////////
class Bone : public IBone
{
	friend class Skeleton;

public:
	enum
	{
		BONE_TYPE_NO_SCALE = 0,
		BONE_TYPE_SCALE = 1
	};

public:
	Bone();
	~Bone(){}

public:
	virtual const Char* getName() const;

	virtual const Char* getProperty() const;

	virtual void setPosition(const Vector3& position, float weight = 1.0f);
	virtual void setRotation(const Quaternion& rotation, float weight = 1.0f);
	virtual void setScale(const Vector3& scale, float weight = 1.0f);

	virtual const Vector3& getPosition() const;
	virtual const Quaternion& getRotation() const;
	virtual const Vector3& getScale() const;

	virtual const Matrix& getAbsoluteTransform() const;
	virtual void setAbsoluteTransform(const Matrix& transform);

	virtual IBone* getParent();
	virtual size_t getChildCount();
	virtual IBone* getChildById(size_t id);

	virtual void updateChildren();
	virtual void updateTree();

	const CoreBone* getCoreBone() const;

	void blendTransform(float weight,
						const Vector3& position,
						const Quaternion& rotation);
	void blendTransform(float weight,
						const Vector3& position,
						const Quaternion& rotation,
						const Vector3& scale);

	bool hasWeightLeft() const;

	void update();

	float getLodError() const;

	void updateLodError(float error);

private:
	void reset();
	void resetWithScaleType();

	void lockTransform();
	
private:
	const CoreBone*		m_core;
	Skeleton*			m_skeleton;

	Vector3				m_position;
	Quaternion			m_rotation;
	Vector3				m_scale;

	Vector3				m_positionTemp;
	Quaternion			m_rotationTemp;
	Vector3				m_scaleTemp;

	Matrix				m_transform;

	float				m_totalWeight;
	float				m_totalWeightTemp;

	unsigned short		m_fileType;
	unsigned short		m_type;

	float				m_lodError;	//depends on attached skin, so it's not in core bone
};

////////////////////////////////////////////////////////////////////////////////////////////////////
class Skeleton : public ISkeleton, public ResourceInstance
{
public:
	Skeleton(const SkeletonResource* resource);
	~Skeleton();

public:
	virtual size_t getBoneCount() const;

	virtual IBone* getBoneById(size_t id);

	virtual IBone* getBoneByName(const Char* name);

	virtual void setCallback(ISkeletonCallback* callback);

	virtual void removeCallback();

	virtual IIkSolver* addIkSolver(IBone* sourceBone,
									const IkBoneData* data,
									size_t boneCount,
									float threshold,
									bool keepSourceRotation = true);

	const SkeletonResource* getSkeletonResource() const;

	const VECTOR(Bone)& getBones() const;

	void reset();
	void resetWithScaleType();

	void resetLodError();

	void lockTransform();

	int getBoneId(const STRING& name);

	Bone* getBone(int id);

	void update();

	void setTransform(const Matrix& transform);
	const Matrix& getTransform() const;

	void build();

private:
	void ikUpdate();

private:
	const SkeletonResource*		m_resource;
	VECTOR(Bone)				m_bones;
	Matrix						m_transform;
	ISkeletonCallback*			m_callback;
	LIST(IkSolver)				m_ikSolvers;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
inline Bone::Bone()
	: m_core(NULL)
	, m_skeleton(NULL)
	, m_fileType(BONE_TYPE_NO_SCALE)
	, m_type(BONE_TYPE_NO_SCALE)
	, m_totalWeight(0.0f)
	, m_totalWeightTemp(0.0f)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////
inline const CoreBone* Bone::getCoreBone() const
{
	return m_core;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
inline void Bone::blendTransform(float weight,
								   const Vector3& position,
								   const Quaternion& rotation)
{
	assert (m_totalWeight < 1.0f && weight > 0.0f);

	if (m_totalWeightTemp == 0.0f)
	{
		m_positionTemp = position;
		m_rotationTemp = rotation;
		m_scaleTemp = m_core->scale;
		m_totalWeightTemp = weight;
	}
	else
	{
		float factor = weight / (weight + m_totalWeightTemp);
		m_positionTemp.lerp(position, factor);
	#ifdef QUATERNION_SLERP
		m_rotationTemp.slerp(rotation, factor);
	#else
		m_rotationTemp.nlerp(rotation, factor);
	#endif
		if (BONE_TYPE_SCALE == m_type)
		{
			//can't do this b/c default scale can be non-unit
			//m_scaleTemp.lerp(Vector3::UNIT_SCALE, factor);
		}
		m_totalWeightTemp += weight;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
inline void Bone::blendTransform(float weight,
								   const Vector3& position,
								   const Quaternion& rotation,
								   const Vector3& scale)
{
	assert (m_totalWeight < 1.0f && weight > 0.0f);

	if (m_totalWeightTemp == 0.0f)
	{
		m_positionTemp = position;
		m_rotationTemp = rotation;
		m_scaleTemp = scale;
		m_totalWeightTemp = weight;
	}
	else
	{
		float factor = weight / (weight + m_totalWeightTemp);
		m_positionTemp.lerp(position, factor);
	#ifdef QUATERNION_SLERP
		m_rotationTemp.slerp(rotation, factor);
	#else
		m_rotationTemp.nlerp(rotation, factor);
	#endif
		m_scaleTemp.lerp(scale, factor);
		m_totalWeightTemp += weight;
	}
	m_type = BONE_TYPE_SCALE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
inline void Bone::setPosition(const Vector3& position, float weight)
{
	if (weight == 1.0f)
	{
		m_position = position;
	}
	else
	{
		m_position.lerp(position, weight);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
inline void Bone::setRotation(const Quaternion& rotation, float weight)
{
	if (weight == 1.0f)
	{
		m_rotation = rotation;
	}
	else
	{
	#ifdef QUATERNION_SLERP
		m_rotation.slerp(rotation, weight);
	#else
		m_rotation.nlerp(rotation, weight);
	#endif
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
inline void Bone::setScale(const Vector3& scale, float weight)
{
	if (weight == 1.0f)
	{
		m_scale = scale;
	}
	else
	{
		m_scale.lerp(scale, weight);
	}
	m_type = BONE_TYPE_SCALE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
inline const Vector3& Bone::getPosition() const
{
	return m_position;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
inline const Quaternion& Bone::getRotation() const
{
	return m_rotation;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
inline const Vector3& Bone::getScale() const
{
	return m_scale;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
inline const Matrix& Bone::getAbsoluteTransform() const
{
	return m_transform;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
inline void Bone::setAbsoluteTransform(const Matrix& transform)
{
	m_transform = transform;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
inline void Bone::reset()
{
	m_totalWeightTemp = 0.0f;
	m_totalWeight = 0.0f;
	//m_type = m_fileType;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
inline void Bone::resetWithScaleType()
{
	m_totalWeightTemp = 0.0f;
	m_totalWeight = 0.0f;
	m_type = m_fileType;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
inline void Bone::updateTree()
{
	update();
	updateChildren();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
inline bool Bone::hasWeightLeft() const
{
	return (m_totalWeight < 1.0f);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
inline float Bone::getLodError() const
{
	return m_lodError;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//CSkeleton
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
inline size_t Skeleton::getBoneCount() const
{
	return m_bones.size();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
inline IBone* Skeleton::getBoneById(size_t id)
{
	return &m_bones[id];
}

////////////////////////////////////////////////////////////////////////////////////////////////////
inline IBone* Skeleton::getBoneByName(const Char* name)
{
	int id = getBoneId(name);
	if (id < 0)
	{
		return NULL;
	}
	return getBone(id);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
inline void Skeleton::setCallback(ISkeletonCallback* callback)
{
	m_callback = callback;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
inline void Skeleton::removeCallback()
{
    m_callback = NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
inline const SkeletonResource* Skeleton::getSkeletonResource() const
{
	return m_resource;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
inline const VECTOR(Bone)& Skeleton::getBones() const
{
	return m_bones;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
inline Bone* Skeleton::getBone(int id)
{
	assert(id >= 0 && id < static_cast<int>(m_bones.size()));
	return &(m_bones[ id ]);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
inline const Matrix& Skeleton::getTransform() const
{
	return m_transform;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
inline void Skeleton::setTransform(const Matrix& transform)
{
	m_transform = transform;
}

}

#endif
