#ifndef __GRP_B_SPLINE_H__
#define __GRP_B_SPLINE_H__

#include "ISpline.h"
#include "SplineFunctions.h"

namespace grp
{

template <typename T>
class Spline : public ISpline<T>
{
public:
	Spline();

	virtual void addKey(const T& key, float time);

	virtual void setKeys(size_t keyCount, const T* keys, float* time);

	virtual void buildKnots(bool loop = false);

	virtual void clear();

	virtual size_t keyCount() const;

	virtual float maxTime() const;

	virtual T sample(float time, bool loop = false) const;

private:
	void findKey(float time) const;

private:
	VECTOR(T)		m_keys;
	VECTOR(T)		m_knots;
	VECTOR(float)	m_times;
	mutable size_t	m_lastKey;
};

}

#endif
