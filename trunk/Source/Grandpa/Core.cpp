#include "Precompiled.h"
#include "Core.h"

namespace grp
{

///////////////////////////////////////////////////////////////////////////////
// Vector2
///////////////////////////////////////////////////////////////////////////////
const Vector2 Vector2::ZERO	= Vector2(0.0f, 0.0f);

///////////////////////////////////////////////////////////////////////////////
// Vector3
///////////////////////////////////////////////////////////////////////////////
const Vector3 Vector3::ZERO	= Vector3(0.0f, 0.0f, 0.0f);

///////////////////////////////////////////////////////////////////////////////
// Vector4
///////////////////////////////////////////////////////////////////////////////
const Vector4 Vector4::ZERO	= Vector4(0.0f, 0.0f, 0.0f, 0.0f);

///////////////////////////////////////////////////////////////////////////////
// Matrix
///////////////////////////////////////////////////////////////////////////////
const Matrix Matrix::ZERO = Matrix(0.0f, 0.0f, 0.0f, 0.0f,
									0.0f, 0.0f, 0.0f, 0.0f,
									0.0f, 0.0f, 0.0f, 0.0f,
									0.0f, 0.0f, 0.0f, 0.0f);
const Matrix Matrix::IDENTITY = Matrix(1.0f, 0.0f, 0.0f, 0.0f,
										0.0f, 1.0f, 0.0f, 0.0f,
										0.0f, 0.0f, 1.0f, 0.0f,
										0.0f, 0.0f, 0.0f, 1.0f);

///////////////////////////////////////////////////////////////////////////////
// Quaternion
///////////////////////////////////////////////////////////////////////////////
const Quaternion Quaternion::IDENTITY = Quaternion(0.0f, 0.0f, 0.0f, 1.0f);

///////////////////////////////////////////////////////////////////////////////
// AaBox
///////////////////////////////////////////////////////////////////////////////
const AaBox AaBox::EMPTY;

Quaternion& Quaternion::operator=(const Matrix& m)
{
	float tr, s;
	static const int nxt[3] = { 1, 2, 0 };

	tr = m.M[0][0] + m.M[1][1] + m.M[2][2];

	if (tr > 0.0f) 
	{
		s = ::sqrtf(tr + 1.0f);
		W = s * 0.5f;
		s = 0.5f / s;

		X = (m.M[1][2] - m.M[2][1]) * s;
		Y = (m.M[2][0] - m.M[0][2]) * s;
		Z = (m.M[0][1] - m.M[1][0]) * s;
	} 
	else 
	{
		// diagonal is negative
		int i, j, k;
		float qt[4];

		i = 0;
		if (m.M[1][1] > m.M[0][0])
		{
			i = 1;
		}
		if (m.M[2][2] > m.M[i][i])
		{
			i = 2;
		}
		j = nxt[i];
		k = nxt[j];

		s = m.M[i][i] - m.M[j][j] - m.M[k][k] + 1.0f;
		s = ::sqrtf(s);

		qt[i] = s * 0.5f;

		if (s != 0.0f)
		{
			s = 0.5f / s;
		}
		qt[3] = (m.M[j][k] - m.M[k][j]) * s;
		qt[j] = (m.M[i][j] + m.M[j][i]) * s;
		qt[k] = (m.M[i][k] + m.M[k][i]) * s;

		X = qt[0];
		Y = qt[1];
		Z = qt[2];
		W = qt[3];
	}
	return *this;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
unsigned long compressQuaternion(const Quaternion& q)
{
	unsigned long compressed = 0;
	
	float maxAbs = 0.0f;
	size_t maxIndex = 0;
	bool minus = false;
	for (size_t i = 0; i < 4; ++i)
	{
		float value = ((float*)&q)[i];
		float absValue = fabs(value);
		if (absValue > maxAbs)
		{
			maxAbs = absValue;
			maxIndex = i;
			minus = (value < 0.0f);
		}
	}
	compressed |= (maxIndex << 30);

	int bitMove = 0;
	for (size_t i = 0; i < 4; ++i)
	{
		if (i == maxIndex)
		{
			continue;
		}
		float value = ((float*)&q)[i];
		if (minus)
		{
			value = -value;
		}
		assert(fabs(value) < 0.7071068f);
		unsigned long dwValue = (unsigned long)((value + 0.7071068f) * 1023 / 1.414214f + 0.5f);
		compressed |= (dwValue << (bitMove));
		bitMove += 10;
	}
	return compressed;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void decompressQuaternion(Quaternion& q, unsigned long c)
{
	size_t maxIndex = (c >> 30);

	int bitMove = 0;
	float sqr = 1.0f;
	for (size_t i = 0; i < 4; ++i)
	{
		if (i == maxIndex)
		{
			continue;
		}
		unsigned long dwValue = (c >> bitMove) & 1023;
		float value = dwValue / 1023.0f * 1.414214f - 0.7071068f;
		((float*)&q)[i] = value;
		sqr -= (value * value);
		bitMove += 10;
	}
	((float*)&q)[maxIndex] = sqrt(sqr);
}

}
