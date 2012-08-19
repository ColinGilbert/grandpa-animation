																																				#include "Precompiled.h"
#include "Bsp.h"

namespace grp
{

//容差
const float EPSILON = 0.001f;

const int CUT_WEIGHT		= 3;
const int BALANCE_WEIGHT	= 1;
const int ON_WEIGHT			= 1;

///////////////////////////////////////////////////////////////////////////////
inline Vector3 PlaneHit(const Vector3& v0, const Vector3& v1, const Plane& plane)
{
	float d0 = plane.dotCoord(v0);
	float d1 = plane.dotCoord(v1);
	return v0 + (v1 - v0) * d0 / (d0 - d1);
}

///////////////////////////////////////////////////////////////////////////////
BspNode::BspNode()
	: m_pTree(NULL)
	, m_dwType(0)
{
	m_pChildren[FRONT] = NULL;
	m_pChildren[BACK] = NULL;
}

///////////////////////////////////////////////////////////////////////////////
BspNode::~BspNode()
{
}

///////////////////////////////////////////////////////////////////////////////
int BspNode::GetMaxDepth() const
{
	int iDepth[2] = { 1, 1 };

	for (int i = 0; i < 2; ++i)
	{
		if (m_pChildren[i] != NULL)
		{
			iDepth[i] += m_pChildren[i]->GetMaxDepth();
		}
	}
	return std::max(iDepth[FRONT], iDepth[BACK]);
}

///////////////////////////////////////////////////////////////////////////////
void BspNode::Build(const VECTOR(BspTriangle)& vecTri, SideType side)
{
	m_dwType = side;

	if (vecTri.empty())
	{
		m_dwType |= LEAF_NODE;
		return;
	}

	const BspTriangle* pBestSpliter = SelectSpliter(vecTri);
	assert(pBestSpliter != NULL);

	m_Plane = pBestSpliter->plane;

	VECTOR(BspTriangle) vecTriFront, vecTriBack;

	Split(*pBestSpliter, vecTri, vecTriFront, vecTriBack);

	//递归子节点
	m_pChildren[FRONT] = m_pTree->AddNode();
	m_pChildren[FRONT]->Build(vecTriFront, FRONT);

	m_pChildren[BACK] = m_pTree->AddNode();
	m_pChildren[BACK]->Build(vecTriBack, BACK);
}

///////////////////////////////////////////////////////////////////////////////
bool BspNode::PointIntersect(const Vector3& vPos) const
{
	if (IsLeaf())
	{
		return (m_dwType & BACK);
	}
	assert(m_pChildren[FRONT] != NULL);
	assert(m_pChildren[BACK] != NULL);

	if (m_Plane.dotCoord(vPos) >= 0.0f)
	{
		return m_pChildren[FRONT]->PointIntersect(vPos);
	}
	return m_pChildren[BACK]->PointIntersect(vPos);
}

///////////////////////////////////////////////////////////////////////////////
bool BspNode::RayIntersect(const Vector3& v0, const Vector3& v1) const
{
	if (IsLeaf())
	{
		return (m_dwType & BACK);
	}
	assert(m_pChildren[FRONT] != NULL);
	assert(m_pChildren[BACK] != NULL);

	float d0 = m_Plane.dotCoord(v0),
		  d1 = m_Plane.dotCoord(v1);

	if (d0 >= 0.0f && d1 >= 0.0f)
	{
		return m_pChildren[FRONT]->RayIntersect(v0, v1);
	}
	if (d0 < 0.0f && d1 < 0.0f)
	{
		return m_pChildren[BACK]->RayIntersect(v0, v1);
	}
	//cross
	Vector3 vMid = v0 + (v1 - v0) * d0 / (d0 - d1);
	int iFirst;
	if (d0 > d1)
	{
		//正面射入背面，记录交点和碰撞面
		assert(m_pTree != NULL);
		m_pTree->m_vHit = vMid;
		m_pTree->m_pPlaneHit = &m_Plane;
		iFirst = 0;
	}
	else
	{
		//从背面射入正面
		if (NULL == m_pTree->m_pPlaneHit)
		{
			m_pTree->m_vHit = vMid;
			m_pTree->m_pPlaneHit = &m_Plane;
		}
		iFirst = 1;
	}
	if (m_pChildren[iFirst]->RayIntersect(v0, vMid) ||
		m_pChildren[1 - iFirst]->RayIntersect(vMid, v1))
	{
		return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////
bool BspNode::Import(std::istream& input)
{
	input.read((char*)&m_dwType, sizeof(unsigned long));
	input.read((char*)&m_Plane, sizeof(Plane));
	if (!IsLeaf())
	{
		assert(m_pTree != NULL);
		m_pChildren[FRONT] = m_pTree->AddNode();
		m_pChildren[FRONT]->Import(input);
		m_pChildren[BACK] = m_pTree->AddNode();
		m_pChildren[BACK]->Import(input);
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
void BspNode::Export(std::ostream& output) const
{
	output.write((char*)&m_dwType, sizeof(unsigned long));
	output.write((char*)&m_Plane, sizeof(Plane));
	if (!IsLeaf())
	{
		m_pChildren[FRONT]->Export(output);
		m_pChildren[BACK]->Export(output);
	}
}

///////////////////////////////////////////////////////////////////////////////
bool BspNode::SphereIntersect(const Vector3& v0, const Vector3& v1, float fRadius) const
{
	if (IsLeaf())
	{
		return (m_dwType & BACK);
	}
	assert(m_pChildren[FRONT] != NULL);
	assert(m_pChildren[BACK] != NULL);

	float d0 = m_Plane.dotCoord(v0),
		  d1 = m_Plane.dotCoord(v1);

	if (d0 >= fRadius && d1 >= fRadius)
	{
		return m_pChildren[FRONT]->SphereIntersect(v0, v1, fRadius);
	}
	if (d0 < -fRadius && d1 < -fRadius)
	{
		return m_pChildren[BACK]->SphereIntersect(v0, v1, fRadius);
	}
	//相交
	Vector3 vMidFirst, vMidSecond;
	int iFirst;
	if (d0 > d1)
	{
		//正面进入背面，记录碰撞面
		assert(m_pTree != NULL);
		m_pTree->m_pPlaneHit = &m_Plane;
		iFirst = 0;
		float d = std::min(d0 + fRadius, d0 - d1);
		vMidFirst = v0 + (v1 - v0) * d / (d0 - d1);
		d = std::max(d0 - fRadius, 0.0f);
		vMidSecond = v0 + (v1 - v0) * d / (d0 - d1);
	}
	else
	{
		iFirst = 1;
		float d = std::max(d0 - fRadius, d0 - d1);
		vMidFirst = v0 + (v1 - v0) * d / (d0 - d1);
		d = std::min(d0 + fRadius, 0.0f);
		vMidSecond = v0 + (v1 - v0) * d / (d0 - d1);
		//if (NULL == m_pTree->m_pPlaneHit)
		//{
		//	m_pTree->m_pPlaneHit = &m_Plane;
		//}
	}
	if (m_pChildren[iFirst]->SphereIntersect(v0, vMidFirst, fRadius) ||
		m_pChildren[1 - iFirst]->SphereIntersect(vMidSecond, v1, fRadius))
	{
		return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////
const BspTriangle* BspNode::SelectSpliter(const VECTOR(BspTriangle)& vecTri) const
{
	int iBestValue = INT_MAX;
	const BspTriangle* pBest = NULL;

	for (VECTOR(BspTriangle)::const_iterator iter = vecTri.begin();
		  iter != vecTri.end();
		  ++iter)
	{
		const BspTriangle& tri = *iter;

		int iNumFront = 0, iNumBack = 0, iNumOn = 0, iNumCross = 0;

		TestSpliter(tri, vecTri, iNumFront, iNumBack, iNumOn, iNumCross);

		int iDiffer = abs(iNumFront - iNumBack);	//正反面三角形数的差
		int iValue = CUT_WEIGHT * iNumCross
				   + BALANCE_WEIGHT * iDiffer
				   - iNumOn * ON_WEIGHT;
		if (iValue < iBestValue)
		{
			iBestValue = iValue;
			pBest = &tri;
		}
	}
	return pBest;
}

///////////////////////////////////////////////////////////////////////////////
//给定一个三角形数组和一个分割平面，统计在平面两边各有多少个三角形
//如果平面和三角形相交，那么iFront和iBack各加1
void BspNode::TestSpliter(const BspTriangle& divider,
							const VECTOR(BspTriangle)& vecTri,
							int& iFront,
							int& iBack,
							int& iOn,
							int& iCross) const
{
	iFront = 0;
	iBack = 0;

	for (VECTOR(BspTriangle)::const_iterator iter = vecTri.begin();
		  iter != vecTri.end();
		  ++iter)
	{
		const BspTriangle& tri = *iter;
		//忽略分割面本身
		if (&tri == &divider)
		{
			continue;
		}
		SideType st = m_pTree->TriangleOnPlaneSide(tri, divider.plane);
		switch (st)
		{
		case FRONT:
			++iFront;
			break;

		case BACK:
			++iBack;
			break;

		case ON:
			++iOn;
			break;

		case CROSS:
			++iCross;
			break;

		default:
			break;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
//给定一个三角形数组和一个分割平面，将所有三角形分为两个子集
void BspNode::Split(const BspTriangle& divider,
					const VECTOR(BspTriangle)& vecTri,
					VECTOR(BspTriangle)& vecTriFront,
					VECTOR(BspTriangle)& vecTriBack) const
{
	for (VECTOR(BspTriangle)::const_iterator iter = vecTri.begin();
		  iter != vecTri.end();
		  ++iter)
	{
		const BspTriangle& tri = *iter;
		//忽略分割面本身
		if (&tri == &divider)
		{
			continue;
		}
		SideType st = m_pTree->TriangleOnPlaneSide(tri, divider.plane);
		switch (st)
		{
		case FRONT:
			vecTriFront.push_back(tri);
			break;

		case BACK:
			vecTriBack.push_back(tri);
			break;

		case CROSS:
			m_pTree->CutTriangle(tri, vecTriFront, vecTriBack, divider.plane);
			break;

		default:
			break;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
//BspTree
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
BspTree::BspTree()
	: m_pRoot(NULL)
	, m_pPlaneHit(NULL)
{
}

///////////////////////////////////////////////////////////////////////////////
BspTree::~BspTree()
{
}

///////////////////////////////////////////////////////////////////////////////
bool BspTree::AddMesh(const Vector3* pVertex,
					    size_t dwNumVertex,
						const unsigned long* pIndex,
						size_t dwNumTriangle)
{
	assert(pVertex != NULL);
	assert(pIndex != NULL);

	//添加顶点
	size_t dwOldNumVertex = m_vecVertex.size();

	m_vecVertex.resize(dwOldNumVertex + dwNumVertex);
	memcpy(&(m_vecVertex[dwOldNumVertex]), pVertex, dwNumVertex * sizeof(Vector3));

	//添加三角形
	size_t dwOldNumTri = m_vecTriangle.size();
	m_vecTriangle.resize(dwOldNumTri + dwNumTriangle);
	for (size_t i = 0; i < dwNumTriangle; ++i)
	{
		//索引需要修正一下
		m_vecTriangle[dwOldNumTri + i].vertices[0] = pIndex[i * 3 + 0] + dwOldNumVertex;
		m_vecTriangle[dwOldNumTri + i].vertices[1] = pIndex[i * 3 + 1] + dwOldNumVertex;
		m_vecTriangle[dwOldNumTri + i].vertices[2] = pIndex[i * 3 + 2] + dwOldNumVertex;

		BuildTriangle(m_vecTriangle[i]);
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
bool BspTree::Build()
{
	if (m_vecVertex.empty() || m_vecTriangle.empty())
	{
		return false;
	}

	//设足够大的数组，避免重新分配内存导致子节点指针失效
	m_vecNode.clear();
	m_vecNode.reserve(m_vecTriangle.size() * 10);

	m_pRoot = AddNode();
	m_pRoot->Build(m_vecTriangle, BspNode::FRONT);

	//build完就没用了
	m_vecVertex.clear();
	m_vecTriangle.clear();

	return true;
}

///////////////////////////////////////////////////////////////////////////////
BspTree::HitType BspTree::PointIntersect(const Vector3& vPos)
{
	assert(m_pRoot != NULL);
	return m_pRoot->PointIntersect(vPos) ? INSIDE : CLEAR;
}

///////////////////////////////////////////////////////////////////////////////
BspTree::HitType BspTree::RayIntersect(const Vector3& v0,
										  const Vector3& v1,
										  Vector3* pPtHit,
										  Plane* pPlaneHit)
{
	m_vHit.X = 0.0f;
	m_vHit.Y = 0.0f;
	m_vHit.Z = 0.0f;
	m_pPlaneHit = NULL;

	assert(m_pRoot != NULL);
	if (m_pRoot->RayIntersect(v0, v1))
	{
		if (NULL == m_pPlaneHit)
		{
			return INSIDE;
		}
		if (pPtHit != NULL)
		{
			*pPtHit = m_vHit;
		}
		if (pPlaneHit != NULL)
		{
			*pPlaneHit = *m_pPlaneHit;
		}
		return INTERSECT;
	}
	return CLEAR;
}

///////////////////////////////////////////////////////////////////////////////
BspTree::HitType BspTree::SphereIntersect(const Vector3& v0,
											 const Vector3& v1,
											 float fRadius,
											 Plane *pPlaneHit)
{
	m_pPlaneHit = NULL;
	assert(m_pRoot != NULL);
	if (m_pRoot->SphereIntersect(v0, v1, fRadius))
	{
		if (NULL == m_pPlaneHit)
		{
			return INSIDE;
		}
		if (pPlaneHit != NULL)
		{
			*pPlaneHit = *m_pPlaneHit;
		}
		return INTERSECT;
	}
	return CLEAR;
}

///////////////////////////////////////////////////////////////////////////////
bool BspTree::Import(std::istream& input)
{
	m_vecNode.clear();

	unsigned long dwNodeNum;
	input.read((char*)&dwNodeNum, sizeof(unsigned long));
	m_vecNode.reserve(dwNodeNum);

	m_pRoot = AddNode();
	m_pRoot->Import(input);
	return true;
}

///////////////////////////////////////////////////////////////////////////////
void BspTree::Export(std::ostream& output) const
{
	unsigned long dwNodeNum = static_cast<unsigned long>(getNodeCount());
	output.write((char*)&dwNodeNum, sizeof(unsigned long));

	assert(m_pRoot != NULL);
	m_pRoot->Export(output);
}

///////////////////////////////////////////////////////////////////////////////
//private

///////////////////////////////////////////////////////////////////////////////
void BspTree::BuildTriangle(BspTriangle& triangle) const
{
	const Vector3&	v0 = m_vecVertex[triangle.vertices[0]],
					v1 = m_vecVertex[triangle.vertices[1]],
					v2 = m_vecVertex[triangle.vertices[2]];
	//法线
	Vector3 crossResult = (v1 - v0).cross(v2 - v0);
	triangle.plane.getNormal() = crossResult.getNormalized();
	//偏移
	triangle.plane.getDist() = -v0.dot(triangle.plane.getNormal());	
}

///////////////////////////////////////////////////////////////////////////////
void BspTree::CutTriangle(const BspTriangle& tri,
							VECTOR(BspTriangle)& vecTriFront,
							VECTOR(BspTriangle)& vecTriBack,
							const Plane& plane)
{
	Vector3 v[3];
	float d[3];
	for (int i = 0; i < 3; ++i)
	{
		v[i] = m_vecVertex[tri.vertices[i]];
		d[i] = plane.dotCoord(v[i]);
	}

	int first  = 2,
		second = 0;
	while (!(d[second] > EPSILON && d[first] <= EPSILON))
	{
		first = second;
		second++;
	}
	//此时first在反面，second在正面

	//first,second线段和分割面交点
	Vector3 h = PlaneHit(v[first], v[second], plane);
	//正面
	do
	{
		first = second;
		second++;
		if (second >= 3)
		{
			second = 0;
		}

		//vecTriFront.resize
		BspTriangle triNew;

		triNew.vertices[0] = AddVertex(h);
		triNew.vertices[1] = tri.vertices[first];

		if (d[second] > EPSILON)
		{
			triNew.vertices[2] = tri.vertices[second];
		}
		else
		{
			h = PlaneHit(v[first], v[second], plane);
			triNew.vertices[2] = AddVertex(h);
		}
		triNew.plane = tri.plane;

		vecTriFront.push_back(triNew);

	} while (d[second] > EPSILON);

	// Skip zero area triangle
	if (fabsf(d[second]) <= EPSILON)
	{
		first = second;
		second++;
		if (second >= 3)
		{
			second = 0;
		}
	}

	//反面
	do
	{
		first = second;
		second++;
		if (second >= 3)
		{
			second = 0;
		}

		BspTriangle triNew;
		triNew.vertices[0] = AddVertex(h);
		triNew.vertices[1] = tri.vertices[first];
		if (d[second] < -EPSILON)
		{
			triNew.vertices[2] = tri.vertices[second];
		}
		else
		{
			triNew.vertices[2] = AddVertex(PlaneHit(v[first], v[second], plane));
		}

		triNew.plane = tri.plane;

		vecTriBack.push_back(triNew);
		
	} while (d[second] < -EPSILON);
}

///////////////////////////////////////////////////////////////////////////////
BspNode::SideType BspTree::TriangleOnPlaneSide(const BspTriangle& tri,
												  const Plane& plane) const
{
	bool bFront = false, bBack = false;
	for (int i = 0; i < 3; ++i)
	{
		float fDist = plane.dotCoord(m_vecVertex[tri.vertices[i]]);
		if (fDist < -EPSILON)
		{
			if (bFront)
			{
				return BspNode::CROSS;
			}
			bBack = true;
		}
		else if (fDist > EPSILON)
		{
			if (bBack)
			{
				return BspNode::CROSS;
			}
			bFront = true;
		}
	}
	if (bBack)
	{
		return BspNode::BACK;
	}
	if (bFront)
	{
		return BspNode::FRONT;
	}
	//方向相同的才算共面
	//if (plane.DotNormal(tri.plane.N) > 0.0f)
	//{
		return BspNode::ON;
	//}
	//return BspNode::BACK;
}

}
