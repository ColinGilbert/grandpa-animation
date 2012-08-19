#ifndef __GRP_PLANE_H__
#define __GRP_PLANE_H__

namespace grp
{

class Plane
{
public:
	float A, B, C, D;

public:
	Plane();
	Plane(float, float, float, float);
	Plane(const Vector3& normal, float constant);
	Plane(const Vector3& normal, const Vector3& p);
	Plane(const Vector3& p0, const Vector3& p1, const Vector3& p2);
	explicit Plane(const float*);
	Plane(const Plane&);

	Plane& operator=(const Plane&);
	Plane& operator*=(float);
	Plane& operator/=(float);

	bool operator==(const Plane&) const;
	bool operator!=(const Plane&) const;

	Plane operator+() const;
	Plane operator-() const;

	Plane operator*(float) const;
	Plane operator/(float) const;

	Plane& setAsVector4(const Vector4& v);

	friend Plane operator*(float, const Plane&);

	Vector3& getNormal();
	const Vector3& getNormal() const;
	float& getDist();

	//copy from irrlicht
	bool getIntersectionWithLine(const Vector3& lineStart, const Vector3& lineDirection, Vector3& intersection) const;
	bool getIntersectionWithPlane(const Plane& other, Vector3& lineStart, Vector3& lineDirection) const;
	bool getIntersectionWithPlanes(const Plane& o1,	const Plane& o2, Vector3& point) const;

public:
	float dot(const Vector4& v) const;
	float dotCoord(const Vector3&) const;
	float dotNormal(const Vector3&) const;
	Plane getNormalized() const;
	Plane& normalize();
};

inline Plane::Plane()
{
}

inline Plane::Plane(float a, float b, float c, float d)
{
	A = a;
	B = b;
	C = c;
	D = d;
}

inline Plane::Plane(const Vector3& normal, float constant)
{
	A = normal.X;
	B = normal.Y;
	C = normal.Z;
	D = -constant;
}

inline Plane::Plane (const Vector3& normal, const Vector3& p)
{
	A = normal.X;
	B = normal.Y;
	C = normal.Z;
	D = -normal.dot(p);
}

inline Plane::Plane(const Vector3& p0, const Vector3& p1, const Vector3& p2)
{
	Vector3 edge1 = p1 - p0;
	Vector3 edge2 = p2 - p0;
	Vector3 temp = edge1.cross(edge2).getNormalized();
	A = temp.X;
	B = temp.Y;
	C = temp.Z;
	D = -temp.dot(p0);
}

inline Plane::Plane(const float* p)
{
	A = p[0];
	B = p[1];
	C = p[2];
	D = p[3];
}

inline Plane::Plane(const Plane& p)
{
	A = p.A;
	B = p.B;
	C = p.C;
	D = p.D;
}

inline Plane& Plane::operator=(const Plane& p)
{
	A = p.A;
	B = p.B;
	C = p.C;
	D = p.D;

	return *this;
}

inline Plane& Plane::operator*=(float f)
{
	A *= f;
	B *= f;
	C *= f;
	D *= f;

	return *this;
}

inline Plane& Plane::operator/=(float f)
{
	float inv = 1.0f / f;
	A *= inv;
	B *= inv;
	C *= inv;
	D *= inv;

	return *this;
}

inline bool Plane::operator==(const Plane& p) const
{
	return A == p.A && B == p.B && C == p.C && D == p.D;
}

inline bool Plane::operator!=(const Plane& p) const
{
	return A != p.A || B != p.B || C != p.C || D != p.D;
}

inline Plane Plane::operator+() const
{
	return *this;
}

inline Plane Plane::operator-() const
{
	return Plane(-A, -B, -C, -D);
}

inline Plane Plane::operator*(float f) const
{
	return Plane(A * f, B * f, C * f, D * f);
}

inline Plane Plane::operator/(float f) const
{
	float inv = 1.0f / f;
	return Plane(A * inv, B * inv, C * inv, D * inv);
}

inline Plane& Plane::setAsVector4(const Vector4& v)
{
	A = v.X;
	B = v.Y;
	C = v.Z;
	D = v.W;
	return *this;
}

inline Plane operator*(float f, const Plane& p)
{
	return Plane(f * p.A, f * p.B, f * p.C, f * p.D);
}

inline float Plane::dot(const Vector4& v) const
{
	return A * v.X + B * v.Y + C * v.Z + D * v.W;
}

inline float Plane::dotCoord(const Vector3& v) const
{
	return A * v.X + B * v.Y + C * v.Z + D;
}

inline float Plane::dotNormal(const Vector3& v) const
{
	return A * v.X + B * v.Y + C * v.Z;
}

inline Plane Plane::getNormalized() const
{
	float l = ((Vector3*)this)->length();
	if (l != 0.0f)
	{
		l = 1.0f / l;
		return Plane(A * l, B * l, C * l, D * l);
	}
	else
	{
		return *this;
	}
}

inline Plane& Plane::normalize()
{
	float l = ((Vector3*)this)->length();
	if (l != 0.0f)
	{
		l = 1.0f / l;
		*this *= l;
	}
	return *this;
}

inline Vector3& Plane::getNormal()
{
	return *((Vector3*)this);
}

inline const Vector3& Plane::getNormal() const
{
	return *((const Vector3*)this);
}

inline float& Plane::getDist()
{
	return D;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline bool Plane::getIntersectionWithLine(const Vector3& lineStart, const Vector3& lineDirection, Vector3& intersection) const
{
	float t2 = getNormal().dot(lineDirection);

	if (t2 == 0)
	{
		return false;
	}
	float t = -(getNormal().dot(lineStart) + D) / t2;
	intersection = lineStart + (lineDirection * t);
	return true;
}

inline bool Plane::getIntersectionWithPlane(const Plane& other, Vector3& lineStart, Vector3& lineDirection) const
{
	float fn00 = getNormal().length();
	float fn01 = getNormal().dot(other.getNormal());
	float fn11 = other.getNormal().length();
	double det = fn00 * fn11 - fn01 * fn01;

	if (fabs(det) < 0.00000001)
	{
		return false;
	}
	double invdet = 1.0 / det;
	double fc0 = (fn11* - D + fn01 * other.D) * invdet;
	double fc1 = (fn00* - other.D + fn01 * D) * invdet;

	lineDirection = getNormal().cross(other.getNormal());
	lineStart = getNormal() * (float)fc0 + other.getNormal() * (float)fc1;
	return true;
}

inline bool Plane::getIntersectionWithPlanes(const Plane& o1, const Plane& o2, Vector3& point) const
{
	Vector3 lineStart, lineDirection;
	if (getIntersectionWithPlane(o1, lineStart, lineDirection))
	{
		return o2.getIntersectionWithLine(lineStart, lineDirection, point);
	}
	return false;
}

}

#endif
