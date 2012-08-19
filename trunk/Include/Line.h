#ifndef __GRP_LINE_H__
#define __GRP_LINE_H__

#include "Vector.h"

namespace grp
{

class Line3
{
public:
	Vector3		Start;
	Vector3		End;

public:
	Line3();
	Line3(const Vector3& start, const Vector3& end);
	Line3(float startX, float startY, float startZ, float endX, float endY, float endZ);

	void set(const Vector3& start, const Vector3& end);
	void set(float startX, float startY, float startZ, float endX, float endY, float endZ);

	Line3 operator+(const Vector3& point) const;
	Line3& operator+=(const Vector3& point);

	Line3 operator-(const Vector3& point) const;
	Line3& operator-=(const Vector3& point);

	bool operator==(const Line3& other) const;
	bool operator!=(const Line3& other) const;

	float getLength() const;
	float getLengthSq() const;

	Vector3 getMiddle() const;
	Vector3 getVector() const;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
inline Line3::Line3()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline Line3::Line3(const Vector3& start, const Vector3& end)
	: Start(start)
	, End(end)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline Line3::Line3(float startX, float startY, float startZ, float endX, float endY, float endZ)
	: Start(startX, startY, startZ)
	, End(endX, endY, endZ)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void Line3::set(const Vector3& start, const Vector3& end)
{
	Start = start;
	End = end;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void Line3::set(float startX, float startY, float startZ, float endX, float endY, float endZ)
{
	Start.set(startX, startY, startZ);
	End.set(endX, endY, endZ);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline Line3 Line3::operator+(const Vector3& point) const
{
	return Line3(Start + point, End + point);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline Line3& Line3::operator+=(const Vector3& point)
{
	Start += point;
	End += point;
	return *this;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline Line3 Line3::operator-(const Vector3& point) const
{
	return Line3(Start - point, End - point);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline Line3& Line3::operator-=(const Vector3& point)
{
	Start -= point;
	End -= point;
	return *this;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline bool Line3::operator==(const Line3& other) const
{
	return (Start == other.Start && End == other.End);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline bool Line3::operator!=(const Line3& other) const
{
	return !(Start == other.Start && End == other.End);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline float Line3::getLength() const
{
	return Start.distance(End);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline float Line3::getLengthSq() const
{
	return Start.distanceSq(End);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline Vector3 Line3::getMiddle() const
{
	return (Start + End) / 2.0f;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline Vector3 Line3::getVector() const
{
	return End - Start;
}

}

#endif
