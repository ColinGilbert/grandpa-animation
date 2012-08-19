#ifndef __GRP_VECTOR_H__
#define __GRP_VECTOR_H__

namespace grp
{

///////////////////////////////////////////////////////////////////////////////////////////////////
// Vector2
///////////////////////////////////////////////////////////////////////////////////////////////////
class Vector2
{
public:
	float X, Y;

public:
	Vector2();
	Vector2(float, float);
	explicit Vector2(const float*);
	Vector2(const Vector2&);

	operator float*();
	operator const float*() const;

	Vector2& operator=(const Vector2&);
	Vector2& operator+=(const Vector2&);
	Vector2& operator-=(const Vector2&);
	Vector2& operator*=(float);
	Vector2& operator/=(float);

	bool operator==(const Vector2&) const;
	bool operator!=(const Vector2&) const;

	Vector2 operator+() const;
	Vector2 operator-() const;

	Vector2 operator+(const Vector2&) const;
	Vector2 operator-(const Vector2&) const;
	Vector2 operator*(float) const;
	Vector2 operator/(float) const;

	friend Vector2 operator*(float, const Vector2&);

public:
	void set(float, float);

	float length() const;
	float lengthSq() const;
	float distance(const Vector2&) const;
	float distanceSq(const Vector2&) const;

	float dot(const Vector2&) const;
	float crossZ(const Vector2&) const;
	Vector2 getLerp(const Vector2&, float) const;
	Vector2& lerp(const Vector2&, float);
	Vector2 reflect(const Vector2& normal) const;
	Vector2 getNormalized() const;
	Vector2& normalize();

public:
	GRANDPA_API static const Vector2 ZERO;
};

inline Vector2::Vector2()
{
}

inline Vector2::Vector2(float x, float y)
	: X(x), Y(y)
{
}

inline Vector2::Vector2(const float* p)
	: X(p[0]), Y(p[1])
{
}

inline Vector2::Vector2(const Vector2& v)
	: X(v.X), Y(v.Y)
{
}

inline Vector2& Vector2::operator=(const Vector2& v)
{
	X = v.X;
	Y = v.Y;

	return *this;
}

inline Vector2& Vector2::operator+=(const Vector2& v)
{
	X += v.X;
	Y += v.Y;

	return *this;
}

inline Vector2& Vector2::operator-=(const Vector2& v)
{
	X -= v.X;
	Y -= v.Y;

	return *this;
}

inline Vector2& Vector2::operator*=(float f)
{
	X *= f;
	Y *= f;
	return *this;
}

inline Vector2& Vector2::operator/=(float f)
{
	float inv = 1.0f / f;
	X *= inv;
	Y *= inv;

	return *this;
}

inline bool Vector2::operator==(const Vector2& v) const
{
	return X == v.X && Y == v.Y;
}

inline bool Vector2::operator!=(const Vector2& v) const
{
	return X != v.X || Y != v.Y;
}

inline Vector2 Vector2::operator+() const
{
	return *this;
}

inline Vector2 Vector2::operator-() const
{
	return Vector2(-X, -Y);
}

inline Vector2 Vector2::operator+(const Vector2& v) const
{
	return Vector2(X + v.X, Y + v.Y);
}

inline Vector2 Vector2::operator-(const Vector2& v) const
{
	return Vector2(X - v.X, Y - v.Y);
}

inline Vector2 Vector2::operator*(float f) const
{
	return Vector2(X * f, Y * f);
}

inline Vector2 Vector2::operator/(float f) const
{
	float inv = 1.0f / f;
	return Vector2(X * inv, Y * inv);
}

inline Vector2 operator*(float lhs, const Vector2& rhs)
{
	return Vector2(lhs * rhs.X, lhs * rhs.Y);
}

inline void Vector2::set(float x, float y)
{
	X = x;
	Y = y;
}

inline float Vector2::length() const
{
	return sqrtf(X * X + Y * Y);
}

inline float Vector2::lengthSq() const
{
	return X * X + Y * Y;
}

inline float Vector2::distance(const Vector2& v) const
{
	return Vector2(X - v.X, Y - v.Y).length(); 
}

inline float Vector2::distanceSq(const Vector2& v) const
{
	return Vector2(X - v.X, Y - v.Y).lengthSq();
}

inline float Vector2::dot(const Vector2& v) const
{
	return X * v.X + Y * v.Y;
}

inline float Vector2::crossZ(const Vector2& v) const
{
	return X * v.X - Y * v.Y;
}

inline Vector2 Vector2::getLerp(const Vector2& v, float l) const
{
	return Vector2(	X + l *(v.X - X),
					Y + l *(v.Y - Y));
}

inline Vector2& Vector2::lerp(const Vector2& v, float l)
{
	X += l * (v.X - X);
	Y += l * (v.Y - Y);
	return *this;
}

inline Vector2 Vector2::reflect(const Vector2& normal) const
{
	return Vector2(*this -(2 * dot(normal) * normal));
}

inline Vector2 Vector2::getNormalized() const
{
	float l = length();
	return l != 0.0f ?(*this / l) : *this;
}

inline Vector2& Vector2::normalize()
{
	float l = length();
	if (l != 0.0f)
	{
		*this /= l;
	}
	return *this;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Vector3
///////////////////////////////////////////////////////////////////////////////////////////////////
class Vector3
{
public:
	union
	{
		struct
		{
			float X, Y, Z;
		};
		struct
		{
			float yaw, pitch, roll;
		};
	};

public:
	Vector3();
	Vector3(float, float, float);
	explicit Vector3(const float*);
	Vector3(const Vector3&);

	Vector3& operator=(const Vector3&);
	Vector3& operator+=(const Vector3&);
	Vector3& operator-=(const Vector3&);
	Vector3& operator*=(float);
	Vector3& operator/=(float);

	bool operator==(const Vector3&) const;
	bool operator!=(const Vector3&) const;

	Vector3 operator+() const;
	Vector3 operator-() const;

	Vector3 operator+(const Vector3&) const;
	Vector3 operator-(const Vector3&) const;
	Vector3 operator*(float) const;
	Vector3 operator/(float) const;

	friend Vector3 operator*(float, const Vector3&);

public:
	void set(float, float, float);

	float length() const;
	float lengthSq() const;
	float distance(const Vector3&) const;
	float distanceSq(const Vector3&) const;

	float dot(const Vector3&) const;
	Vector3 cross(const Vector3&) const;
	Vector3 getLerp(const Vector3&, float) const;
	Vector3& lerp(const Vector3&, float);
	Vector3 reflect(const Vector3& normal) const;
	Vector3 getNormalized() const;
	Vector3& normalize();

public:
	GRANDPA_API static const Vector3 ZERO;
};

typedef Vector3 Euler;

inline Vector3::Vector3()
{
}

inline Vector3::Vector3(float x, float y, float z)
	: X(x), Y(y), Z(z)
{
}

inline Vector3::Vector3(const float* p)
	: X(p[0]), Y(p[1]), Z(p[2])
{
}

inline Vector3::Vector3(const Vector3& v)
	: X(v.X), Y(v.Y), Z(v.Z)
{
}

inline Vector3& Vector3::operator=(const Vector3& v)
{
	X = v.X;
	Y = v.Y;
	Z = v.Z;

	return *this;
}

inline Vector3& Vector3::operator+=(const Vector3& v)
{
	X += v.X;
	Y += v.Y;
	Z += v.Z;

	return *this;
}

inline Vector3& Vector3::operator-=(const Vector3& v)
{
	X -= v.X;
	Y -= v.Y;
	Z -= v.Z;

	return *this;
}

inline Vector3& Vector3::operator*=(float f)
{
	X *= f;
	Y *= f;
	Z *= f;

	return *this;
}

inline Vector3& Vector3::operator/=(float f)
{
	float inv = 1.0f / f;
	X *= inv;
	Y *= inv;
	Z *= inv;

	return *this;
}

inline bool Vector3::operator==(const Vector3& v) const
{
	return X == v.X && Y == v.Y && Z == v.Z;
}

inline bool Vector3::operator!=(const Vector3& v) const
{
	return X != v.X || Y != v.Y || Z != v.Z;
}

inline Vector3 Vector3::operator+() const
{
	return *this;
}

inline Vector3 Vector3::operator-() const
{
	return Vector3(-X, -Y, -Z);
}

inline Vector3 Vector3::operator+(const Vector3& v) const
{
	return Vector3(X + v.X, Y + v.Y, Z + v.Z);
}

inline Vector3 Vector3::operator-(const Vector3& v) const
{
	return Vector3(X - v.X, Y - v.Y, Z - v.Z);
}

inline Vector3 Vector3::operator*(float f) const
{
	return Vector3(X * f, Y * f, Z * f);
}

inline Vector3 Vector3::operator/(float f) const
{
	float inv = 1.0f / f;
	return Vector3(X * inv, Y * inv, Z * inv);
}

inline Vector3 operator*(float lhs, const Vector3& rhs)
{
	return Vector3(lhs * rhs.X, lhs * rhs.Y, lhs * rhs.Z);
}

inline void Vector3::set(float x, float y, float z)
{
	X = x;
	Y = y;
	Z = z;
}

inline float Vector3::length() const
{
	return sqrtf(X * X + Y * Y + Z * Z);
}

inline float Vector3::lengthSq() const
{
	return X * X + Y * Y + Z * Z;
}

inline float Vector3::distance(const Vector3& v) const
{
	return Vector3(X - v.X, Y - v.Y, Z - v.Z).length();
}

inline float Vector3::distanceSq(const Vector3& v) const
{
	return Vector3(X - v.X, Y - v.Y, Z - v.Z).lengthSq();
}

inline float Vector3::dot(const Vector3& v) const
{
	return X * v.X + Y * v.Y + Z * v.Z;
}

inline Vector3 Vector3::cross(const Vector3& v) const
{
	return Vector3(	Y * v.Z - Z * v.Y,
					Z * v.X - X * v.Z,
					X * v.Y - Y * v.X);
}

inline Vector3 Vector3::getLerp(const Vector3& v, float l) const
{
	return Vector3(	X + l * (v.X - X),
					Y + l * (v.Y - Y),
					Z + l * (v.Z - Z));
}

inline Vector3& Vector3::lerp(const Vector3& v, float l)
{
	X += l * (v.X - X);
	Y += l * (v.Y - Y);
	Z += l * (v.Z - Z);
	return *this;
}

inline Vector3 Vector3::reflect(const Vector3& normal) const
{
	return Vector3(*this -(2 * dot(normal) * normal));
}

inline Vector3 Vector3::getNormalized() const
{
	float l = length();
	return l != 0.0f ?(*this / l) : *this;
}

inline Vector3& Vector3::normalize()
{
	float l = length();
	if (l != 0.0f)
	{
		*this /= l;
	}
	return *this;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Vector4
///////////////////////////////////////////////////////////////////////////////////////////////////
class Vector4
{
public:
	float X, Y, Z, W;

public:
	Vector4();
	Vector4(float, float, float, float);
	explicit Vector4(const float*);
	Vector4(const Vector4&);

	Vector4& operator=(const Vector4&);
	Vector4& operator+=(const Vector4&);
	Vector4& operator-=(const Vector4&);
	Vector4& operator*=(float);
	Vector4& operator/=(float);

	bool operator==(const Vector4&) const;
	bool operator!=(const Vector4&) const;

	Vector4 operator+() const;
	Vector4 operator-() const;

	Vector4 operator+(const Vector4&) const;
	Vector4 operator-(const Vector4&) const;
	Vector4 operator*(float) const;
	Vector4 operator/(float) const;

	friend Vector4 operator*(float, const Vector4&);

public:
	void set(float, float, float, float);

	float length() const;
	float lengthSq() const;
	float distance(const Vector4&) const;
	float distanceSq(const Vector4&) const;

	float dot(const Vector4&) const;
	Vector4 getLerp(const Vector4&, float) const;
	Vector4& lerp(const Vector4&, float);
	Vector4 getNormalized() const;
	Vector4& normalize();

public:
	GRANDPA_API static const Vector4 ZERO;
};

inline Vector4::Vector4()
{
}

inline Vector4::Vector4(float x, float y, float z, float w)
	: X(x), Y(y), Z(z), W(w)
{
}

inline Vector4::Vector4(const float* p)
	: X(p[0]), Y(p[1]), Z(p[2]), W(p[3])
{
}

inline Vector4::Vector4(const Vector4& v)
	: X(v.X), Y(v.Y), Z(v.Z), W(v.W)
{
}

inline Vector4& Vector4::operator=(const Vector4& v)
{
	X = v.X;
	Y = v.Y;
	Z = v.Z;
	W = v.W;

	return *this;
}

inline Vector4& Vector4::operator+=(const Vector4& v)
{
	X += v.X;
	Y += v.Y;
	Z += v.Z;
	W += v.W;

	return *this;
}

inline Vector4& Vector4::operator-=(const Vector4& v)
{
	X -= v.X;
	Y -= v.Y;
	Z -= v.Z;
	W -= v.W;

	return *this;
}

inline Vector4& Vector4::operator*=(float f)
{
	X *= f;
	Y *= f;
	Z *= f;
	W *= f;

	return *this;
}

inline Vector4& Vector4::operator/=(float f)
{
	float inv = 1.0f / f;
	X *= inv;
	Y *= inv;
	Z *= inv;
	W *= inv;

	return *this;
}

inline bool Vector4::operator==(const Vector4& v) const
{
	return X == v.X && Y == v.Y && Z == v.Z && W == v.W;
}

inline bool Vector4::operator!=(const Vector4& v) const
{
	return X != v.X || Y != v.Y || Z != v.Z || W != v.W;
}

inline Vector4 Vector4::operator+() const
{
	return *this;
}

inline Vector4 Vector4::operator-() const
{
	return Vector4(-X, -Y, -Z, -W);
}

inline Vector4 Vector4::operator+(const Vector4& v) const
{
	return Vector4(X + v.X, Y + v.Y, Z + v.Z, W + v.W);
}

inline Vector4 Vector4::operator-(const Vector4& v) const
{
	return Vector4(X - v.X, Y - v.Y, Z - v.Z, W - v.W);
}

inline Vector4 Vector4::operator*(float f) const
{
	return Vector4(X * f, Y * f, Z * f, W * f);
}

inline Vector4 Vector4::operator/(float f) const
{
	float inv = 1.0f / f;
	return Vector4(X * inv, Y * inv, Z * inv, W * inv);
}

inline Vector4 operator*(float lhs, const Vector4& rhs)
{
	return Vector4(lhs * rhs.X, lhs * rhs.Y, lhs * rhs.Z, lhs * rhs.W);
}

inline void Vector4::set(float x, float y, float z, float w)
{
	X = x;
	Y = y;
	Z = z;
	W = w;
}

inline float Vector4::length() const
{
	return sqrtf(X * X + Y * Y + Z * Z + W * W);
}

inline float Vector4::lengthSq() const
{
	return X * X + Y * Y + Z * Z + W * W;
}

inline float Vector4::distance(const Vector4& v) const
{
	return Vector4(X - v.X, Y - v.Y, Z - v.Z, W - v.W).length();
}

inline float Vector4::distanceSq(const Vector4& v) const
{
	return Vector4(X - v.X, Y - v.Y, Z - v.Z, W - v.W).lengthSq();
}

inline float Vector4::dot(const Vector4& v) const
{
	return X * v.X + Y * v.Y + Z * v.Z + W * v.W;
}

inline Vector4 Vector4::getLerp(const Vector4& v, float l) const
{
	return Vector4(	X + l * (v.X - X),
					Y + l * (v.Y - Y),
					Z + l * (v.Z - Z),
					W + l * (v.W - W));
}

inline Vector4& Vector4::lerp(const Vector4& v, float l)
{
	X += l * (v.X - X);
	Y += l * (v.Y - Y);
	Z += l * (v.Z - Z);
	W += l * (v.W - W);
	return *this;
}

inline Vector4 Vector4::getNormalized() const
{
	float l = length();
	return l != 0.0f ? (*this / l) : *this;
}

inline Vector4& Vector4::normalize()
{
	float l = length();
	if (l != 0.0f)
	{
		*this /= l;
	}
	return *this;
}

}

#endif
