#include "Precompiled.h"
#include "IkSolver.h"
#include "Skeleton.h"
#include <cassert>
#include "Performance.h"

namespace grp
{

///////////////////////////////////////////////////////////////////////////////////////////////////
IkSolver::IkSolver(IBone* sourceBone, const IkBoneData* data, size_t boneCount,
					float threshold, bool keepSourceRotation)
	: m_sourceBone(sourceBone)
	, m_thresholdSq(threshold * threshold)
	, m_enabled(false)
	, m_keepSourceRotation(keepSourceRotation)
{
	assert(data != NULL);
	m_ikBones.resize(boneCount);
	for (size_t i = 0; i < boneCount; ++i)
	{
		m_ikBones[i] = data[i];
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void IkSolver::update()
{
	PERF_NODE_FUNC();

	if (!m_enabled || m_sourceBone == NULL)
	{
		return;
	}

	Matrix backupRotation;
	if (m_keepSourceRotation)
	{
		//assuming there's no scale
		backupRotation = m_sourceBone->getAbsoluteTransform();
		backupRotation.removeTranslation();
	}

	size_t repeat = 5;
	while (repeat-- > 0)
	{
		for (VECTOR(IkBoneData)::const_iterator iter = m_ikBones.begin();
			iter != m_ikBones.end();
			++iter)
		{
			const IkBoneData& boneData = *iter;
			Bone* bone = static_cast<Bone*>(boneData.bone);
			const Matrix& boneTransform = bone->getAbsoluteTransform();

			Vector3 dstLocal = boneTransform.invTransformVector3(m_targetPosition);
			Vector3 srcLocal = boneTransform.invTransformVector3(m_sourceBone->getAbsoluteTransform().getTranslation());

			if (dstLocal.distanceSq(srcLocal) < m_thresholdSq)
			{
				goto Out;
			}
			srcLocal.normalize();
			dstLocal.normalize();
			Vector3 axis = srcLocal.cross(dstLocal);
			float angle = asinf(axis.length());
			axis.normalize();
			Quaternion rotation(axis, angle);
			Quaternion newRotation = rotation * bone->getRotation();
			if (boneData.eulerLimit)
			{
				Euler euler = (boneData.eulerType == EULER_ZXY) ?
								newRotation.getEuler_zxy() :
								newRotation.getEuler_yxz();
				euler.yaw = std::max(euler.yaw, boneData.eulerMin.yaw);
				euler.pitch = std::max(euler.pitch, boneData.eulerMin.pitch);
				euler.roll = std::max(euler.roll, boneData.eulerMin.roll);
				euler.yaw = std::min(euler.yaw, boneData.eulerMax.yaw);
				euler.pitch = std::min(euler.pitch, boneData.eulerMax.pitch);
				euler.roll = std::min(euler.roll, boneData.eulerMax.roll);
				if (boneData.eulerType == EULER_ZXY)
				{
					newRotation.setEuler_zxy(euler);
				}
				else
				{
					newRotation.setEuler_yxz(euler);
				}
			}
			bone->setRotation(newRotation);
			bone->updateTree();
		}
	}
Out:
	if (m_keepSourceRotation)
	{
		backupRotation.setTranslation(m_sourceBone->getAbsoluteTransform().getTranslation());
		m_sourceBone->setAbsoluteTransform(backupRotation);
		m_sourceBone->updateChildren();
	}
}

}
