#ifndef __GRP_I_SPLINE_H__
#define __GRP_I_SPLINE_H__

namespace grp
{

template<typename T>
class ISpline
{
public:
	virtual void addKey(const T& key, float time) = 0;

	virtual void setKeys(size_t keyCount, const T* keys, float* time) = 0;

	virtual void buildKnots(bool loop = false) = 0;	//loop only work when first key and last key are identical

	virtual void clear() = 0;

	virtual size_t keyCount() const = 0;

	virtual float maxTime() const = 0;

	virtual T sample(float time, bool loop = false) const = 0;

protected:
	virtual ~ISpline(){}
};

}

#endif
