#ifndef __GRP_TRIANGLE_H__
#define __GRP_TRIANGLE_H__

//for the record, this class is copied from irrlicht
#include "Vector.h"

namespace grp
{

class Triangle
{
public:
	Vector3 V0, V1, V2;

public:
	Triangle(){}
	Triangle(const Vector3& p0, const Vector3& p1, const Vector3& p2);

	bool operator==(const Triangle& other) const;

	Vector3 getNormal() const;

	bool isPointInside(const Vector3& p) const;

	bool getPlaneIntersectPointWithLine(const Line3& line, Vector3& intersectPoint) const;
	bool getPlaneIntersectPointWithLine(const Vector3& lineStart, const Vector3& lineDirection, 
										Vector3& intersectPoint) const;

	bool getIntersectPointWithLine(const Line3& line, Vector3& intersectPoint) const;
	bool getIntersectPointWithLine(const Vector3& line, const Vector3& lineDirection, Vector3& intersectPoint) const;

private:
	bool onSameSide(const Vector3& p1, const Vector3& p2, const Vector3& a, const Vector3& b) const;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
inline Triangle::Triangle(const Vector3& p0, const Vector3& p1, const Vector3& p2)
	: V0(p0), V1(p1), V2(p2)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////
inline Vector3 Triangle::getNormal() const
{
	return (V1 - V0).cross(V2 - V0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
inline bool Triangle::operator==(const Triangle& other) const
{
	return (other.V0 == V0 && other.V1 == V1 && other.V2 == V2);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
inline bool Triangle::isPointInside(const Vector3& p) const
{
	return (onSameSide(p, V0, V1, V2) &&
			onSameSide(p, V1, V2, V0) &&
			onSameSide(p, V2, V0, V1));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
inline bool Triangle::onSameSide(const Vector3& p1, const Vector3& p2, const Vector3& a, const Vector3& b) const
{
	Vector3 a2b = b - a;
	Vector3 c1 = a2b.cross(p1 - a);
	Vector3 c2 = a2b.cross(p2 - a);
	return (c1.dot(c2) >= 0.0f);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
inline bool Triangle::getPlaneIntersectPointWithLine(const Line3& line, Vector3& intersectPoint) const
{
	Vector3 lineDir = line.getVector().normalize();
	return getPlaneIntersectPointWithLine(line.Start, lineDir, intersectPoint);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
inline bool Triangle::getPlaneIntersectPointWithLine(const Vector3& lineStart, const Vector3& lineDirection,
													Vector3& intersectPoint) const
{
	Vector3 normal = getNormal().normalize();
	float t2 = normal.dot(lineDirection);

	if (fabs(t2) < 0.00001f)
	{
		return false;
	}
	float d = V0.dot(normal);
	float t = -(normal.dot(lineStart) - d) / t2;
	intersectPoint = lineStart + (lineDirection * t);
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
inline bool Triangle::getIntersectPointWithLine(const Line3& line, Vector3& intersectPoint) const
{
	if (getPlaneIntersectPointWithLine(line, intersectPoint))
	{
		return isPointInside(intersectPoint);
	}
	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
inline bool Triangle::getIntersectPointWithLine(const Vector3& lineStart, const Vector3& lineDirection,
												Vector3& intersectPoint) const
{
	if (getPlaneIntersectPointWithLine(lineStart, lineDirection, intersectPoint))
	{
		return isPointInside(intersectPoint);
	}
	return false;
}

}

#endif
