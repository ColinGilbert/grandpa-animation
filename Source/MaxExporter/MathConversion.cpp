///////////////////////////////////////////////////////////////////////////////
//MathConversion.cpp
//
//描述：
//		导出过程中会用到的数学转换工具
//
//历史：
//		08-01-09	创建
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.h"
#include "MathConversion.h"

using grp::Matrix;
using grp::Quaternion;
using grp::Vector3;
using grp::Vector2;

/////////////////////////////////////////////////////////////////////////////////////////////
void MatrixFromMatrix3(Matrix& m44, const Matrix3& mat3)
{
	memcpy(&(m44.getVector3(0)), &(mat3.GetRow(0)), sizeof(Point3));
	memcpy(&(m44.getVector3(1)), &(mat3.GetRow(1)), sizeof(Point3));
	memcpy(&(m44.getVector3(2)), &(mat3.GetRow(2)), sizeof(Point3));
	memcpy(&(m44.getVector3(3)), &(mat3.GetRow(3)), sizeof(Point3));
	m44._14 = 0.0f;
	m44._24 = 0.0f;
	m44._34 = 0.0f;
	m44._44 = 1.0f;
}

/////////////////////////////////////////////////////////////////////////////////////////////
void MatrixSwapYZ(Matrix& m)
{
	 //矩阵的第2/3行互换，第2/3列互换
	for (int i = 0; i < 4; ++i)
	{
		float fTemp = m.M[1][i];
		m.M[1][i] = m.M[2][i];
		m.M[2][i] = fTemp;
	}
	for (int i = 0; i < 4; ++i)
	{
		float fTemp = m.M[i][1];
		m.M[i][1] = m.M[i][2];
		m.M[i][2] = fTemp;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////
void GetTransformFromMatrix3(const Matrix3& mat3,
							  Vector3& vTranslation,
							  Quaternion& qRotation,
							  Vector3& vScale)
{
	Matrix m44;
	::MatrixFromMatrix3(m44, mat3);
	m44.decomposition(vTranslation, vScale, qRotation);
}

/////////////////////////////////////////////////////////////////////////////////////////////
void CalculateTangent(const Vector3 v[3],
					   const Vector2 uv[3],
					   int index,	//calculate tangent and binormal for which point
					   Vector3& vTangent,
					   Vector3& vBinormal)
{
	assert(index >= 0 && index < 3);

	Vector3 v0 = v[index];
	Vector3 v1 = v[(index + 1) % 3];
	Vector3 v2 = v[(index + 2) % 3];
	Vector2 uv0 = uv[index];
	Vector2 uv1 = uv[(index + 1) % 3];
	Vector2 uv2 = uv[(index + 2) % 3];

	if (uv2.Y == uv1.Y)
	{
		if (uv2.X > uv1.X)
		{
			vTangent = (v2 - v1).getNormalized();
		}
		else
		{
			vTangent = (v1 - v2).getNormalized();
		}
	}
	else
	{
		float scale = (uv0.Y - uv1.Y) / (uv2.Y - uv1.Y);
		Vector3 dest = v1 + scale * (v2 - v1);
		Vector2 uvDest = uv1 + scale * (uv2 - uv1);
		if (uvDest.X > uv0.X)
		{
			vTangent = (dest - v0).getNormalized();
		}
		else
		{
			vTangent = (v0 - dest).getNormalized();
		}
	}
	if (uv2.X == uv1.X)
	{
		if (uv2.Y > uv1.Y)
		{
			vBinormal = (v2 - v1).getNormalized();
		}
		else
		{
			vBinormal = (v1 - v2).getNormalized();
		}
	}
	else
	{
		float scale = (uv0.X - uv1.X) / (uv2.X - uv1.X);
		Vector3 dest = v1 + scale * (v2 - v1);
		Vector2 uvDest = uv1 + scale * (uv2 - uv1);
		if (uvDest.Y > uv0.Y)
		{
			vBinormal = (dest - v0).getNormalized();
		}
		else
		{
			vBinormal = (v0 - dest).getNormalized();
		}
	}
}
