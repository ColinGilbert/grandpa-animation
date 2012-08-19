#ifndef __GRP_FRUSTUM_H__
#define __GRP_FRUSTUM_H__

#include "Plane.h"
#include "Vector.h"
#include "Matrix.h"
#include "AaBox.h"

namespace grp
{

class Frustum
{
public:
	enum FrustumPlane
	{
		PLANE_LEFT = 0,
		PLANE_RIGHT,
		PLANE_BOTTOM,
		PLANE_TOP,
		PLANE_NEAR,
		PLANE_FAR,
		PLANE_COUNT
	};

public:
	Frustum();
	Frustum(const Vector3& cameraPos, const Matrix& matrix);

	void buildFromMatrix(const Vector3& cameraPos, const Matrix& matrix);

	bool intersectWithBox(const AaBox& box) const;
	//bool intersectWithLine(const Line3& line) const;
	bool isPointInside(const Vector3& point) const;

	Vector3 getFarLeftUp() const;
	Vector3 getFarLeftDown() const;
	Vector3 getFarRightUp() const;
	Vector3 getFarRightDown() const;

public:
	Vector3	CameraPos;
	Plane	Planes[PLANE_COUNT];
	int		VertexLUT[PLANE_COUNT];
};

///////////////////////////////////////////////////////////////////////////////////////////////////
inline Frustum::Frustum()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline Frustum::Frustum(const Vector3& cameraPos, const Matrix& matrix)
{
	buildFromMatrix(cameraPos, matrix);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void Frustum::buildFromMatrix(const Vector3& cameraPos, const Matrix& matrix)
{
	CameraPos = cameraPos;

	Vector4 column1(matrix._11, matrix._21, matrix._31, matrix._41);
	Vector4 column2(matrix._12, matrix._22, matrix._32, matrix._42);
	Vector4 column3(matrix._13, matrix._23, matrix._33, matrix._43);
	Vector4 column4(matrix._14, matrix._24, matrix._34, matrix._44);

	Planes[PLANE_LEFT]	.setAsVector4(column4 + column1).normalize();
	Planes[PLANE_RIGHT]	.setAsVector4(column4 - column1).normalize();
	Planes[PLANE_BOTTOM].setAsVector4(column4 + column2).normalize();
	Planes[PLANE_TOP]	.setAsVector4(column4 - column2).normalize();
	Planes[PLANE_NEAR]	.setAsVector4(column4 + column3).normalize();
	Planes[PLANE_FAR]	.setAsVector4(column4 - column3).normalize();

	//	   5-------4
	//	  /|      /|
	//	 / |     / |
	//	1-------0  |
	//	|  7----|--6  far
	//	| /     | /
	//	|/      |/
	//	3-------2  near
	//  build a bit-field that will tell us the indices for the nearest and farthest vertices from each plane...
	for (int i = 0; i < 6; i++)
	{
		VertexLUT[i] = ((Planes[i].A < 0.0f) ? 1 : 0) |
					   ((Planes[i].B < 0.0f) ? 2 : 0) |
					   ((Planes[i].C < 0.0f) ? 4 : 0);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline bool Frustum::intersectWithBox(const AaBox& box) const
{
	for (int i = 0; i < PLANE_COUNT; i++)
	{
		int nV = VertexLUT[i];
		// pVertex is diagonally opposed to nVertex
		Vector3 nVertex((nV & 1) ? box.MinEdge.X : box.MaxEdge.X,
						 (nV & 2) ? box.MinEdge.Y : box.MaxEdge.Y,
						 (nV & 4) ? box.MinEdge.Z : box.MaxEdge.Z);

		if (Planes[i].dotCoord(nVertex) < 0.0f)
		{
			return false;
		}
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline bool Frustum::isPointInside(const Vector3& point) const
{
	for (int i = 0; i < PLANE_COUNT; i++)
	{
		float dot = Planes[i].dotCoord(point);
		if (dot < 0.0f)
		{
			return false;
		}
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline Vector3 Frustum::getFarLeftUp() const
{
	Vector3 p;
	Planes[PLANE_FAR].getIntersectionWithPlanes(Planes[PLANE_TOP], Planes[PLANE_LEFT], p);
	return p;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline Vector3 Frustum::getFarLeftDown() const
{
	Vector3 p;
	Planes[PLANE_FAR].getIntersectionWithPlanes(Planes[PLANE_BOTTOM], Planes[PLANE_LEFT], p);
	return p;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline Vector3 Frustum::getFarRightUp() const
{
	Vector3 p;
	Planes[PLANE_FAR].getIntersectionWithPlanes(Planes[PLANE_TOP], Planes[PLANE_RIGHT], p);
	return p;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline Vector3 Frustum::getFarRightDown() const
{
	Vector3 p;
	Planes[PLANE_FAR].getIntersectionWithPlanes(Planes[PLANE_BOTTOM], Planes[PLANE_RIGHT], p);
	return p;
}

}

#endif
