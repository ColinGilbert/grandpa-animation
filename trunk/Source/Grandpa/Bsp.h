#ifndef __GRP_BSP_TREE_H__
#define __GRP_BSP_TREE_H__

#include <vector>
#include <istream>
#include <ostream>
#include "Plane.h"

namespace grp
{

class BspTree;

///////////////////////////////////////////////////////////////////////////////
struct BspTriangle
{
	Plane			plane;
	unsigned long	vertices[3];
};

///////////////////////////////////////////////////////////////////////////////
class BspNode
{
	friend class BspTree;

public:
	enum SideType
	{
		FRONT = 0,
		BACK,
		ON,
		CROSS
	};
	enum
	{
		LEAF_NODE	= 2
	};

public:
	BspNode();
	~BspNode();

public:
	int GetMaxDepth() const;

	void Build(const VECTOR(BspTriangle)& vecTri, SideType side);

	bool PointIntersect(const Vector3& vPos) const;

	bool RayIntersect(const Vector3& v0, const Vector3& v1) const;

	bool SphereIntersect(const Vector3& v0, const Vector3& v1, float fRadius) const;

	bool IsLeaf() const;

	bool Import(std::istream& input);

	void Export(std::ostream& output) const;

private:
	const BspTriangle* SelectSpliter(const VECTOR(BspTriangle)& vecTri) const;

	void TestSpliter(const BspTriangle& spliter,
					  const VECTOR(BspTriangle)& vecTri,
					  int& iFront,
					  int& iBack,
					  int& iOn,
					  int& iCross) const;

	void Split(const BspTriangle& divider,
				const VECTOR(BspTriangle)& vecTri,
				VECTOR(BspTriangle)& vecTriFront,
				VECTOR(BspTriangle)& vecTriBack) const;

private:
	BspTree*		m_pTree;
	Plane			m_Plane;
	BspNode*		m_pChildren[2];	//0-front,1-back
	unsigned long	m_dwType;		//bit0:front or back; bit1:isleaf
};

///////////////////////////////////////////////////////////////////////////////
inline bool BspNode::IsLeaf() const
{
	return ((m_dwType & LEAF_NODE) != 0);
}

///////////////////////////////////////////////////////////////////////////////
class BspTree
{
	friend class BspNode;

public:
	enum HitType
	{
		CLEAR		= 0,
		INTERSECT	= 1,
		INSIDE		= 2
	};

public:
	BspTree();
	~BspTree();

public:
	size_t getNodeCount() const;
	int getMaxDepth() const;

	bool AddMesh(const Vector3* pVertex,
				  size_t dwNumVertex,
				  const unsigned long* pIndex,
				  size_t dwNumTriangle);

	bool Build();

	HitType PointIntersect(const Vector3& vPos);

	HitType RayIntersect(const Vector3& v0,
						  const Vector3& v1,
						  Vector3* pPtHit = NULL,
						  Plane* pPlaneHit = NULL);

	HitType SphereIntersect(const Vector3& v0,
							 const Vector3& v1,
							 float fRadius,
							 Plane* pPlaneHit = NULL);


	bool Import(std::istream& input);

	void Export(std::ostream& output) const;

private:
	void BuildTriangle(BspTriangle& triangle) const;

	void CutTriangle(const BspTriangle& tri,
					  VECTOR(BspTriangle)& vecTriFront,
					  VECTOR(BspTriangle)& vecTriBack,
					  const Plane& plane);

	BspNode::SideType TriangleOnPlaneSide(const BspTriangle& tri,
											const Plane& plane) const;

	BspNode* AddNode();

	unsigned long AddVertex(const Vector3& vertex);

	//temp for debug
//private:
public:
	BspNode*		m_pRoot;

	Vector3			m_vHit;
	const Plane*	m_pPlaneHit;

	VECTOR(BspNode)	m_vecNode;

	//only be used by build()
	VECTOR(Vector3)		m_vecVertex;
	VECTOR(BspTriangle)	m_vecTriangle;
};

///////////////////////////////////////////////////////////////////////////////
inline size_t BspTree::getNodeCount() const
{
	return m_vecNode.size();
}

///////////////////////////////////////////////////////////////////////////////
inline int BspTree::getMaxDepth() const
{
	if (NULL == m_pRoot)
	{
		return 0;
	}
	return m_pRoot->GetMaxDepth();
}

///////////////////////////////////////////////////////////////////////////////
inline BspNode* BspTree::AddNode()
{
	//temp
	assert(m_vecNode.size() < m_vecNode.capacity());
	m_vecNode.resize(m_vecNode.size() + 1);
	BspNode* pNode = &(m_vecNode.back());
	pNode->m_pTree = this;
	return pNode;
}

///////////////////////////////////////////////////////////////////////////////
inline unsigned long BspTree::AddVertex(const Vector3& vertex)
{
	m_vecVertex.push_back(vertex);
	return static_cast<unsigned long>(m_vecVertex.size() - 1);
}

}

#endif
