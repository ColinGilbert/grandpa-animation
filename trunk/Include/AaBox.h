#ifndef __GRP_AABOX_H__
#define __GRP_AABOX_H__

#include "Vector.h"
#include "Line.h"

namespace grp
{

class AaBox
{
public:
	Vector3		MinEdge;
	Vector3		MaxEdge;

public:
	AaBox();
	AaBox(const Vector3& point);
	AaBox(const Vector3& minEdge, const Vector3& maxEdge);
	AaBox(float minX, float minY, float minZ, float maxX, float maxY, float maxZ);

	bool isEmpty() const;

	void reset(const Vector3& point);
	void reset(const Vector3& minEdge, const Vector3& maxEdge);
	void reset(float minX, float minY, float minZ, float maxX, float maxY, float maxZ);

	bool operator==(const AaBox& other) const;
	bool operator!=(const AaBox& other) const;

	void addInternalPoint(float x, float y, float z);
	void addInternalPoint(const Vector3& point);
	void addInternalBox(const AaBox& box);

	bool isPointInside(const Vector3& point) const;
	bool isPointTotalInside(const Vector3& point) const;

	bool intersectWithLine(const Line3& line) const;
	bool getIntersectPointWithLine(const Vector3& start, const Vector3& end, Vector3& intersectPoint) const;

	Vector3 getCenter() const;
	Vector3 getSize() const;
	/*
	   3-------7
	  /|      /|
	 / |     / |
	1-------5  |
	|  2----|--6
	| /     | /
	|/      |/
	0-------4 
	*/
	void getVertices(Vector3* vertices) const;

public:
	GRANDPA_API static const AaBox EMPTY;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
inline AaBox::AaBox()
	: MinEdge(0, 0, 0)
	, MaxEdge(0, 0, 0)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline AaBox::AaBox(const Vector3& point)
	: MinEdge(point)
	, MaxEdge(point)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline AaBox::AaBox(const Vector3& minEdge, const Vector3& maxEdge)
	: MinEdge(minEdge)
	, MaxEdge(maxEdge)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline AaBox::AaBox(float minX, float minY, float minZ, float maxX, float maxY, float maxZ)
	: MinEdge(minX, minY, minZ)
	, MaxEdge(maxX, maxY, maxZ)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline bool AaBox::isEmpty() const
{
	return (MinEdge == MaxEdge);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void AaBox::reset(const Vector3& point)
{
	MinEdge = MaxEdge = point;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void AaBox::reset(const Vector3& minEdge, const Vector3& maxEdge)
{
	MinEdge = minEdge;
	MaxEdge = maxEdge;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void AaBox::reset(float minX, float minY, float minZ, float maxX, float maxY, float maxZ)
{
	MinEdge.set(minX, minY, minZ);
	MaxEdge.set(maxX, maxY, maxZ);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline bool AaBox::operator==(const AaBox& other) const
{
	return (MinEdge == other.MinEdge && MaxEdge == other.MaxEdge);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline bool AaBox::operator!=(const AaBox& other) const
{
	return !(MinEdge == other.MinEdge && MaxEdge == other.MaxEdge);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void AaBox::addInternalPoint(float x, float y, float z)
{
	if (x > MaxEdge.X)	MaxEdge.X = x;
	if (y > MaxEdge.Y)	MaxEdge.Y = y;
	if (z > MaxEdge.Z)	MaxEdge.Z = z;
	if (x < MinEdge.X)	MinEdge.X = x;
	if (y < MinEdge.Y)	MinEdge.Y = y;
	if (z < MinEdge.Z)	MinEdge.Z = z;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void AaBox::addInternalPoint(const Vector3& point)
{
	addInternalPoint(point.X, point.Y, point.Z);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void AaBox::addInternalBox(const AaBox& box)
{
	addInternalPoint(box.MinEdge);
	addInternalPoint(box.MaxEdge);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline bool AaBox::isPointInside(const Vector3& point) const
{
	return (point.X >= MinEdge.X && point.X <= MaxEdge.X &&
			 point.Y >= MinEdge.Y && point.Y <= MaxEdge.Y &&
			 point.Z >= MinEdge.Z && point.Z <= MaxEdge.Z);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline bool AaBox::isPointTotalInside(const Vector3& point) const
{
	return (point.X > MinEdge.X && point.X < MaxEdge.X &&
			 point.Y > MinEdge.Y && point.Y < MaxEdge.Y &&
			 point.Z > MinEdge.Z && point.Z < MaxEdge.Z);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline bool AaBox::intersectWithLine(const Line3& line) const
{
	float halfLength = line.getLength() * 0.5f;
	const Vector3 lineVector = line.getVector().getNormalized();
	const Vector3 lineMiddle = line.getMiddle();
	const Vector3 e = getSize() * 0.5f;
	const Vector3 t = getCenter() - lineMiddle;

	if ((fabs(t.X) > e.X + halfLength * fabs(lineVector.X)) || 
		 (fabs(t.Y) > e.Y + halfLength * fabs(lineVector.Y)) ||
		 (fabs(t.Z) > e.Z + halfLength * fabs(lineVector.Z)))
	{
		return false;
	}
	float r = e.Y * fabs(lineVector.Z) + e.Z * fabs(lineVector.Y);
	if (fabs(t.Y * lineVector.Z - t.Z * lineVector.Y) > r)
	{
		return false;
	}
	r = e.X * fabs(lineVector.Z) + e.Z * fabs(lineVector.X);
	if (fabs(t.Z * lineVector.X - t.X * lineVector.Z) > r)
	{
		return false;
	}
	r = e.X * fabs(lineVector.Y) + e.Y * fabs(lineVector.X);
	if (fabs(t.X * lineVector.Y - t.Y * lineVector.X) > r)
	{
		return false;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline bool AaBox::getIntersectPointWithLine(const Vector3& start, const Vector3& end, Vector3& intersectPoint) const
{
	float minDistSq = 9999999999.0f;
	float t = 0.0f;
	bool intersect = false;

#define EDGE_INTERSECT(y, x, z, edge)	\
	t = (edge.y - start.y) / (end.y - start.y);\
	if (t >= 0.0f && t <= 1.0f)\
	{\
		intersect = true;\
		Vector3 p = start + (end - start) * t;\
		if (p.x >= MinEdge.x && p.x <= MaxEdge.x &&\
			p.z >= MinEdge.z && p.z <= MaxEdge.z)\
		{\
			float distSq = start.distanceSq(p);\
			if (distSq < minDistSq)\
			{\
				intersectPoint = p;\
				minDistSq = distSq;\
			}\
		}\
	}

	if (end.Y - start.Y != 0.0f)
	{
		EDGE_INTERSECT(Y, X, Z, MinEdge);
		EDGE_INTERSECT(Y, X, Z, MaxEdge);
	}
	if (end.X - start.X != 0.0f)
	{
		EDGE_INTERSECT(X, Y, Z, MinEdge);
		EDGE_INTERSECT(X, Y, Z, MaxEdge);
	}
	if (end.Z - start.Z != 0.0f)
	{
		EDGE_INTERSECT(Z, X, Y, MinEdge);
		EDGE_INTERSECT(Z, X, Y, MaxEdge);
	}
	return intersect;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline Vector3 AaBox::getCenter() const
{
	return (MinEdge + MaxEdge) / 2.0f;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline Vector3 AaBox::getSize() const
{
	return (MaxEdge - MinEdge);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void AaBox::getVertices(Vector3* vertices) const
{
	const Vector3 center = getCenter();
	const Vector3 diag = center - MaxEdge;

	vertices[0].set(center.X + diag.X, center.Y + diag.Y, center.Z + diag.Z);
	vertices[1].set(center.X + diag.X, center.Y - diag.Y, center.Z + diag.Z);
	vertices[2].set(center.X + diag.X, center.Y + diag.Y, center.Z - diag.Z);
	vertices[3].set(center.X + diag.X, center.Y - diag.Y, center.Z - diag.Z);
	vertices[4].set(center.X - diag.X, center.Y + diag.Y, center.Z + diag.Z);
	vertices[5].set(center.X - diag.X, center.Y - diag.Y, center.Z + diag.Z);
	vertices[6].set(center.X - diag.X, center.Y + diag.Y, center.Z - diag.Z);
	vertices[7].set(center.X - diag.X, center.Y - diag.Y, center.Z - diag.Z);
}

}

#endif
