#ifndef __GRP_I_MODEL_H__
#define __GRP_I_MODEL_H__

#include "IAnimation.h"

namespace grp
{

class IResource;
class IPart;
class IAnimation;
class IEventHandler;
class ISkeleton;
class IProperty;

enum AttachType
{
	ATTACH_NORMAL = 0,
	ATTACH_NO_SCALE,
	ATTACH_NO_ROTATION
	//do we need attachment without rotation but with scale?
};

const unsigned long UPDATE_INVISIBLE = 1;
const unsigned long UPDATE_NO_BOUNDING_BOX = 2;

class IPartFunctions
{
public:
	virtual IPart* setPart(const Char* slot, const IResource* resource) = 0;
	virtual IPart* setPart(const Char* slot, const Char* url, void* param = NULL) = 0;
	virtual IPart* findPart(const Char* slot) const = 0;
	virtual void removePart(const Char* slot) = 0;
	virtual void removeAllParts() = 0;
	virtual IPart* getFirstPart(const Char** slot = NULL) = 0;
	virtual IPart* getNextPart(const Char** slot = NULL) = 0;
};

class IAnimationFunctions
{
public:
	virtual IAnimation* playAnimation(const Char* slot,
										AnimationMode mode,
										int priority = 1,
										int syncGroup = -1,
										float fadeinTime = 0.3f,
										float fadeoutTime = 0.3f) = 0;
	virtual IAnimation* findAnimation(const Char* slot) const = 0;
	virtual bool stopAnimation(const Char* slot, float fadeoutTime = -1.0f) = 0;	//-1 means use fadeout time that specified when calling playAnimation
	virtual void stopAllAnimations(float fadeoutTime = 0.3f) = 0;
	virtual bool isAnimationPlaying(const Char* slot) const = 0;
	virtual bool hasAnimation(const Char* slot) const = 0;
};

class ILodFunctions
{
public:
	virtual void setLodTolerance(float tolerance) = 0;
	virtual float getLodTolerance() const = 0;

	virtual void enableMeshLod(bool enable) = 0;
	virtual bool isMeshLodEnabled() const = 0;

	virtual void enableWeightLod(bool enable) = 0;
	virtual bool isWeightLodEnabled() const = 0;

	virtual void enableSkeletonLod(bool enable) = 0;
	virtual bool isSkeletonLodEnabled() const = 0;
};

class IModel : public IPartFunctions, public IAnimationFunctions, public ILodFunctions
{
public:
	virtual IResource* getResource() const = 0;

	virtual void setTransform(const Matrix& transform) = 0;
	virtual const Matrix& getTransform() const = 0;

	virtual void setFixedBoundingBox(const AaBox& box) = 0;
	virtual void unsetFixedBoundingBox() = 0;
	virtual const AaBox& getBoundingBox() const = 0;

	virtual ISkeleton* getSkeleton() = 0;

	virtual void setGlobalSkinning(bool enable) = 0;
	virtual bool isGlobalSkinning() const = 0;

	virtual IProperty* getProperty() const = 0;

	virtual void setUserData(void* data) = 0;
	virtual void* getUserData() const = 0;

	virtual void update(double time, float elapsedTime, unsigned long flag = 0) = 0;

	virtual void setEventHandler(IEventHandler* handler) = 0;

	//otherModel will be attached to this model
	virtual void attach(IModel* otherModel, const Char* boneName, AttachType type = ATTACH_NORMAL, bool syncAnimation = false) = 0;
	virtual void detach(IModel* otherModel) = 0;
	virtual IModel* getAttachedTo() const = 0;

protected:
	virtual ~IModel(){}
};

}

#endif
