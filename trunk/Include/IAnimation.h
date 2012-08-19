#ifndef __GRP_I_ANIMATION_H__
#define __GRP_I_ANIMATION_H__

namespace grp
{

enum AnimationMode
{
	ANIMATION_SINGLE = 0,
	ANIMATION_LOOP,
	ANIMATION_PINGPONG
};

enum AnimationSampleType
{
	SAMPLE_STEP = 0,
	SAMPLE_LINEAR,
	SAMPLE_SPLINE
};

class IAnimation
{
public:
	virtual float getDuration() const = 0;

	virtual void setTime(float time) = 0;
	virtual float getTime() const = 0;

	virtual void setTimeScale(float timeScale) = 0;
	virtual float getTimeScale() const = 0;

	virtual void setWeight(float weight, float fadeTime = 0.2f) = 0;
	virtual float getWeight() const = 0;

	virtual int getPriority() const = 0;

	virtual void setPlayMode(AnimationMode mode) = 0;
	virtual AnimationMode getPlayMode() const = 0;

	virtual void setUserData(void* data) = 0;
	virtual void* getUserData() const = 0;

protected:
	virtual ~IAnimation(){}
};

}

#endif
