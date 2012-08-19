#include "Precompiled.h"
#include "Spline.h"

namespace grp
{

///////////////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
Spline<T>::Spline()
	: m_lastKey(0)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
void Spline<T>::addKey(const T& key, float time)
{
	m_keys.push_back(key);
	m_times.push_back(time);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
void Spline<T>::setKeys(size_t keyCount, const T* keys, float* time)
{
	m_keys.resize(keyCount);
	m_times.resize(keyCount);
	memcpy(&m_keys[0], keys, sizeof(T) * keyCount);
	memcpy(&m_times[0], time, sizeof(float) * keyCount);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
void Spline<T>::buildKnots(bool loop)
{
	if (m_keys.size() <= 1)
	{
		return;
	}
	m_knots.resize((m_keys.size() - 1) * 2);
	getSplineKnots(&m_keys[0], m_keys.size(), &m_knots[0], &m_times[0], loop);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
void Spline<T>::clear()
{
	m_keys.clear();
	m_knots.clear();
	m_times.clear();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
size_t Spline<T>::keyCount() const
{
	return m_keys.size();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
float Spline<T>::maxTime() const
{
	if (m_times.empty())
	{
		return 0.0f;
	}
	return m_times.back();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
T Spline<T>::sample(float time, bool loop) const
{
	if (loop)
	{
		time = fmod(time, maxTime());
	}
	findKey(time);
	float factor = (time - m_times[m_lastKey]) / (m_times[m_lastKey + 1] - m_times[m_lastKey]);
	return sampleCubicBezier(m_keys[m_lastKey],
							 m_knots[m_lastKey * 2],
							 m_knots[m_lastKey * 2 + 1],
							 m_keys[m_lastKey + 1],
							 factor);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
void Spline<T>::findKey(float time) const
{
	if (m_lastKey >= m_keys.size() || time < m_times[m_lastKey])
	{
		m_lastKey = 0;
	}
	for (;m_lastKey < m_keys.size() - 2; ++m_lastKey)
	{
		if (m_times[m_lastKey + 1] > time)
		{
			return;
		}
	}
}

template class Spline<float>;
template class Spline<Vector2>;
template class Spline<Vector3>;
template class Spline<Vector4>;
template class Spline<Quaternion>;

}
