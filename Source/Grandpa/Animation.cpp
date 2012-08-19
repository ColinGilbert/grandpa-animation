#include "Precompiled.h"
#include "Animation.h"
#include "ContentResource.h"

namespace grp
{

///////////////////////////////////////////////////////////////////////////////////////////////////
Animation::Animation()
	: m_resource(NULL)
	, m_time(0.0f)
	, m_timeScale(1.0f)
	, m_startTime(0.0f)
	, m_endTime(0.0f)
	, m_currentWeight(0.0f)
	, m_targetWeight(0.0f)
	, m_fadeoutTime(0.0f)
	, m_fadeTimeLeft(0.0f)
	, m_priority(0)
	, m_state(STATE_NONE)
	, m_mode(ANIMATION_SINGLE)
	, m_info(NULL)
	, m_userData(NULL)
	, m_ending(false)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
Animation::~Animation()
{
	SAFE_DROP(m_resource);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
float Animation::getDuration() const
{
	return m_endTime - m_startTime;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void Animation::play(int priority, float fadeinTime, float fadeoutTime, AnimationMode mode)
{
	m_time = m_startTime;
	m_priority = priority;
	m_fadeoutTime = fadeoutTime;
	m_mode = mode;
	m_currentWeight = 0.0f;
	m_ending = false;

	setWeight(1.0f, fadeinTime);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void Animation::stop(float fadeoutTime)
{
	m_ending = true;
	if (fadeoutTime < 0.0f)
	{
		fadeoutTime = m_fadeoutTime;
	}
	setWeight(0.0f, fadeoutTime);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool Animation::update(float elapsedTime)
{
	if (m_state == STATE_STOPPED)
	{
		return true;
	}

	m_time += (m_timeScale * elapsedTime);

	if (m_resource != NULL && m_resource->getResourceState() == RES_STATE_COMPLETE)
	{
		updateTime(elapsedTime);
	}

	return updateWeight(elapsedTime);
}

//////////////////////////////////////////////////////////////////////////////////////////////////
void Animation::build(VECTOR(int)& boneIds)
{
	assert(m_resource != NULL);
	assert(m_resource->getResourceState() == RES_STATE_COMPLETE);
	if (m_startTime < 0.0f)
	{
		m_startTime = 0.0f;
	}
	if (m_endTime > m_resource->getDuration() || m_endTime == 0.0f)
	{
		m_endTime = m_resource->getDuration();
	}
	m_boneIds.swap(boneIds);
	setBuilt();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void Animation::updateTime(float elapsedTime)
{
	assert(m_resource != NULL && m_resource->getResourceState() == RES_STATE_COMPLETE);
	
	float duration = m_endTime - m_startTime;
	if (m_time > m_endTime)
	{
		if (m_mode == ANIMATION_LOOP)
		{
			m_time = m_startTime + fmod(m_time - m_startTime, duration);
		}
		else if (m_mode == ANIMATION_PINGPONG)
		{
			if ((static_cast<int>((m_time - m_startTime) / duration) % 2) != 0)
			{
				m_time = m_endTime - fmod(m_time - m_startTime, duration);
			}
			else
			{
				m_time = m_startTime + fmod(m_time - m_startTime, duration);
			}
			m_timeScale = -m_timeScale;
		}
		else	//SINGLE
		{
			if (m_state != STATE_FADING || m_targetWeight != 0.0f)
			{
				if (m_state != STATE_FADING)
				{
					elapsedTime = (m_time - m_endTime) / m_timeScale;
				}
				stop(m_fadeoutTime);
			}
			m_time = m_endTime;
		}
	}
	else if (m_time < m_startTime)
	{
		//assert(m_timeScale < 0.0f);

		if (m_mode == ANIMATION_LOOP)
		{
			m_time = m_endTime - fmod(m_startTime - m_time, duration);
		}
		else if (m_mode == ANIMATION_PINGPONG)
		{
			if ((static_cast<int>((m_startTime - m_time) / duration) % 2) != 0)
			{
				m_time = m_endTime - fmod(m_startTime - m_time, duration);
			}
			else
			{
				m_time = m_startTime + fmod(m_startTime - m_time, duration);
			}
			m_timeScale = -m_timeScale;
		}
		else
		{
			if (m_state != STATE_FADING || m_targetWeight != 0.0f)
			{
				if (m_state != STATE_FADING)
				{
					elapsedTime = m_time / m_timeScale;
				}
				stop(m_fadeoutTime);
			}
			m_time = m_startTime;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool Animation::updateWeight(float elapsedTime)
{
	//negative is allowed
	elapsedTime = fabs(elapsedTime);

	if (m_state != STATE_FADING)
	{
		return true;
	}
	if (elapsedTime >= m_fadeTimeLeft)
	{
		m_currentWeight = m_targetWeight;
		m_fadeTimeLeft = 0.0f;
		if (m_currentWeight <= 0.0f && isEnding())
		{
			m_state = STATE_STOPPED;
			return false;
		}
		m_state = STATE_STEADY;
	}
	else
	{
		assert(m_fadeTimeLeft > 0.0f);
		m_currentWeight += ((m_targetWeight - m_currentWeight)
						* elapsedTime / m_fadeTimeLeft);
		m_fadeTimeLeft -= elapsedTime;
	}
	return true;
}

}
