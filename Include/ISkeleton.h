#ifndef __GRP_I_SKELETON_H__
#define __GRP_I_SKELETON_H__

namespace grp
{

class ISkeleton;

///////////////////////////////////////////////////////////////////////////////////////////////////
class IBone
{
public:
	virtual const Char* getName() const = 0;

	virtual const Char* getProperty() const = 0;

	virtual void setPosition(const Vector3& position, float weight = 1.0f) = 0;
	virtual void setRotation(const Quaternion& rotation, float weight = 1.0f) = 0;
	virtual void setScale(const Vector3& scale, float weight = 1.0f) = 0;

	virtual const Vector3& getPosition() const = 0;
	virtual const Quaternion& getRotation() const = 0;
	virtual const Vector3& getScale() const = 0;

	virtual const Matrix& getAbsoluteTransform() const = 0;
	virtual void setAbsoluteTransform(const Matrix& transform) = 0;

	virtual IBone* getParent() = 0;
	virtual size_t getChildCount() = 0;
	virtual IBone* getChildById(size_t id) = 0;

	virtual void updateTree() = 0;
	virtual void updateChildren() = 0;

protected:
	virtual ~IBone(){}
};

///////////////////////////////////////////////////////////////////////////////////////////////////
class ISkeletonCallback
{
public:
	virtual ~ISkeletonCallback(){}

	//sequence: preupdate, update, postupdate, ik, postik
	virtual void onPreUpdate(ISkeleton* skeleton) {}
	virtual void onPostUpdate(ISkeleton* skeleton) {}
	virtual void onPostIk(ISkeleton* skeleton) {}
};

///////////////////////////////////////////////////////////////////////////////////////////////////
class IIkSolver
{
public:
	virtual IBone* sourceBone() = 0;

	virtual void enable(bool enable) = 0;
	virtual bool isEnabled() const = 0;

	virtual void setTarget(const Vector3& position) = 0;

protected:
	virtual ~IIkSolver(){}
};

///////////////////////////////////////////////////////////////////////////////////////////////////
struct IkBoneData
{
	IkBoneData()
		: bone(NULL)
	{
	}
	IkBoneData(IBone* bone)
		: bone(bone)
		, eulerLimit(false)
	{
	}
	IkBoneData(IBone* bone, EulerType type, const Euler& eulerMin, const Euler& eulerMax)
		: bone(bone)
		, eulerType(type)
		, eulerLimit(true)
		, eulerMin(eulerMin)
		, eulerMax(eulerMax)
	{
	}
	IBone*		bone;
	EulerType	eulerType;
	Euler		eulerMin;
	Euler		eulerMax;
	bool		eulerLimit;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
class ISkeleton
{
public:
	virtual size_t getBoneCount() const = 0;

	virtual IBone* getBoneById(size_t id) = 0;

	virtual IBone* getBoneByName(const Char* name) = 0;

	virtual IIkSolver* addIkSolver(IBone* sourceBone,
									const IkBoneData* data,
									size_t boneCount,
									float threshold,
									bool keepSourceRotation = true) = 0;

	virtual void setCallback(ISkeletonCallback* callback) = 0;

	virtual void removeCallback() = 0;

protected:
	virtual ~ISkeleton(){}
};

}

#endif
