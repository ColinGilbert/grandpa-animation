#ifndef __GRP_ANIMATION_H__
#define __GRP_ANIMATION_H__

#include "IResource.h"
#include "IAnimation.h"
#include "ResourceInstance.h"
#include <vector>

namespace grp
{
class AnimationFile;
template<class T, ResourceType resType> class ContentResource;
typedef ContentResource<AnimationFile, RES_TYPE_ANIMATION> AnimationResource;

class Animation : public IAnimation, public ResourceInstance
{
public:
	Animation();
	~Animation();

	enum State
	{
		STATE_NONE = 0,
		STATE_FADING,
		STATE_STEADY,
		STATE_STOPPED
	};

	virtual float getDuration() const;

	virtual void setTime(float time);
	virtual float getTime() const;

	virtual void setTimeScale(float timeScale);
	virtual float getTimeScale() const;

	virtual void setWeight(float weight, float fadeTime = 0.2f);
	virtual float getWeight() const;

	virtual int getPriority() const;

	virtual void setPlayMode(AnimationMode mode);
	virtual AnimationMode getPlayMode() const;

	virtual void setUserData(void* data);
	virtual void* getUserData() const;

	void play(int priority, float fadeinTime, float fadeoutTime, AnimationMode mode);

	void stop(float fadeoutTime = -1.0f);

	void setClip(float startTime, float endTime);

	bool update(float elapsedTime);

	int getBoneId(unsigned long index) const;

	bool isEnding() const;

	void setAnimationResource(const AnimationResource* resource);
	const AnimationResource* getAnimationResource() const;

	void build(VECTOR(int)& boneIds);

	float getSampleTime() const;

	void setInfo(const void* info);
	const void* getInfo() const;

private:
	void updateTime(float elapsedTime);

	bool updateWeight(float elapsedTime);

private:
	const AnimationResource*	m_resource;
	VECTOR(int)		m_boneIds;

	float	m_time;
	float	m_timeScale;
	float	m_startTime;
	float	m_endTime;

	float	m_currentWeight;
	float	m_targetWeight;

	float	m_fadeoutTime;
	float	m_fadeTimeLeft;

	int				m_priority;
	State			m_state;
	AnimationMode	m_mode;

	const void*		m_info;
	void*			m_userData;

	bool			m_ending;
};


//////////////////////////////////////////////////////////////////////////////////////////////////
inline void Animation::setAnimationResource(const AnimationResource* resource)
{
	SAFE_DROP(m_resource);
	resource->grab();
	m_resource = resource;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
inline const AnimationResource* Animation::getAnimationResource() const
{
	return m_resource;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
inline void Animation::setClip(float startTime, float endTime)
{
	m_startTime = startTime;
	m_endTime = endTime;
	if (m_time < m_startTime)
	{
		m_time = m_startTime;
	}
	else if (m_time > m_endTime)
	{
		m_time = m_endTime;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////
inline int Animation::getBoneId(unsigned long index) const
{
	assert(index < m_boneIds.size());
	return m_boneIds[index];
}

//////////////////////////////////////////////////////////////////////////////////////////////////
inline void Animation::setTime(float time)
{
	m_time = time + m_startTime;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
inline float Animation::getTime() const
{
	return m_time - m_startTime;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
inline void Animation::setTimeScale(float scale)
{
	m_timeScale = scale;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
inline float Animation::getTimeScale() const
{
	return m_timeScale;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
inline void Animation::setWeight(float weight, float fadeTime)
{
	m_targetWeight = weight;
	m_fadeTimeLeft = fadeTime;
	m_state = STATE_FADING;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
inline float Animation::getWeight() const
{
	return m_currentWeight;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
inline int Animation::getPriority() const
{
	return m_priority;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
inline bool Animation::isEnding() const
{
	return m_ending;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
inline void Animation::setPlayMode(AnimationMode mode)
{
	m_mode = mode;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
inline AnimationMode Animation::getPlayMode() const
{
	return m_mode;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
inline void Animation::setUserData(void* data)
{
	m_userData = data;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void* Animation::getUserData() const
{
	return m_userData;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
inline float Animation::getSampleTime() const
{
	return m_time;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
inline void Animation::setInfo(const void* info)
{
	m_info = info;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
inline const void* Animation::getInfo() const
{
	return m_info;
}

}

#endif
