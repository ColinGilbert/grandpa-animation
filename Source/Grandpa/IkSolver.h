#ifndef __GRP_IK_SOLVER_H__
#define __GRP_IK_SOLVER_H__

#include "ISkeleton.h"

namespace grp
{

class IkSolver : public IIkSolver
{
public:
	IkSolver(IBone* sourceBone,
			  const IkBoneData* data,
			  size_t boneCount,
			  float threshold,
			  bool keepSourceRotation);

	virtual IBone* sourceBone();

	virtual void enable(bool enable);
	virtual bool isEnabled() const;

	virtual void setTarget(const Vector3& position);

	void update();

private:
	IBone*				m_sourceBone;
	VECTOR(IkBoneData)	m_ikBones;
	Vector3				m_targetPosition;
	float				m_thresholdSq;
	bool				m_enabled;
	bool				m_keepSourceRotation;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
inline IBone* IkSolver::sourceBone()
{
	return m_sourceBone;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void IkSolver::enable(bool enable)
{
	m_enabled = enable;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline bool IkSolver::isEnabled() const
{
	return m_enabled;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void IkSolver::setTarget(const Vector3& position)
{
	m_targetPosition = position;
}

}

#endif
