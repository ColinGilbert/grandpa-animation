#ifndef __GRP_QUATERNION_H__
#define __GRP_QUATERNION_H__

namespace grp
{

class Matrix;

//only support this two types...yet
enum EulerType
{
	EULER_YXZ = 0,
	EULER_ZXY
};

class Quaternion
{
public:
	float X, Y, Z, W;

public:
	Quaternion();
	Quaternion(float, float, float, float);
	explicit Quaternion(const float*);
	Quaternion(const Quaternion&);
	Quaternion(const Matrix&);
	Quaternion(const Vector3& axis, float angle);

	GRANDPA_API Quaternion& operator=(const Matrix&);

	Quaternion& operator=(const Quaternion&);
	Quaternion& operator+=(const Quaternion&);
	Quaternion& operator-=(const Quaternion&);
	Quaternion& operator*=(const Quaternion&);
	Quaternion& operator*=(float);
	Quaternion& operator/=(float);

	bool operator==(const Quaternion&) const;
	bool operator!=(const Quaternion&) const;

	Quaternion operator+() const;
	Quaternion operator-() const;

	Quaternion operator+(const Quaternion&) const;
	Quaternion operator-(const Quaternion&) const;
	Quaternion operator*(const Quaternion&) const;
	Quaternion operator*(float) const;
	Vector3 operator*(const Vector3&) const;
	Quaternion operator/(float) const;

	friend Quaternion operator*(float, const Quaternion&);

public:
	void set(float, float, float, float);

	float length() const;
	float lengthSq() const;
	float distance(const Quaternion&) const;
	float distanceSq(const Quaternion&) const;

	float dot(const Quaternion&) const;
	Quaternion &identify();
	Quaternion conjugate() const;
	Quaternion getNormalized() const;
	Quaternion& normalize();
	Quaternion getInverse() const;
	Quaternion& inverse();
	Quaternion getLerp(const Quaternion& q, float f) const;
	Quaternion& lerp(const Quaternion& q, float f);
	Quaternion getNlerp(const Quaternion& q, float f) const;
	Quaternion& nlerp(const Quaternion& q, float f);
	Quaternion getSlerp(const Quaternion& q, float f) const;
	Quaternion& slerp(const Quaternion& q, float f);

	float yaw_yxz() const;
	float yaw_zxy() const;
	float pitch_yxz() const;
	float pitch_zxy() const;
	float roll_yxz() const;
	float roll_zxy() const;

	Euler getEuler_yxz() const;
	Euler getEuler_zxy() const;

	void setEuler_yxz(const Euler&);
	void setEuler_zxy(const Euler&);

public:
	GRANDPA_API static const Quaternion	ZERO;
	GRANDPA_API static const Quaternion	IDENTITY;
};

inline Quaternion::Quaternion()
{
}

inline Quaternion::Quaternion(float x, float y, float z, float w)
	: X(x), Y(y), Z(z), W(w)
{
}

inline Quaternion::Quaternion(const float* p)
	: X(p[0]), Y(p[1]), Z(p[2]), W(p[3])
{
}

inline Quaternion::Quaternion(const Quaternion& q)
	: X(q.X), Y(q.Y), Z(q.Z), W(q.W)
{
}

inline Quaternion::Quaternion(const Matrix& m)
{
	*this = m;
}

inline Quaternion& Quaternion::operator=(const Quaternion& q)
{
	X = q.X;
	Y = q.Y;
	Z = q.Z;
	W = q.W;

	return *this;
}

inline Quaternion::Quaternion(const Vector3& axis, float angle)
{
	float halfAngle = 0.5f * angle;
	float fSin = sinf(halfAngle);
	X = fSin * axis.X;
	Y = fSin * axis.Y;
	Z = fSin * axis.Z;
	W = cosf(halfAngle);
}

inline Quaternion& Quaternion::operator+=(const Quaternion& q)
{
	X += q.X;
	Y += q.Y;
	Z += q.Z;
	W += q.W;

	return *this;
}

inline Quaternion& Quaternion::operator-=(const Quaternion& q)
{
	X -= q.X;
	Y -= q.Y;
	Z -= q.Z;
	W -= q.W;

	return *this;
}

inline Quaternion& Quaternion::operator*=(const Quaternion& q)
{
	return *this = *this * q;
}

inline Quaternion& Quaternion::operator*=(float f)
{
	X *= f;
	Y *= f;
	Z *= f;
	W *= f;
	return *this;
}

inline Quaternion& Quaternion::operator/=(float f)
{
	float inv = 1.0f / f;
	X *= inv;
	Y *= inv;
	Z *= inv;
	W *= inv;
	return *this;
}

inline bool Quaternion::operator==(const Quaternion& q) const
{
	return X == q.X && Y == q.Y && Z == q.Z && W == q.W;
}

inline bool Quaternion::operator!=(const Quaternion& q) const
{
	return X != q.X || Y != q.Y || Z != q.Z || W != q.W;
}

inline Quaternion Quaternion::operator+() const
{
	return *this;
}

inline Quaternion Quaternion::operator-() const
{
	return Quaternion(-X, -Y, -Z, -W);
}

inline Quaternion Quaternion::operator+(const Quaternion& q) const
{
	return Quaternion(X + q.X, Y + q.Y, Z + q.Z, W + q.W);
}

inline Quaternion Quaternion::operator-(const Quaternion& q) const
{
	return Quaternion(X - q.X, Y - q.Y, Z - q.Z, W - q.W);
}

inline Quaternion Quaternion::operator*(const Quaternion& q) const
{
	return Quaternion(	q.W * X + q.X * W + q.Y * Z - q.Z * Y,
						q.W * Y - q.X * Z + q.Y * W + q.Z * X,
						q.W * Z + q.X * Y - q.Y * X + q.Z * W,
						q.W * W - q.X * X - q.Y * Y - q.Z * Z);
}

inline Quaternion Quaternion::operator*(float f) const
{
	return Quaternion(X * f, Y * f, Z * f, W * f);
}

inline Vector3 Quaternion::operator*(const Vector3& v) const
{
	Vector3 uv, uuv;
	Vector3 qvec(X, Y, Z);
	uv = qvec.cross(v);
	uuv = qvec.cross(uv);
	uv *= (2.0f * W);
	uuv *= 2.0f;

	return v + uv + uuv;
}

inline Quaternion Quaternion::operator/(float f) const
{
	float inv = 1.0f / f;
	return Quaternion(X * inv, Y * inv, Z * inv, W * inv);
}

inline Quaternion operator*(float f, const Quaternion& q)
{
	return Quaternion(f * q.X, f * q.Y, f * q.Z, f * q.W);
}

inline void Quaternion::set(float x, float y, float z, float w)
{
	X = x;
	Y = y;
	Z = z;
	W = w;
}

inline float Quaternion::length() const
{
	return sqrtf(X * X + Y * Y + Z * Z + W * W);
}

inline float Quaternion::lengthSq() const
{
	return X * X + Y * Y + Z * Z + W * W;
}

inline float Quaternion::distance(const Quaternion& v) const
{
	return Vector4(X - v.X, Y - v.Y, Z - v.Z, W - v.W).length(); 
}

inline float Quaternion::distanceSq(const Quaternion& v) const
{
	return Vector4(X - v.X, Y - v.Y, Z - v.Z, W - v.W).lengthSq();
}

inline float Quaternion::dot(const Quaternion& q) const
{
	return X * q.X + Y * q.Y + Z * q.Z + W * q.W;
}

inline Quaternion &Quaternion::identify()
{
	X = Y = Z = 0.0f;
	W = 1.0f;
	return *this;
}

inline Quaternion Quaternion::conjugate() const
{
	return Quaternion(-X, -Y, -Z, W) ;
}

inline Quaternion Quaternion::getNormalized() const
{
	float l = length();
	return l != 0.0f ? (*this / l) : Quaternion(0.0f, 0.0f, 0.0f, 1.0f);
}

inline Quaternion& Quaternion::normalize()
{
	float l = length();
	if (l != 0.0f)
	{
		*this /= l;
	}
	else
	{
		X = 0.0f;
		Y = 0.0f;
		Z = 0.0f;
		W = 1.0f;
	}
	return *this;
}

inline Quaternion Quaternion::getInverse() const
{
	float l = lengthSq();
	if (l > 0.0f)
	{
		l = 1 / l;
		return Quaternion(-X * l, -Y * l, -Z * l, W * l);
	}
	else
	{
		// return an invalid result to flag the error
		return Quaternion(0.0f, 0.0f, 0.0f, 0.0f);
	}
}

inline Quaternion& Quaternion::inverse()
{
	float l = lengthSq();
	if (l > 0.0f)
	{
		l = 1 / l;
		X = -X * l;
		Y = -Y * l;
		Z = -Z * l;
		W =  W * l;
	}
	else
	{
		// return an invalid result to flag the error
		X = Y = Z = W = 0.0f;
	}
	return *this;
}

inline Quaternion Quaternion::getLerp(const Quaternion& q, float f) const
{
#ifdef QUATERNION_SLERP
	return getSlerp(q, f);
#else
	return getNlerp(q, f);
#endif
}

inline Quaternion& Quaternion::lerp(const Quaternion& q, float f)
{
#ifdef sQUATERNION_SLERP
	return slerp(q, f);
#else
	return nlerp(q, f);
#endif
}

inline Quaternion Quaternion::getNlerp(const Quaternion& q, float f) const
{
	float inv_f = 1.0f - f;
	if (dot(q) < 0.0f)
	{
		f = -f;
	}
	Quaternion ret = *this * inv_f + q * f;
	ret.normalize();
	return ret;
}

inline Quaternion& Quaternion::nlerp(const Quaternion& q, float f)
{
	float inv_f = 1.0f - f;
	if (dot(q) < 0.0f)
	{
		f = -f;
	}
	*this = *this * inv_f + q * f;
	normalize();
	return *this;
}

inline Quaternion Quaternion::getSlerp(const Quaternion& q, float d) const
{
	float norm;
	norm = dot(q);
	
	bool flip = false;
	
	if (norm < 0.0f)
	{
		norm = -norm;
		flip = true;
	}
	
	float inv_d;
	if (1.0f - norm < 0.00001f)
	{
		inv_d = 1.0f - d;
	}
	else
	{
		float theta;
		theta = acosf(norm);
		
		float s = 1.0f / sinf(theta);
		
		inv_d = sinf((1.0f - d) * theta) * s;
		d = sinf(d * theta) * s;
	}
	
	if (flip)
	{
		d = -d;
	}
	
	return Quaternion( inv_d * X + d * q.X,
						inv_d * Y + d * q.Y,
						inv_d * Z + d * q.Z,
						inv_d * W + d * q.W);
}

inline Quaternion& Quaternion::slerp(const Quaternion& q, float d)
{
	float norm;
	norm = X * q.X + Y * q.Y + Z * q.Z + W * q.W;
	
	bool flip = false;
	
	if (norm < 0.0f)
	{
		norm = -norm;
		flip = true;
	}
	
	float inv_d;
	if (1.0f - norm < 0.000001f)
	{
		inv_d = 1.0f - d;
	}
	else
	{
		float theta;
		theta = acosf(norm);
		
		float s = (1.0f / sinf(theta));
		
		inv_d = sinf((1.0f - d) * theta) * s;
		d = sinf(d * theta) * s;
	}
	
	if (flip)
	{
		d = -d;
	}
	X = inv_d * X + d * q.X;
	Y = inv_d * Y + d * q.Y;
	Z = inv_d * Z + d * q.Z;
	W = inv_d * W + d * q.W;
	return *this;
}

inline float Quaternion::yaw_yxz() const
{
	return atan2f(2.0f * (X * Z + Y * W), W * W - X * X - Y * Y + Z * Z);
}

inline float Quaternion::yaw_zxy() const
{
	return atan2f(2.0f * (Z * W - X * Y), W * W - X * X + Y * Y - Z * Z);
}

inline float Quaternion::pitch_yxz() const
{
	return asinf(-2.0f * (Y * Z - X * W));
}

inline float Quaternion::pitch_zxy() const
{
	return asinf(2.0f * (Y * Z + X * W));
}

inline float Quaternion::roll_yxz() const
{
	return atan2f(2.0f * (X * Y + Z * W), W * W - X * X + Y * Y - Z * Z);
}

inline float Quaternion::roll_zxy() const
{
	return atan2f(2.0f * (Y * W - X * Z), W * W - X * X - Y * Y + Z * Z);
}

inline Euler Quaternion::getEuler_yxz() const
{
	return Euler(yaw_yxz(), pitch_yxz(), roll_yxz());
}

inline Euler Quaternion::getEuler_zxy() const
{
	return Euler(yaw_zxy(), pitch_zxy(), roll_zxy());
}

inline void Quaternion::setEuler_yxz(const Euler& e)
{
	float cr, cp, cy, sr, sp, sy, cycp, sysp, sycp, cysp;

	cy = cosf(e.yaw / 2);
	cp = cosf(e.pitch / 2);
	cr = cosf(e.roll / 2);

	sy = sinf(e.yaw / 2);
	sp = sinf(e.pitch / 2);
	sr = sinf(e.roll / 2);

	cycp = cy * cp;
	sysp = sy * sp;
	sycp = sy * cp;
	cysp = cy * sp;

	X = cysp * cr + sycp * sr;
	Y = sycp * cr - cysp * sr;
	Z = cycp * sr - sysp * cr;
	W = cycp * cr + sysp * sr;
}

inline void Quaternion::setEuler_zxy(const Euler& e)
{
	float cr, cp, cy, sr, sp, sy, spcr, cpsr, cpcr, spsr;

	cy = cosf(e.yaw / 2);
	cp = cosf(e.pitch / 2);
	cr = cosf(e.roll / 2);

	sy = sinf(e.yaw / 2);
	sp = sinf(e.pitch / 2);
	sr = sinf(e.roll / 2);

	spcr = sp * cr;
	cpsr = cp * sr;
	cpcr = cp * cr;
	spsr = sp * sr;

	X = spcr * cy - cpsr * sy;
	Y = cpsr * cy + spcr * sy;
	Z = spsr * cy + cpcr * sy;
	W = cpcr * cy - spsr * sy;
}

}

#endif
