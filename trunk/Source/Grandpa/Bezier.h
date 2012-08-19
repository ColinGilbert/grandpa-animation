#ifndef __GRP_BEZIER_H__
#define __GRP_BEZIER_H__

namespace grp
{

//type T can be float, Vector2, Vector3, Vector4, Quaternion
//t: 0~1

template<typename T>
T sampleQuadraticBezier(const T& p0, const T& p1, const T& p2, float t)
{
	float it = 1.0f - t;
	return it * it * p0 + 2 * it * t * p1 + t * t * p2;
}

template<typename T>
inline T sampleCubicBezier(const T& p0, const T& p1, const T& p2, const T& p3, float t)
{
	float it = 1.0f - t;
	float it2 = it * it;
	float it3 = it2 * it;
	float t2 = t * t;
	float t3 = t2 * t;
	return p0 * it3 + (p1 * it2 * t + p2 * it * t2) * 3 + p3 * t3;
}

template<>
inline Quaternion sampleCubicBezier<Quaternion>(const Quaternion& p0, const Quaternion& p1, const Quaternion& p2, const Quaternion& p3, float t)
{
	float it = 1.0f - t;
	float it2 = it * it;
	float it3 = it2 * it;

	float t2 = t * t;
	float t3 = t2 * t;
	if (p0.dot(p3) < 0.0f)
	{
		t3 = -t3;
	}
	if (p0.dot(p1) < 0.0f)
	{
		t = -t;
	}
	if (p0.dot(p2) < 0.0f)
	{
		t2 = -t2;
	}

	return it3 * p0 + 3 * (it2 * t * p1 + it * t2 * p2) + t3 * p3;
}

}

#endif
